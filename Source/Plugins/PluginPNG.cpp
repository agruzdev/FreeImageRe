// ==========================================================
// PNG Loader and Writer
//
// Design and implementation by
// - Floris van den Berg (flvdberg@wxs.nl)
// - Herve Drolon (drolon@infonie.fr)
// - Detlev Vendt (detlev.vendt@brillit.de)
// - Aaron Shumate (trek@startreker.com)
// - Tanner Helland (tannerhelland@users.sf.net)
//
// This file is part of FreeImage 3
//
// COVERED CODE IS PROVIDED UNDER THIS LICENSE ON AN "AS IS" BASIS, WITHOUT WARRANTY
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, WITHOUT LIMITATION, WARRANTIES
// THAT THE COVERED CODE IS FREE OF DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE
// OR NON-INFRINGING. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE COVERED
// CODE IS WITH YOU. SHOULD ANY COVERED CODE PROVE DEFECTIVE IN ANY RESPECT, YOU (NOT
// THE INITIAL DEVELOPER OR ANY OTHER CONTRIBUTOR) ASSUME THE COST OF ANY NECESSARY
// SERVICING, REPAIR OR CORRECTION. THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL
// PART OF THIS LICENSE. NO USE OF ANY COVERED CODE IS AUTHORIZED HEREUNDER EXCEPT UNDER
// THIS DISCLAIMER.
//
// Use at your own risk!
// ==========================================================

#ifdef _MSC_VER 
#pragma warning (disable : 4786) // identifier was truncated to 'number' characters
#endif

#include "FreeImage.h"
#include "Utilities.h"

#include "../Metadata/FreeImageTag.h"

// ----------------------------------------------------------

#define PNG_BYTES_TO_CHECK 8

#undef PNG_Z_DEFAULT_COMPRESSION	// already used in ../LibPNG/pnglibconf.h

// ----------------------------------------------------------

#include <functional>
#include "zlib.h"
#include "png.h"

// ----------------------------------------------------------

typedef struct {
    FreeImageIO *s_io;
    fi_handle    s_handle;
} fi_ioStructure, *pfi_ioStructure;

// ==========================================================
// Plugin Interface
// ==========================================================

static int s_format_id;

// ==========================================================
// libpng interface
// ==========================================================

static void
_ReadProc(png_structp png_ptr, unsigned char *data, png_size_t size) {
    pfi_ioStructure pfio = (pfi_ioStructure)png_get_io_ptr(png_ptr);
	unsigned n = pfio->s_io->read_proc(data, (unsigned int)size, 1, pfio->s_handle);
	if (size && (n == 0)) {
		throw "Read error: invalid or corrupted PNG file";
	}
}

static void
_WriteProc(png_structp png_ptr, unsigned char *data, png_size_t size) {
    pfi_ioStructure pfio = (pfi_ioStructure)png_get_io_ptr(png_ptr);
    pfio->s_io->write_proc(data, (unsigned int)size, 1, pfio->s_handle);
}

static void
_FlushProc(png_structp png_ptr) {
	(png_structp)png_ptr;
	// empty flush implementation
}

static void
error_handler(png_structp png_ptr, const char *error) {
	FreeImage_OutputMessageProc(s_format_id, error);
	png_longjmp(png_ptr, 1);
}

// in FreeImage warnings disabled

static void
warning_handler(png_structp png_ptr, const char *warning) {
	(png_structp)png_ptr;
	(char*)warning;
}

// ==========================================================
// Metadata routines
// ==========================================================

static FIBOOL 
ReadMetadata(png_structp png_ptr, png_infop info_ptr, FIBITMAP *dib) {
	// XMP keyword
	const char *g_png_xmp_keyword = "XML:com.adobe.xmp";

	png_textp text_ptr{};
	png_timep mod_time{};
	int num_text = 0;

	// iTXt/tEXt/zTXt chuncks
	if (png_get_text(png_ptr, info_ptr, &text_ptr, &num_text) > 0) {
		for (int i = 0; i < num_text; i++) {
			// create a tag
			std::unique_ptr<FITAG, decltype(&FreeImage_DeleteTag)> tag(FreeImage_CreateTag(), &FreeImage_DeleteTag);
			if (!tag) return FALSE;

			uint32_t tag_length = (uint32_t) MAX(text_ptr[i].text_length, text_ptr[i].itxt_length);

			FreeImage_SetTagLength(tag.get(), tag_length);
			FreeImage_SetTagCount(tag.get(), tag_length);
			FreeImage_SetTagType(tag.get(), FIDT_ASCII);
			FreeImage_SetTagValue(tag.get(), text_ptr[i].text);

			if (strcmp(text_ptr[i].key, g_png_xmp_keyword) == 0) {
				// store the tag as XMP
				FreeImage_SetTagKey(tag.get(), g_TagLib_XMPFieldName);
				FreeImage_SetMetadata(FIMD_XMP, dib, FreeImage_GetTagKey(tag.get()), tag.get());
			} else {
				// store the tag as a comment
				FreeImage_SetTagKey(tag.get(), text_ptr[i].key);
				FreeImage_SetMetadata(FIMD_COMMENTS, dib, FreeImage_GetTagKey(tag.get()), tag.get());
			}
		}
	}

	// timestamp chunk
	if (png_get_tIME(png_ptr, info_ptr, &mod_time)) {
		char timestamp[32];
		// create a tag
		std::unique_ptr<FITAG, decltype(&FreeImage_DeleteTag)> tag(FreeImage_CreateTag(), &FreeImage_DeleteTag);
		if (!tag) return FALSE;

		// convert as 'yyyy:MM:dd hh:mm:ss'
		snprintf(timestamp, std::size(timestamp), "%4d:%02d:%02d %2d:%02d:%02d", mod_time->year, mod_time->month, mod_time->day, mod_time->hour, mod_time->minute, mod_time->second);

		uint32_t tag_length = (uint32_t)strlen(timestamp) + 1;
		FreeImage_SetTagLength(tag.get(), tag_length);
		FreeImage_SetTagCount(tag.get(), tag_length);
		FreeImage_SetTagType(tag.get(), FIDT_ASCII);
		FreeImage_SetTagID(tag.get(), TAG_DATETIME);
		FreeImage_SetTagValue(tag.get(), timestamp);

		// store the tag as Exif-TIFF
		FreeImage_SetTagKey(tag.get(), "DateTime");
		FreeImage_SetMetadata(FIMD_EXIF_MAIN, dib, FreeImage_GetTagKey(tag.get()), tag.get());
	}

	return TRUE;
}

static FIBOOL 
WriteMetadata(png_structp png_ptr, png_infop info_ptr, FIBITMAP *dib) {
	// XMP keyword
	const char *g_png_xmp_keyword = "XML:com.adobe.xmp";

	FITAG *tag{};
	FIMETADATA *mdhandle{};
	FIBOOL bResult = TRUE;

	png_text text_metadata;
	png_time mod_time;

	// set the 'Comments' metadata as iTXt chuncks

	mdhandle = FreeImage_FindFirstMetadata(FIMD_COMMENTS, dib, &tag);

	if (mdhandle) {
		do {
			memset(&text_metadata, 0, sizeof(png_text));
			text_metadata.compression = 1;							// iTXt, none
			text_metadata.key = (char*)FreeImage_GetTagKey(tag);	// keyword, 1-79 character description of "text"
			text_metadata.text = (char*)FreeImage_GetTagValue(tag);	// comment, may be an empty string (ie "")
			text_metadata.text_length = FreeImage_GetTagLength(tag);// length of the text string
			text_metadata.itxt_length = FreeImage_GetTagLength(tag);// length of the itxt string
			text_metadata.lang = 0;		 // language code, 0-79 characters or a NULL pointer
			text_metadata.lang_key = 0;	 // keyword translated UTF-8 string, 0 or more chars or a NULL pointer

			// set the tag 
			png_set_text(png_ptr, info_ptr, &text_metadata, 1);

		} while (FreeImage_FindNextMetadata(mdhandle, &tag));

		FreeImage_FindCloseMetadata(mdhandle);
		bResult &= TRUE;
	}

	// set the 'XMP' metadata as iTXt chuncks
	tag = nullptr;
	FreeImage_GetMetadata(FIMD_XMP, dib, g_TagLib_XMPFieldName, &tag);
	if (tag && FreeImage_GetTagLength(tag)) {
		memset(&text_metadata, 0, sizeof(png_text));
		text_metadata.compression = 1;							// iTXt, none
		text_metadata.key = (char*)g_png_xmp_keyword;			// keyword, 1-79 character description of "text"
		text_metadata.text = (char*)FreeImage_GetTagValue(tag);	// comment, may be an empty string (ie "")
		text_metadata.text_length = FreeImage_GetTagLength(tag);// length of the text string
		text_metadata.itxt_length = FreeImage_GetTagLength(tag);// length of the itxt string
		text_metadata.lang = 0;		 // language code, 0-79 characters or a NULL pointer
		text_metadata.lang_key = 0;	 // keyword translated UTF-8 string, 0 or more chars or a NULL pointer

		// set the tag 
		png_set_text(png_ptr, info_ptr, &text_metadata, 1);
		bResult &= TRUE;
	}

	// set the Exif-TIFF 'DateTime' metadata as a tIME chunk
	tag = nullptr;
	FreeImage_GetMetadata(FIMD_EXIF_MAIN, dib, "DateTime", &tag);
	if (tag && FreeImage_GetTagLength(tag)) {
		int year, month, day, hour, minute, second;
		const char *value = (char*)FreeImage_GetTagValue(tag);
		if (sscanf(value, "%4d:%02d:%02d %2d:%02d:%02d", &year, &month, &day, &hour, &minute, &second) == 6) {
			mod_time.year	= (png_uint_16)year;
			mod_time.month	= (png_byte)month;
			mod_time.day	= (png_byte)day;
			mod_time.hour	= (png_byte)hour;
			mod_time.minute	= (png_byte)minute;
			mod_time.second	= (png_byte)second;
			png_set_tIME (png_ptr, info_ptr, &mod_time);
		}
	}

	return bResult;
}

// ==========================================================
// Plugin Implementation
// ==========================================================

static const char * DLL_CALLCONV
Format() {
	return "PNG";
}

static const char * DLL_CALLCONV
Description() {
	return "Portable Network Graphics";
}

static const char * DLL_CALLCONV
Extension() {
	return "png";
}

static const char * DLL_CALLCONV
RegExpr() {
	return "^.PNG\r";
}

static const char * DLL_CALLCONV
MimeType() {
	return "image/png";
}

static FIBOOL DLL_CALLCONV
Validate(FreeImageIO *io, fi_handle handle) {
	const uint8_t png_signature[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
	uint8_t signature[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	io->read_proc(&signature, 1, 8, handle);

	return (memcmp(png_signature, signature, 8) == 0);
}

static FIBOOL DLL_CALLCONV
SupportsExportDepth(int depth) {
	return (
			(depth == 1) ||
			(depth == 4) ||
			(depth == 8) ||
			(depth == 24) ||
			(depth == 32)
		);
}

static FIBOOL DLL_CALLCONV 
SupportsExportType(FREE_IMAGE_TYPE type) {
	return (
		(type == FIT_BITMAP) ||
		(type == FIT_UINT16) ||
		(type == FIT_RGB16) ||
		(type == FIT_RGBA16)
	);
}

static FIBOOL DLL_CALLCONV
SupportsICCProfiles() {
	return TRUE;
}

static FIBOOL DLL_CALLCONV
SupportsNoPixels() {
	return TRUE;
}

// --------------------------------------------------------------------------

/**
Configure the decoder so that decoded pixels are compatible with a FREE_IMAGE_TYPE format. 
Set conversion instructions as needed. 
@param png_ptr PNG handle
@param info_ptr PNG info handle
@param flags Decoder flags
@param output_image_type Returned FreeImage converted image type
@return Returns TRUE if successful, returns FALSE otherwise
@see png_read_update_info
*/
static FIBOOL 
ConfigureDecoder(png_structp png_ptr, png_infop info_ptr, int flags, FREE_IMAGE_TYPE *output_image_type) {
	// get original image info
	const int color_type = png_get_color_type(png_ptr, info_ptr);
	const int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	const int pixel_depth = bit_depth * png_get_channels(png_ptr, info_ptr);

	FREE_IMAGE_TYPE image_type = FIT_BITMAP;	// assume standard image type

	// check for transparency table or single transparent color
	FIBOOL bIsTransparent = png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) == PNG_INFO_tRNS ? TRUE : FALSE;

	// check allowed combinations of colour type and bit depth
	// then get converted FreeImage type

	switch (color_type) {
		case PNG_COLOR_TYPE_GRAY:		// color type '0', bitdepth = 1, 2, 4, 8, 16
			switch (bit_depth) {
				case 1:
				case 2:
				case 4:
				case 8:
					// expand grayscale images to the full 8-bit from 2-bit/pixel
					if (pixel_depth == 2) {
						png_set_expand_gray_1_2_4_to_8(png_ptr);
					}

					// if a tRNS chunk is provided, we must also expand the grayscale data to 8-bits,
					// this allows us to make use of the transparency table with existing FreeImage methods
					if (bIsTransparent && (pixel_depth < 8)) {
						png_set_expand_gray_1_2_4_to_8(png_ptr);
					}
					break;

				case 16:
					image_type = (pixel_depth == 16) ? FIT_UINT16 : FIT_UNKNOWN;

					// 16-bit grayscale images can contain a transparent value (shade)
					// if found, expand the transparent value to a full alpha channel
					if (bIsTransparent && (image_type != FIT_UNKNOWN)) {
						// expand tRNS to a full alpha channel
						png_set_tRNS_to_alpha(png_ptr);
						
						// expand new 16-bit gray + 16-bit alpha to full 64-bit RGBA
						png_set_gray_to_rgb(png_ptr);

						image_type = FIT_RGBA16;
					}
					break;

				default:
					image_type = FIT_UNKNOWN;
					break;
			}
			break;

		case PNG_COLOR_TYPE_RGB:		// color type '2', bitdepth = 8, 16
			switch (bit_depth) {
				case 8:
					image_type = (pixel_depth == 24) ? FIT_BITMAP : FIT_UNKNOWN;
					break;
				case 16:
					image_type = (pixel_depth == 48) ? FIT_RGB16 : FIT_UNKNOWN;
					break;
				default:
					image_type = FIT_UNKNOWN;
					break;
			}
			// sometimes, 24- or 48-bit images may contain transparency information
			// check for this use case and convert to an alpha-compatible format
			if (bIsTransparent && (image_type != FIT_UNKNOWN)) {
				// if the image is 24-bit RGB, mark it as 32-bit; if it is 48-bit, mark it as 64-bit
				image_type = (pixel_depth == 24) ? FIT_BITMAP : (pixel_depth == 48) ? FIT_RGBA16 : FIT_UNKNOWN;
				// expand tRNS chunk to alpha channel
				png_set_tRNS_to_alpha(png_ptr);
			}
			break;

		case PNG_COLOR_TYPE_PALETTE:	// color type '3', bitdepth = 1, 2, 4, 8
			switch (bit_depth) {
				case 1:
				case 2:
				case 4:
				case 8:
					// expand palette images to the full 8 bits from 2 bits/pixel
					if (pixel_depth == 2) {
						png_set_packing(png_ptr);
					}

					// if a tRNS chunk is provided, we must also expand the palletized data to 8-bits,
					// this allows us to make use of the transparency table with existing FreeImage methods
					if (bIsTransparent && (pixel_depth < 8)) {
						png_set_packing(png_ptr);
					}
					break;

				default:
					image_type = FIT_UNKNOWN;
					break;
			}
			break;

		case PNG_COLOR_TYPE_GRAY_ALPHA:	// color type '4', bitdepth = 8, 16
			switch (bit_depth) {
				case 8:
					// 8-bit grayscale + 8-bit alpha => convert to 32-bit RGBA
					image_type = (pixel_depth == 16) ? FIT_BITMAP : FIT_UNKNOWN;
					break;
				case 16:
					// 16-bit grayscale + 16-bit alpha => convert to 64-bit RGBA
					image_type = (pixel_depth == 32) ? FIT_RGBA16 : FIT_UNKNOWN;
					break;
				default:
					image_type = FIT_UNKNOWN;
					break;
			}
			// expand 8-bit greyscale + 8-bit alpha to 32-bit
			// expand 16-bit greyscale + 16-bit alpha to 64-bit
			png_set_gray_to_rgb(png_ptr);
			break;

		case PNG_COLOR_TYPE_RGB_ALPHA:	// color type '6', bitdepth = 8, 16
			switch (bit_depth) {
				case 8:
					break;
				case 16:
					image_type = (pixel_depth == 64) ? FIT_RGBA16 : FIT_UNKNOWN;
					break;
				default:
					image_type = FIT_UNKNOWN;
					break;
			}
			break;
	}

	// check for unknown or invalid formats
	if (image_type == FIT_UNKNOWN) {
		*output_image_type = image_type;
		return FALSE;
	}

#ifndef FREEIMAGE_BIGENDIAN
	if ((image_type == FIT_UINT16) || (image_type == FIT_RGB16) || (image_type == FIT_RGBA16)) {
		// turn on 16-bit byte swapping
		png_set_swap(png_ptr);
	}
#endif						

#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
	if ((image_type == FIT_BITMAP) && ((color_type == PNG_COLOR_TYPE_RGB) || (color_type == PNG_COLOR_TYPE_RGB_ALPHA))) {
		// flip the RGB pixels to BGR (or RGBA to BGRA)
		png_set_bgr(png_ptr);
	}
#endif

	// gamma correction
	// unlike the example in the libpng documentation, we have *no* idea where
	// this file may have come from--so if it doesn't have a file gamma, don't
	// do any correction ("do no harm")

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_gAMA)) {
		double gamma = 0;
		double screen_gamma = 2.2;

		if (png_get_gAMA(png_ptr, info_ptr, &gamma) && ( flags & PNG_IGNOREGAMMA ) != PNG_IGNOREGAMMA) {
			png_set_gamma(png_ptr, screen_gamma, gamma);
		}
	}

	// all transformations have been registered; now update info_ptr data		
	png_read_update_info(png_ptr, info_ptr);

	// return the output image type
	*output_image_type = image_type;

	return TRUE;
}

static FIBITMAP * DLL_CALLCONV
Load(FreeImageIO *io, fi_handle handle, int page, int flags, void *data) {
	png_uint_32 width, height;
	int color_type;
	int bit_depth;
	int pixel_depth = 0;	// pixel_depth = bit_depth * channels

    fi_ioStructure fio;
    fio.s_handle = handle;
	fio.s_io = io;
    
	if (handle) {
		FIBOOL header_only = (flags & FIF_LOAD_NOPIXELS) == FIF_LOAD_NOPIXELS;

		try {		
			// check to see if the file is in fact a PNG file

			uint8_t png_check[PNG_BYTES_TO_CHECK];

			io->read_proc(png_check, PNG_BYTES_TO_CHECK, 1, handle);

			if (png_sig_cmp(png_check, (png_size_t)0, PNG_BYTES_TO_CHECK) != 0) {
				return nullptr;	// Bad signature
			}
			
			// create the chunk manage structure

			std::unique_ptr<png_struct, std::function<void(png_structp)>> png_ptr(png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, error_handler, warning_handler), [](png_structp v){ png_destroy_read_struct(&v, nullptr, nullptr); });

			if (!png_ptr) {
				return nullptr;
			}

			// create the info structure

			std::unique_ptr<png_info, std::function<void(png_infop)>> info_ptr(png_create_info_struct(png_ptr.get()), [&png_ptr](png_infop v){ png_destroy_info_struct(png_ptr.get(), &v); });

			if (!info_ptr) {
				return nullptr;
			}

			// init the IO

			png_set_read_fn(png_ptr.get(), &fio, _ReadProc);
			
			// PNG errors will be redirected here

			if (setjmp(png_jmpbuf(png_ptr.get()))) {
				// assume error_handler was called before by the PNG library
				throw((const char*)nullptr);
			}

			// because we have already read the signature...

			png_set_sig_bytes(png_ptr.get(), PNG_BYTES_TO_CHECK);

			// read the IHDR chunk

			png_read_info(png_ptr.get(), info_ptr.get());
			png_get_IHDR(png_ptr.get(), info_ptr.get(), &width, &height, &bit_depth, &color_type, nullptr, nullptr, nullptr);

			// configure the decoder

			FREE_IMAGE_TYPE image_type = FIT_BITMAP;

			if (!ConfigureDecoder(png_ptr.get(), info_ptr.get(), flags, &image_type)) {
				throw FI_MSG_ERROR_UNSUPPORTED_FORMAT;
			}

			// update image info

			color_type = png_get_color_type(png_ptr.get(), info_ptr.get());
			bit_depth = png_get_bit_depth(png_ptr.get(), info_ptr.get());
			pixel_depth = bit_depth * png_get_channels(png_ptr.get(), info_ptr.get());

			// create a dib and write the bitmap header
			// set up the dib palette, if needed
			std::unique_ptr<FIBITMAP, decltype(&FreeImage_Unload)> dib(nullptr, &FreeImage_Unload);
			switch (color_type) {
				case PNG_COLOR_TYPE_RGB:
				case PNG_COLOR_TYPE_RGB_ALPHA:
					dib.reset(FreeImage_AllocateHeaderT(header_only, image_type, width, height, pixel_depth, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK));
					break;

				case PNG_COLOR_TYPE_PALETTE:
					dib.reset(FreeImage_AllocateHeaderT(header_only, image_type, width, height, pixel_depth, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK));
					if (dib) {
						png_colorp png_palette{};
						int palette_entries = 0;

						png_get_PLTE(png_ptr.get(),info_ptr.get(), &png_palette, &palette_entries);

						palette_entries = MIN((unsigned)palette_entries, FreeImage_GetColorsUsed(dib.get()));

						// store the palette

						FIRGBA8 *palette = FreeImage_GetPalette(dib.get());
						for (int i = 0; i < palette_entries; i++) {
							palette[i].red   = png_palette[i].red;
							palette[i].green = png_palette[i].green;
							palette[i].blue  = png_palette[i].blue;
						}
					}
					break;

				case PNG_COLOR_TYPE_GRAY:
					dib.reset(FreeImage_AllocateHeaderT(header_only, image_type, width, height, pixel_depth, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK));

					if (dib && (pixel_depth <= 8)) {
						FIRGBA8 *palette = FreeImage_GetPalette(dib.get());
						const int palette_entries = 1 << pixel_depth;

						for (int i = 0; i < palette_entries; i++) {
							palette[i].red   =
							palette[i].green =
							palette[i].blue  = (uint8_t)((i * 255) / (palette_entries - 1));
						}
					}
					break;

				default:
					throw FI_MSG_ERROR_UNSUPPORTED_FORMAT;
			}

			if (!dib) {
				throw FI_MSG_ERROR_DIB_MEMORY;
			}

			// store the transparency table

			if (png_get_valid(png_ptr.get(), info_ptr.get(), PNG_INFO_tRNS)) {
				// array of alpha (transparency) entries for palette
				png_bytep trans_alpha = nullptr;
				// number of transparent entries
				int num_trans = 0;
				// graylevel or color sample values of the single transparent color for non-paletted images
				png_color_16p trans_color = nullptr;

				png_get_tRNS(png_ptr.get(), info_ptr.get(), &trans_alpha, &num_trans, &trans_color);

				if ((color_type == PNG_COLOR_TYPE_GRAY) && trans_color) {
					// single transparent color
					if (trans_color->gray < 256) { 
						uint8_t table[256]; 
						memset(table, 0xFF, 256); 
						table[trans_color->gray] = 0; 
						FreeImage_SetTransparencyTable(dib.get(), table, 256); 
					}
					// check for a full transparency table, too
					else if ((trans_alpha) && (pixel_depth <= 8)) {
						FreeImage_SetTransparencyTable(dib.get(), (uint8_t *)trans_alpha, num_trans);
					}

				} else if ((color_type == PNG_COLOR_TYPE_PALETTE) && trans_alpha) {
					// transparency table
					FreeImage_SetTransparencyTable(dib.get(), (uint8_t *)trans_alpha, num_trans);
				}
			}

			// store the background color (only supported for FIT_BITMAP types)

			if ((image_type == FIT_BITMAP) && png_get_valid(png_ptr.get(), info_ptr.get(), PNG_INFO_bKGD)) {
				// Get the background color to draw transparent and alpha images over.
				// Note that even if the PNG file supplies a background, you are not required to
				// use it - you should use the (solid) application background if it has one.

				png_color_16p image_background = nullptr;
				FIRGBA8 rgbBkColor;

				if (png_get_bKGD(png_ptr.get(), info_ptr.get(), &image_background)) {
					rgbBkColor.red      = (uint8_t)image_background->red;
					rgbBkColor.green    = (uint8_t)image_background->green;
					rgbBkColor.blue     = (uint8_t)image_background->blue;
					rgbBkColor.alpha = 0;

					FreeImage_SetBackgroundColor(dib.get(), &rgbBkColor);
				}
			}

			// get physical resolution

			if (png_get_valid(png_ptr.get(), info_ptr.get(), PNG_INFO_pHYs)) {
				png_uint_32 res_x, res_y;
				
				// we'll overload this var and use 0 to mean no phys data,
				// since if it's not in meters we can't use it anyway

				int res_unit_type = PNG_RESOLUTION_UNKNOWN;

				png_get_pHYs(png_ptr.get(),info_ptr.get(), &res_x, &res_y, &res_unit_type);

				if (res_unit_type == PNG_RESOLUTION_METER) {
					FreeImage_SetDotsPerMeterX(dib.get(), res_x);
					FreeImage_SetDotsPerMeterY(dib.get(), res_y);
				}
			}

			// get possible ICC profile

			if (png_get_valid(png_ptr.get(), info_ptr.get(), PNG_INFO_iCCP)) {
				png_charp profile_name = nullptr;
				png_bytep profile_data = nullptr;
				png_uint_32 profile_length = 0;
				int  compression_type;

				png_get_iCCP(png_ptr.get(), info_ptr.get(), &profile_name, &compression_type, &profile_data, &profile_length);

				// copy ICC profile data (must be done after FreeImage_AllocateHeader)

				FreeImage_CreateICCProfile(dib.get(), profile_data, profile_length);
			}

			// --- header only mode => clean-up and return

			if (header_only) {
				// get possible metadata (it can be located both before and after the image data)
				ReadMetadata(png_ptr.get(), info_ptr.get(), dib.get());
				return dib.release();
			}

			// set the individual row_pointers to point at the correct offsets

			std::unique_ptr<void, decltype(&free)> safeRowPointers(malloc(height * sizeof(png_bytep)), &free);
			if (!safeRowPointers) {
				return nullptr;
			}
			auto **row_pointers = static_cast<png_bytepp>(safeRowPointers.get());

			// read in the bitmap bits via the pointer table
			// allow loading of PNG with minor errors (such as images with several IDAT chunks)

			for (png_uint_32 k = 0; k < height; k++) {
				row_pointers[height - 1 - k] = FreeImage_GetScanLine(dib.get(), k);
			}

			png_set_benign_errors(png_ptr.get(), 1);
			png_read_image(png_ptr.get(), row_pointers);

			// check if the bitmap contains transparency, if so enable it in the header

			if (FreeImage_GetBPP(dib.get()) == 32) {
				FreeImage_SetTransparent(dib.get(), FIC_RGBALPHA == FreeImage_GetColorType(dib.get()));
			}

			// cleanup

			safeRowPointers.reset();
			row_pointers = nullptr;

			// read the rest of the file, getting any additional chunks in info_ptr

			png_read_end(png_ptr.get(), info_ptr.get());

			// get possible metadata (it can be located both before and after the image data)

			ReadMetadata(png_ptr.get(), info_ptr.get(), dib.get());

			return dib.release();

		} catch (const char *text) {
			if (text) {
				FreeImage_OutputMessageProc(s_format_id, text);
			}
			
			return nullptr;
		}
	}

	return nullptr;
}

// --------------------------------------------------------------------------

static FIBOOL DLL_CALLCONV
Save(FreeImageIO *io, FIBITMAP *dib, fi_handle handle, int page, int flags, void *data) {
	png_colorp palette{};
	png_uint_32 width, height;
	FIBOOL has_alpha_channel = FALSE;

	FIRGBA8 *pal;					// pointer to dib palette
	int bit_depth, pixel_depth;		// pixel_depth = bit_depth * channels
	int palette_entries;
	int	interlace_type;

	fi_ioStructure fio;
    fio.s_handle = handle;
	fio.s_io = io;

	if ((dib) && (handle)) {
		try {
			// create the chunk manage structure

			std::unique_ptr<png_struct, std::function<void(png_structp)>> png_ptr(png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, error_handler, warning_handler), [](png_structp v){ png_destroy_write_struct(&v, nullptr); });

			if (!png_ptr)  {
				return FALSE;
			}

			// allocate/initialize the image information data.

			std::unique_ptr<png_info, std::function<void(png_infop)>> info_ptr(png_create_info_struct(png_ptr.get()), [&png_ptr](png_infop v){ png_destroy_info_struct(png_ptr.get(), &v); });

			if (!info_ptr)  {
				return FALSE;
			}

			// Set error handling.  REQUIRED if you aren't supplying your own
			// error handling functions in the png_create_write_struct() call.

			if (setjmp(png_jmpbuf(png_ptr.get())))  {
				// if we get here, we had a problem reading the file
				return FALSE;
			}

			// init the IO

			png_set_write_fn(png_ptr.get(), &fio, _WriteProc, _FlushProc);

			// set physical resolution

			png_uint_32 res_x = (png_uint_32)FreeImage_GetDotsPerMeterX(dib);
			png_uint_32 res_y = (png_uint_32)FreeImage_GetDotsPerMeterY(dib);

			if ((res_x > 0) && (res_y > 0))  {
				png_set_pHYs(png_ptr.get(), info_ptr.get(), res_x, res_y, PNG_RESOLUTION_METER);
			}
	
			// Set the image information here.  Width and height are up to 2^31,
			// bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
			// the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
			// PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
			// or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
			// PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
			// currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED

			width = FreeImage_GetWidth(dib);
			height = FreeImage_GetHeight(dib);
			pixel_depth = FreeImage_GetBPP(dib);

			FIBOOL bInterlaced = FALSE;
			if ((flags & PNG_INTERLACED) == PNG_INTERLACED) {
				interlace_type = PNG_INTERLACE_ADAM7;
				bInterlaced = TRUE;
			} else {
				interlace_type = PNG_INTERLACE_NONE;
			}

			// set the ZLIB compression level or default to PNG default compression level (ZLIB level = 6)
			int zlib_level = flags & 0x0F;
			if ((zlib_level >= 1) && (zlib_level <= 9)) {
				png_set_compression_level(png_ptr.get(), zlib_level);
			} else if ((flags & PNG_Z_NO_COMPRESSION) == PNG_Z_NO_COMPRESSION) {
				png_set_compression_level(png_ptr.get(), Z_NO_COMPRESSION);
			}

			// filtered strategy works better for high color images
			if (pixel_depth >= 16){
				png_set_compression_strategy(png_ptr.get(), Z_FILTERED);
				png_set_filter(png_ptr.get(), 0, PNG_FILTER_NONE|PNG_FILTER_SUB|PNG_FILTER_PAETH);
			} else {
				png_set_compression_strategy(png_ptr.get(), Z_DEFAULT_STRATEGY);
			}

			FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(dib);
			if (image_type == FIT_BITMAP) {
				// standard image type
				bit_depth = (pixel_depth > 8) ? 8 : pixel_depth;
			} else {
				// 16-bit greyscale or 16-bit RGB(A)
				bit_depth = 16;
			}

			// check for transparent images
			FIBOOL bIsTransparent = 
				(image_type == FIT_BITMAP) && FreeImage_IsTransparent(dib) && (FreeImage_GetTransparencyCount(dib) > 0) ? TRUE : FALSE;

			switch (FreeImage_GetColorType(dib)) {
				case FIC_MINISWHITE:
					if (!bIsTransparent) {
						// Invert monochrome files to have 0 as black and 1 as white (no break here)
						png_set_invert_mono(png_ptr.get());
					}
					// (fall through)

				case FIC_MINISBLACK:
					if (!bIsTransparent) {
						png_set_IHDR(png_ptr.get(), info_ptr.get(), width, height, bit_depth, 
							PNG_COLOR_TYPE_GRAY, interlace_type, 
							PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
						break;
					}
					// If a monochrome image is transparent, save it with a palette
					// (fall through)

				case FIC_PALETTE:
				{
					png_set_IHDR(png_ptr.get(), info_ptr.get(), width, height, bit_depth, 
						PNG_COLOR_TYPE_PALETTE, interlace_type, 
						PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

					// set the palette

					palette_entries = 1 << bit_depth;
					palette = (png_colorp)png_malloc(png_ptr.get(), palette_entries * sizeof (png_color));
					pal = FreeImage_GetPalette(dib);

					for (int i = 0; i < palette_entries; i++) {
						palette[i].red   = pal[i].red;
						palette[i].green = pal[i].green;
						palette[i].blue  = pal[i].blue;
					}
					
					png_set_PLTE(png_ptr.get(), info_ptr.get(), palette, palette_entries);

					// You must not free palette here, because png_set_PLTE only makes a link to
					// the palette that you malloced.  Wait until you are about to destroy
					// the png structure.

					break;
				}

				case FIC_RGBALPHA :
					has_alpha_channel = TRUE;

					png_set_IHDR(png_ptr.get(), info_ptr.get(), width, height, bit_depth, 
						PNG_COLOR_TYPE_RGBA, interlace_type, 
						PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
					// flip BGR pixels to RGB
					if (image_type == FIT_BITMAP) {
						png_set_bgr(png_ptr);
					}
#endif
					break;
	
				case FIC_RGB:
					png_set_IHDR(png_ptr.get(), info_ptr.get(), width, height, bit_depth, 
						PNG_COLOR_TYPE_RGB, interlace_type, 
						PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
					// flip BGR pixels to RGB
					if (image_type == FIT_BITMAP) {
						png_set_bgr(png_ptr.get());
					}
#endif
					break;
					
				case FIC_CMYK:
					break;
			}

			// write possible ICC profile

			FIICCPROFILE *iccProfile = FreeImage_GetICCProfile(dib);
			if (iccProfile->size && iccProfile->data) {
				// skip ICC profile check
				png_set_option(png_ptr.get(), PNG_SKIP_sRGB_CHECK_PROFILE, 1);
				png_set_iCCP(png_ptr.get(), info_ptr.get(), "Embedded Profile", 0, (png_const_bytep)iccProfile->data, iccProfile->size);
			}

			// write metadata

			WriteMetadata(png_ptr.get(), info_ptr.get(), dib);

			// Optional gamma chunk is strongly suggested if you have any guess
			// as to the correct gamma of the image.
			// png_set_gAMA(png_ptr, info_ptr, gamma);

			// set the transparency table

			if (bIsTransparent) {
				png_set_tRNS(png_ptr.get(), info_ptr.get(), FreeImage_GetTransparencyTable(dib), FreeImage_GetTransparencyCount(dib), nullptr);
			}

			// set the background color

			if (FreeImage_HasBackgroundColor(dib)) {
				png_color_16 image_background;
				FIRGBA8 rgbBkColor;

				FreeImage_GetBackgroundColor(dib, &rgbBkColor);
				memset(&image_background, 0, sizeof(png_color_16));
				image_background.blue  = rgbBkColor.blue;
				image_background.green = rgbBkColor.green;
				image_background.red   = rgbBkColor.red;
				image_background.index = rgbBkColor.alpha;

				png_set_bKGD(png_ptr.get(), info_ptr.get(), &image_background);
			}
			
			// Write the file header information.

			png_write_info(png_ptr.get(), info_ptr.get());

			// write out the image data

#ifndef FREEIMAGE_BIGENDIAN
			if (bit_depth == 16) {
				// turn on 16 bit byte swapping
				png_set_swap(png_ptr.get());
			}
#endif

			int number_passes = 1;
			if (bInterlaced) {
				number_passes = png_set_interlace_handling(png_ptr.get());
			}

			if ((pixel_depth == 32) && (!has_alpha_channel)) {
				auto buffer = std::make_unique<uint8_t[]>(width * 3);

				// transparent conversion to 24-bit
				// the number of passes is either 1 for non-interlaced images, or 7 for interlaced images
				for (int pass = 0; pass < number_passes; pass++) {
					for (png_uint_32 k = 0; k < height; k++) {
						FreeImage_ConvertLine32To24(buffer.get(), FreeImage_GetScanLine(dib, height - k - 1), width);
						png_write_row(png_ptr.get(), buffer.get());
					}
				}
			} else {
				// the number of passes is either 1 for non-interlaced images, or 7 for interlaced images
				for (int pass = 0; pass < number_passes; pass++) {
					for (png_uint_32 k = 0; k < height; k++) {
						png_write_row(png_ptr.get(), FreeImage_GetScanLine(dib, height - k - 1));
					}
				}
			}

			// It is REQUIRED to call this to finish writing the rest of the file
			// Bug with png_flush

			png_write_end(png_ptr.get(), info_ptr.get());

			// clean up after the write, and free any memory allocated
			if (palette) {
				png_free(png_ptr.get(), palette);
			}

			return TRUE;

		} catch (const char *text) {
			FreeImage_OutputMessageProc(s_format_id, text);
		}
	}

	return FALSE;
}

// ==========================================================
//   Init
// ==========================================================

void DLL_CALLCONV
InitPNG(Plugin *plugin, int format_id) {
	s_format_id = format_id;

	plugin->format_proc = Format;
	plugin->description_proc = Description;
	plugin->extension_proc = Extension;
	plugin->regexpr_proc = RegExpr;
	plugin->open_proc = nullptr;
	plugin->close_proc = nullptr;
	plugin->pagecount_proc = nullptr;
	plugin->pagecapability_proc = nullptr;
	plugin->load_proc = Load;
	plugin->save_proc = Save;
	plugin->validate_proc = Validate;
	plugin->mime_proc = MimeType;
	plugin->supports_export_bpp_proc = SupportsExportDepth;
	plugin->supports_export_type_proc = SupportsExportType;
	plugin->supports_icc_profiles_proc = SupportsICCProfiles;
	plugin->supports_no_pixels_proc = SupportsNoPixels;
}


std::unique_ptr<FIDEPENDENCY> MakePngDependencyInfo() {
	auto info = std::make_unique<FIDEPENDENCY>();
	info->name = "libpng";
	info->fullVersion = PNG_LIBPNG_VER_STRING;
	info->majorVersion = PNG_LIBPNG_VER_MAJOR;
	info->minorVersion = PNG_LIBPNG_VER_MINOR;
	return info;
}
