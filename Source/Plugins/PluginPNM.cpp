// ==========================================================
// PNM (PPM, PGM, PBM) Loader and Writer
//
// Design and implementation by
// - Floris van den Berg (flvdberg@wxs.nl)
// - Herve Drolon (drolon@infonie.fr)
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

#include "FreeImage.h"
#include "Utilities.h"

// ==========================================================
// Internal functions
// ==========================================================

/**
Get an integer value from the actual position pointed by handle
*/
static int
GetInt(FreeImageIO *io, fi_handle handle) {
    char c = 0;
	FIBOOL bFirstChar;

    // skip forward to start of next number

	if (!io->read_proc(&c, 1, 1, handle)) {
		throw FI_MSG_ERROR_PARSING;
	}

    while (1) {
        // eat comments

        if (c == '#') {
			// if we're at a comment, read to end of line

            bFirstChar = TRUE;

            while (1) {
				if (!io->read_proc(&c, 1, 1, handle)) {
					throw FI_MSG_ERROR_PARSING;
				}

				if (bFirstChar && c == ' ') {
					// loop off 1 sp after #
					bFirstChar = FALSE;
				} else if (c == '\n') {
					break;
				}
			}
		}

        if (c >= '0' && c <='9') {
			// we've found what we were looking for
            break;
		}

		if (!io->read_proc(&c, 1, 1, handle)) {
			throw FI_MSG_ERROR_PARSING;
		}
    }

    // we're at the start of a number, continue until we hit a non-number

    int i = 0;

    while (1) {
        i = (i * 10) + (c - '0');

		if (!io->read_proc(&c, 1, 1, handle)) {
			throw FI_MSG_ERROR_PARSING;
		}

		if (c < '0' || c > '9') {
			break;
		}
    }

    return i;
}

/**
Read a uint16_t value taking into account the endianess issue
*/
static inline uint16_t 
ReadWord(FreeImageIO *io, fi_handle handle) {
	uint16_t level = 0;
	io->read_proc(&level, 2, 1, handle); 
#ifndef FREEIMAGE_BIGENDIAN
	SwapShort(&level);	// PNM uses the big endian convention
#endif
	return level;
}

/**
Write a uint16_t value taking into account the endianess issue
*/
static inline void 
WriteWord(FreeImageIO *io, fi_handle handle, const uint16_t value) {
	uint16_t level = value;
#ifndef FREEIMAGE_BIGENDIAN
	SwapShort(&level);	// PNM uses the big endian convention
#endif
	io->write_proc(&level, 2, 1, handle);
}


// ==========================================================
// Plugin Interface
// ==========================================================

static int s_format_id;

// ==========================================================
// Plugin Implementation
// ==========================================================

static const char * DLL_CALLCONV
Format() {
	return "PNM";
}

static const char * DLL_CALLCONV
Description() {
	return "Portable Network Media";
}

static const char * DLL_CALLCONV
Extension() {
	return "pbm,pgm,ppm";
}

static const char * DLL_CALLCONV
RegExpr() {
	return nullptr;
}

static const char * DLL_CALLCONV
MimeType() {
	return "image/freeimage-pnm";
}

static FIBOOL DLL_CALLCONV
Validate(FreeImageIO *io, fi_handle handle) {
	const uint8_t pbm_id1[] = { 0x50, 0x31 };
	const uint8_t pbm_id2[] = { 0x50, 0x34 };
	const uint8_t pgm_id1[] = { 0x50, 0x32 };
	const uint8_t pgm_id2[] = { 0x50, 0x35 };
	const uint8_t ppm_id1[] = { 0x50, 0x33 };
	const uint8_t ppm_id2[] = { 0x50, 0x36 };
	uint8_t signature[2] = { 0, 0 };

	io->read_proc(signature, 1, sizeof(pbm_id1), handle);

	if (memcmp(pbm_id1, signature, sizeof(pbm_id1)) == 0)
		return TRUE;

	if (memcmp(pbm_id2, signature, sizeof(pbm_id2)) == 0)
		return TRUE;

	if (memcmp(pgm_id1, signature, sizeof(pgm_id1)) == 0)
		return TRUE;

	if (memcmp(pgm_id2, signature, sizeof(pgm_id2)) == 0)
		return TRUE;

	if (memcmp(ppm_id1, signature, sizeof(ppm_id1)) == 0)
		return TRUE;

	if (memcmp(ppm_id2, signature, sizeof(ppm_id2)) == 0)
		return TRUE;

	return FALSE;
}

static FIBOOL DLL_CALLCONV
SupportsExportDepth(int depth) {
	return (
			(depth == 1) ||
			(depth == 8) ||
			(depth == 24)
		);
}

static FIBOOL DLL_CALLCONV 
SupportsExportType(FREE_IMAGE_TYPE type) {
	return (
		(type == FIT_BITMAP)  ||
		(type == FIT_UINT16)  ||
		(type == FIT_RGB16)
	);
}

static FIBOOL DLL_CALLCONV
SupportsNoPixels() {
	return TRUE;
}

// ----------------------------------------------------------

static FIBITMAP * DLL_CALLCONV
Load(FreeImageIO *io, fi_handle handle, int page, int flags, void *data) {
	char id_one = 0, id_two = 0;
	int x, y;
	FIRGBA8 *pal;	// pointer to dib palette
	int i;

	if (!handle) {
		return nullptr;
	}

	FIBOOL header_only = (flags & FIF_LOAD_NOPIXELS) == FIF_LOAD_NOPIXELS;

	try {
		FREE_IMAGE_TYPE image_type = FIT_BITMAP;	// standard image: 1-, 8-, 24-bit

		// Read the first two bytes of the file to determine the file format
		// "P1" = ascii bitmap, "P2" = ascii greymap, "P3" = ascii pixmap,
		// "P4" = raw bitmap, "P5" = raw greymap, "P6" = raw pixmap

		io->read_proc(&id_one, 1, 1, handle);
		io->read_proc(&id_two, 1, 1, handle);

		if ((id_one != 'P') || (id_two < '1') || (id_two > '6')) {			
			// signature error
			throw FI_MSG_ERROR_MAGIC_NUMBER;
		}

		// Read the header information: width, height and the 'max' value if any

		int width  = GetInt(io, handle);
		int height = GetInt(io, handle);
		int maxval = 1;

		if ((id_two == '2') || (id_two == '5') || (id_two == '3') || (id_two == '6')) {
			maxval = GetInt(io, handle);
			if ((maxval <= 0) || (maxval > 65535)) {
				FreeImage_OutputMessageProc(s_format_id, "Invalid max value : %d", maxval);
				throw (const char*)nullptr;
			}
		}

		// Create a new DIB
		std::unique_ptr<FIBITMAP, decltype(&FreeImage_Unload)> dib(nullptr, &FreeImage_Unload);
		switch (id_two) {
			case '1':
			case '4':
				// 1-bit
				dib.reset(FreeImage_AllocateHeader(header_only, width, height, 1));
				break;

			case '2':
			case '5':
				if (maxval > 255) {
					// 16-bit greyscale
					image_type = FIT_UINT16;
					dib.reset(FreeImage_AllocateHeaderT(header_only, image_type, width, height));
				} else {
					// 8-bit greyscale
					dib.reset(FreeImage_AllocateHeader(header_only, width, height, 8));
				}
				break;

			case '3':
			case '6':
				if (maxval > 255) {
					// 48-bit RGB
					image_type = FIT_RGB16;
					dib.reset(FreeImage_AllocateHeaderT(header_only, image_type, width, height));
				} else {
					// 24-bit RGB
					dib.reset(FreeImage_AllocateHeader(header_only, width, height, 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK));
				}
				break;
		}

		if (!dib) {
			throw FI_MSG_ERROR_DIB_MEMORY;
		}

		// Build a greyscale palette if needed

		if (image_type == FIT_BITMAP) {
			switch (id_two)  {
				case '1':
				case '4':
					pal = FreeImage_GetPalette(dib.get());
					pal[0].red = pal[0].green = pal[0].blue = 0;
					pal[1].red = pal[1].green = pal[1].blue = 255;
					break;

				case '2':
				case '5':
					pal = FreeImage_GetPalette(dib.get());
					for (i = 0; i < 256; i++) {
						pal[i].red	=
						pal[i].green =
						pal[i].blue	= (uint8_t)i;
					}
					break;

				default:
					break;
			}
		}

		if (header_only) {
			// header only mode
			return dib.release();
		}

		// Read the image...

		switch (id_two)  {
			case '1':
			case '4':
				// write the bitmap data

				if (id_two == '1') {	// ASCII bitmap
					for (y = 0; y < height; y++) {		
						uint8_t *bits = FreeImage_GetScanLine(dib.get(), height - 1 - y);

						for (x = 0; x < width; x++) {
							if (GetInt(io, handle) == 0)
								bits[x >> 3] |= (0x80 >> (x & 0x7));
							else
								bits[x >> 3] &= (0xFF7F >> (x & 0x7));
						}
					}
				}  else {		// Raw bitmap
					int line = CalculateLine(width, 1);

					for (y = 0; y < height; y++) {	
						uint8_t *bits = FreeImage_GetScanLine(dib.get(), height - 1 - y);

						for (x = 0; x < line; x++) {
							io->read_proc(&bits[x], 1, 1, handle);

							bits[x] = ~bits[x];
						}
					}
				}

				return dib.release();

			case '2':
			case '5':
				if (image_type == FIT_BITMAP) {
					// write the bitmap data

					if (id_two == '2') {		// ASCII greymap
						int level = 0;

						for (y = 0; y < height; y++) {	
							uint8_t *bits = FreeImage_GetScanLine(dib.get(), height - 1 - y);

							for (x = 0; x < width; x++) {
								level = GetInt(io, handle);
								bits[x] = (uint8_t)((255 * level) / maxval);
							}
						}
					} else {		// Raw greymap
						uint8_t level = 0;

						for (y = 0; y < height; y++) {		
							uint8_t *bits = FreeImage_GetScanLine(dib.get(), height - 1 - y);

							for (x = 0; x < width; x++) {
								io->read_proc(&level, 1, 1, handle);
								bits[x] = (uint8_t)((255 * (int)level) / maxval);
							}
						}
					}
				}
				else if (image_type == FIT_UINT16) {
					// write the bitmap data

					if (id_two == '2') {		// ASCII greymap
						int level = 0;

						for (y = 0; y < height; y++) {	
							auto *bits = (uint16_t*)FreeImage_GetScanLine(dib.get(), height - 1 - y);

							for (x = 0; x < width; x++) {
								level = GetInt(io, handle);
								bits[x] = (uint16_t)((65535 * (double)level) / maxval);
							}
						}
					} else {		// Raw greymap
						uint16_t level = 0;

						for (y = 0; y < height; y++) {		
							auto *bits = (uint16_t*)FreeImage_GetScanLine(dib.get(), height - 1 - y);

							for (x = 0; x < width; x++) {
								level = ReadWord(io, handle);
								bits[x] = (uint16_t)((65535 * (double)level) / maxval);
							}
						}
					}
				}

				return dib.release();

			case '3':
			case '6':
				if (image_type == FIT_BITMAP) {
					// write the bitmap data

					if (id_two == '3') {		// ASCII pixmap
						int level = 0;

						for (y = 0; y < height; y++) {	
							uint8_t *bits = FreeImage_GetScanLine(dib.get(), height - 1 - y);

							for (x = 0; x < width; x++) {
								level = GetInt(io, handle);
								bits[FI_RGBA_RED] = (uint8_t)((255 * level) / maxval);		// R
								level = GetInt(io, handle);
								bits[FI_RGBA_GREEN] = (uint8_t)((255 * level) / maxval);	// G
								level = GetInt(io, handle);
								bits[FI_RGBA_BLUE] = (uint8_t)((255 * level) / maxval);	// B

								bits += 3;
							}
						}
					}  else {			// Raw pixmap
						uint8_t level = 0;

						for (y = 0; y < height; y++) {	
							uint8_t *bits = FreeImage_GetScanLine(dib.get(), height - 1 - y);

							for (x = 0; x < width; x++) {
								io->read_proc(&level, 1, 1, handle); 
								bits[FI_RGBA_RED] = (uint8_t)((255 * (int)level) / maxval);	// R

								io->read_proc(&level, 1, 1, handle);
								bits[FI_RGBA_GREEN] = (uint8_t)((255 * (int)level) / maxval);	// G

								io->read_proc(&level, 1, 1, handle);
								bits[FI_RGBA_BLUE] = (uint8_t)((255 * (int)level) / maxval);	// B

								bits += 3;
							}
						}
					}
				}
				else if (image_type == FIT_RGB16) {
					// write the bitmap data

					if (id_two == '3') {		// ASCII pixmap
						int level = 0;

						for (y = 0; y < height; y++) {	
							auto *bits = (FIRGB16*)FreeImage_GetScanLine(dib.get(), height - 1 - y);

							for (x = 0; x < width; x++) {
								level = GetInt(io, handle);
								bits[x].red = (uint16_t)((65535 * (double)level) / maxval);		// R
								level = GetInt(io, handle);
								bits[x].green = (uint16_t)((65535 * (double)level) / maxval);	// G
								level = GetInt(io, handle);
								bits[x].blue = (uint16_t)((65535 * (double)level) / maxval);	// B
							}
						}
					}  else {			// Raw pixmap
						uint16_t level = 0;

						for (y = 0; y < height; y++) {	
							auto *bits = (FIRGB16*)FreeImage_GetScanLine(dib.get(), height - 1 - y);

							for (x = 0; x < width; x++) {
								level = ReadWord(io, handle);
								bits[x].red = (uint16_t)((65535 * (double)level) / maxval);		// R
								level = ReadWord(io, handle);
								bits[x].green = (uint16_t)((65535 * (double)level) / maxval);	// G
								level = ReadWord(io, handle);
								bits[x].blue = (uint16_t)((65535 * (double)level) / maxval);	// B
							}
						}
					}
				}

				return dib.release();
		}

	} catch (const char *text)  {
		if (text) {
			switch (id_two)  {
				case '1':
				case '4':
					FreeImage_OutputMessageProc(s_format_id, text);
					break;

				case '2':
				case '5':
					FreeImage_OutputMessageProc(s_format_id, text);
					break;

				case '3':
				case '6':
					FreeImage_OutputMessageProc(s_format_id, text);
					break;
			}
		}
	}
		
	return nullptr;
}

static FIBOOL DLL_CALLCONV
Save(FreeImageIO *io, FIBITMAP *dib, fi_handle handle, int page, int flags, void *data) {
	// ----------------------------------------------------------
	//   PNM Saving
	// ----------------------------------------------------------
	//
	// Output format :
	//
	// Bit depth		flags			file format
	// -------------    --------------  -----------
	// 1-bit / pixel	PNM_SAVE_ASCII	PBM (P1)
	// 1-bit / pixel	PNM_SAVE_RAW	PBM (P4)
	// 8-bit / pixel	PNM_SAVE_ASCII	PGM (P2)
	// 8-bit / pixel	PNM_SAVE_RAW	PGM (P5)
	// 24-bit / pixel	PNM_SAVE_ASCII	PPM (P3)
	// 24-bit / pixel	PNM_SAVE_RAW	PPM (P6)
	// ----------------------------------------------------------

	int x, y;

	char buffer[256];	// temporary buffer whose size should be enough for what we need

	if (!dib || !handle) return FALSE;
	
	FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(dib);

	int bpp		= FreeImage_GetBPP(dib);
	int width	= FreeImage_GetWidth(dib);
	int height	= FreeImage_GetHeight(dib);

	// Find the appropriate magic number for this file type

	int magic = 0;
	int maxval = 255;

	switch (image_type) {
		case FIT_BITMAP:
			switch (bpp) {
				case 1 :
					magic = 1;	// PBM file (B & W)
					break;
				case 8 :
					magic = 2;	// PGM file	(Greyscale)
					break;

				case 24 :
					magic = 3;	// PPM file (RGB)
					break;

				default:
					return FALSE;	// Invalid bit depth
			}
			break;
		
		case FIT_UINT16:
			magic = 2;	// PGM file	(Greyscale)
			maxval = 65535;
			break;

		case FIT_RGB16:
			magic = 3;	// PPM file (RGB)
			maxval = 65535;
			break;

		default:
			return FALSE;
	}


	if (flags == PNM_SAVE_RAW)
		magic += 3;

	// Write the header info

	snprintf(buffer, std::size(buffer), "P%d\n%d %d\n", magic, width, height);
	io->write_proc(&buffer, (unsigned int)strlen(buffer), 1, handle);

	if (bpp != 1) {
		snprintf(buffer, std::size(buffer), "%d\n", maxval);
		io->write_proc(&buffer, (unsigned int)strlen(buffer), 1, handle);
	}

	// Write the image data
	///////////////////////

	if (image_type == FIT_BITMAP) {
		switch (bpp)  {
			case 24 :            // 24-bit RGB, 3 bytes per pixel
			{
				if (flags == PNM_SAVE_RAW)  {
					for (y = 0; y < height; y++) {
						// write the scanline to disc
						uint8_t *bits = FreeImage_GetScanLine(dib, height - 1 - y);

						for (x = 0; x < width; x++) {
							io->write_proc(&bits[FI_RGBA_RED], 1, 1, handle);	// R
							io->write_proc(&bits[FI_RGBA_GREEN], 1, 1, handle);	// G
							io->write_proc(&bits[FI_RGBA_BLUE], 1, 1, handle);	// B

							bits += 3;
						}
					}
				} else {
					int length = 0;

					for (y = 0; y < height; y++) {
						// write the scanline to disc
						uint8_t *bits = FreeImage_GetScanLine(dib, height - 1 - y);
						
						for (x = 0; x < width; x++) {
							snprintf(buffer, std::size(buffer), "%3d %3d %3d ", bits[FI_RGBA_RED], bits[FI_RGBA_GREEN], bits[FI_RGBA_BLUE]);

							io->write_proc(&buffer, (unsigned int)strlen(buffer), 1, handle);

							length += 12;

							if (length > 58) {
								// No line should be longer than 70 characters
								snprintf(buffer, std::size(buffer), "\n");
								io->write_proc(&buffer, (unsigned int)strlen(buffer), 1, handle);
								length = 0;
							}

							bits += 3;
						}
					}

				}
			}
			break;

			case 8:		// 8-bit greyscale
			{
				if (flags == PNM_SAVE_RAW)  {
					for (y = 0; y < height; y++) {
						// write the scanline to disc
						uint8_t *bits = FreeImage_GetScanLine(dib, height - 1 - y);

						for (x = 0; x < width; x++) {
							io->write_proc(&bits[x], 1, 1, handle);
						}
					}
				} else {
					int length = 0;

					for (y = 0; y < height; y++) {
						// write the scanline to disc
						uint8_t *bits = FreeImage_GetScanLine(dib, height - 1 - y);

						for (x = 0; x < width; x++) {
							snprintf(buffer, std::size(buffer), "%3d ", bits[x]);

							io->write_proc(&buffer, (unsigned int)strlen(buffer), 1, handle);

							length += 4;

							if (length > 66) {
								// No line should be longer than 70 characters
								snprintf(buffer, std::size(buffer), "\n");
								io->write_proc(&buffer, (unsigned int)strlen(buffer), 1, handle);
								length = 0;
							}
						}
					}
				}
			}
			break;

			case 1:		// 1-bit B & W
			{
				int color;

				if (flags == PNM_SAVE_RAW)  {
					for (y = 0; y < height; y++) {
						// write the scanline to disc
						uint8_t *bits = FreeImage_GetScanLine(dib, height - 1 - y);

						for (x = 0; x < (int)FreeImage_GetLine(dib); x++)
							io->write_proc(&bits[x], 1, 1, handle);
					}
				} else  {
					int length = 0;

					for (y = 0; y < height; y++) {
						// write the scanline to disc
						uint8_t *bits = FreeImage_GetScanLine(dib, height - 1 - y);

						for (x = 0; x < (int)FreeImage_GetLine(dib) * 8; x++)	{
							color = (bits[x>>3] & (0x80 >> (x & 0x07))) != 0;

							snprintf(buffer, std::size(buffer), "%c ", color ? '1':'0');

							io->write_proc(&buffer, (unsigned int)strlen(buffer), 1, handle);

							length += 2;

							if (length > 68) {
								// No line should be longer than 70 characters
								snprintf(buffer, std::size(buffer), "\n");
								io->write_proc(&buffer, (unsigned int)strlen(buffer), 1, handle);
								length = 0;
							}
						}
					}
				}
			}
			
			break;
		}
	} // if (FIT_BITMAP)

	else if (image_type == FIT_UINT16) {		// 16-bit greyscale
		if (flags == PNM_SAVE_RAW)  {
			for (y = 0; y < height; y++) {
				// write the scanline to disc
				uint16_t *bits = (uint16_t*)FreeImage_GetScanLine(dib, height - 1 - y);

				for (x = 0; x < width; x++) {
					WriteWord(io, handle, bits[x]);
				}
			}
		} else {
			int length = 0;

			for (y = 0; y < height; y++) {
				// write the scanline to disc
				uint16_t *bits = (uint16_t*)FreeImage_GetScanLine(dib, height - 1 - y);

				for (x = 0; x < width; x++) {
					snprintf(buffer, std::size(buffer), "%5d ", bits[x]);

					io->write_proc(&buffer, (unsigned int)strlen(buffer), 1, handle);

					length += 6;

					if (length > 64) {
						// No line should be longer than 70 characters
						snprintf(buffer, std::size(buffer), "\n");
						io->write_proc(&buffer, (unsigned int)strlen(buffer), 1, handle);
						length = 0;
					}
				}
			}
		}
	}

	else if (image_type == FIT_RGB16) {		// 48-bit RGB
		if (flags == PNM_SAVE_RAW)  {
			for (y = 0; y < height; y++) {
				// write the scanline to disc
				FIRGB16 *bits = (FIRGB16*)FreeImage_GetScanLine(dib, height - 1 - y);

				for (x = 0; x < width; x++) {
					WriteWord(io, handle, bits[x].red);		// R
					WriteWord(io, handle, bits[x].green);	// G
					WriteWord(io, handle, bits[x].blue);	// B
				}
			}
		} else {
			int length = 0;

			for (y = 0; y < height; y++) {
				// write the scanline to disc
				FIRGB16 *bits = (FIRGB16*)FreeImage_GetScanLine(dib, height - 1 - y);
				
				for (x = 0; x < width; x++) {
					snprintf(buffer, std::size(buffer), "%5d %5d %5d ", bits[x].red, bits[x].green, bits[x].blue);

					io->write_proc(&buffer, (unsigned int)strlen(buffer), 1, handle);

					length += 18;

					if (length > 52) {
						// No line should be longer than 70 characters
						snprintf(buffer, std::size(buffer), "\n");
						io->write_proc(&buffer, (unsigned int)strlen(buffer), 1, handle);
						length = 0;
					}
				}
			}

		}
	}

	return TRUE;
}

// ==========================================================
//   Init
// ==========================================================

void DLL_CALLCONV
InitPNM(Plugin *plugin, int format_id) {
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
	plugin->supports_icc_profiles_proc = nullptr;
	plugin->supports_no_pixels_proc = SupportsNoPixels;
}
