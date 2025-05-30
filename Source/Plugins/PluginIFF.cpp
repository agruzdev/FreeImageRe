// ==========================================================
// Deluxe Paint Loader
//
// Design and implementation by
// - Floris van den Berg (flvdberg@wxs.nl)
// - Mark Sibly (marksibly@blitzbasic.com)
// - Aaron Shumate (trek@startreker.com)
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

// ----------------------------------------------------------
//  Internal typedefs and structures
// ----------------------------------------------------------

#ifdef _WIN32
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif

typedef struct {
    uint16_t w, h;                    /* raster width & height in pixels */
    uint16_t x, y;                    /* position for this image */
    uint8_t nPlanes;                 /* # source bitplanes */
    uint8_t masking;                 /* masking technique */
    uint8_t compression;             /* compression algorithm */
    uint8_t pad1;                    /* UNUSED.  For consistency, put 0 here.*/
    uint16_t transparentColor;        /* transparent "color number" */
    uint8_t xAspect, yAspect;        /* aspect ratio, a rational number x/y */
    uint16_t pageWidth, pageHeight;   /* source "page" size in pixels */
} BMHD;

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

#ifndef FREEIMAGE_BIGENDIAN
static void
SwapHeader(BMHD *header) {
	SwapShort(&header->w);
	SwapShort(&header->h);
	SwapShort(&header->x);
	SwapShort(&header->y);
	SwapShort(&header->transparentColor);
	SwapShort(&header->pageWidth);
	SwapShort(&header->pageHeight);
}
#endif

// ----------------------------------------------------------

/* IFF chunk IDs */

typedef uint32_t IFF_ID;

#define MAKE_ID(a, b, c, d)         ((IFF_ID)(a)<<24 | (IFF_ID)(b)<<16 | (IFF_ID)(c)<<8 | (IFF_ID)(d))

#define ID_FORM     MAKE_ID('F', 'O', 'R', 'M')     /* EA IFF 85 group identifier */
#define ID_CAT      MAKE_ID('C', 'A', 'T', ' ')     /* EA IFF 85 group identifier */
#define ID_LIST     MAKE_ID('L', 'I', 'S', 'T')     /* EA IFF 85 group identifier */
#define ID_PROP     MAKE_ID('P', 'R', 'O', 'P')     /* EA IFF 85 group identifier */
#define ID_END      MAKE_ID('E', 'N', 'D', ' ')     /* unofficial END-of-FORM identifier (see Amiga RKM Devices Ed.3 page 376) */

#define ID_ILBM     MAKE_ID('I', 'L', 'B', 'M')     /* EA IFF 85 raster bitmap form */
#define ID_DEEP     MAKE_ID('D', 'E', 'E', 'P')     /* Chunky pixel image files (Used in TV Paint) */
#define ID_RGB8     MAKE_ID('R', 'G', 'B', '8')     /* RGB image forms, Turbo Silver (Impulse) */
#define ID_RGBN     MAKE_ID('R', 'G', 'B', 'N')     /* RGB image forms, Turbo Silver (Impulse) */
#define ID_PBM      MAKE_ID('P', 'B', 'M', ' ')     /* 256-color chunky format (DPaint 2 ?) */
#define ID_ACBM     MAKE_ID('A', 'C', 'B', 'M')     /* Amiga Contiguous Bitmap (AmigaBasic) */
/* generic */
#define ID_FVER     MAKE_ID('F', 'V', 'E', 'R')     /* AmigaOS version string */
#define ID_JUNK     MAKE_ID('J', 'U', 'N', 'K')     /* always ignore this chunk */
#define ID_ANNO     MAKE_ID('A', 'N', 'N', 'O')     /* EA IFF 85 Generic Annotation chunk */
#define ID_AUTH     MAKE_ID('A', 'U', 'T', 'H')     /* EA IFF 85 Generic Author chunk */
#define ID_CHRS     MAKE_ID('C', 'H', 'R', 'S')     /* EA IFF 85 Generic character string chunk */
#define ID_NAME     MAKE_ID('N', 'A', 'M', 'E')     /* EA IFF 85 Generic Name of art, music, etc. chunk */
#define ID_TEXT     MAKE_ID('T', 'E', 'X', 'T')     /* EA IFF 85 Generic unformatted ASCII text chunk */
#define ID_copy     MAKE_ID('(', 'c', ')', ' ')     /* EA IFF 85 Generic Copyright text chunk */
/* ILBM chunks */
#define ID_BMHD     MAKE_ID('B', 'M', 'H', 'D')     /* ILBM BitmapHeader */
#define ID_CMAP     MAKE_ID('C', 'M', 'A', 'P')     /* ILBM 8bit RGB colormap */
#define ID_GRAB     MAKE_ID('G', 'R', 'A', 'B')     /* ILBM "hotspot" coordiantes */
#define ID_DEST     MAKE_ID('D', 'E', 'S', 'T')     /* ILBM destination image info */
#define ID_SPRT     MAKE_ID('S', 'P', 'R', 'T')     /* ILBM sprite identifier */
#define ID_CAMG     MAKE_ID('C', 'A', 'M', 'G')     /* Amiga viewportmodes */
#define ID_BODY     MAKE_ID('B', 'O', 'D', 'Y')     /* ILBM image data */
#define ID_CRNG     MAKE_ID('C', 'R', 'N', 'G')     /* color cycling */
#define ID_CCRT     MAKE_ID('C', 'C', 'R', 'T')     /* color cycling */
#define ID_CLUT     MAKE_ID('C', 'L', 'U', 'T')     /* Color Lookup Table chunk */
#define ID_DPI      MAKE_ID('D', 'P', 'I', ' ')     /* Dots per inch chunk */
#define ID_DPPV     MAKE_ID('D', 'P', 'P', 'V')     /* DPaint perspective chunk (EA) */
#define ID_DRNG     MAKE_ID('D', 'R', 'N', 'G')     /* DPaint IV enhanced color cycle chunk (EA) */
#define ID_EPSF     MAKE_ID('E', 'P', 'S', 'F')     /* Encapsulated Postscript chunk */
#define ID_CMYK     MAKE_ID('C', 'M', 'Y', 'K')     /* Cyan, Magenta, Yellow, & Black color map (Soft-Logik) */
#define ID_CNAM     MAKE_ID('C', 'N', 'A', 'M')     /* Color naming chunk (Soft-Logik) */
#define ID_PCHG     MAKE_ID('P', 'C', 'H', 'G')     /* Line by line palette control information (Sebastiano Vigna) */
#define ID_PRVW     MAKE_ID('P', 'R', 'V', 'W')     /* A mini duplicate ILBM used for preview (Gary Bonham) */
#define ID_XBMI     MAKE_ID('X', 'B', 'M', 'I')     /* eXtended BitMap Information (Soft-Logik) */
#define ID_CTBL     MAKE_ID('C', 'T', 'B', 'L')     /* Newtek Dynamic Ham color chunk */
#define ID_DYCP     MAKE_ID('D', 'Y', 'C', 'P')     /* Newtek Dynamic Ham chunk */
#define ID_SHAM     MAKE_ID('S', 'H', 'A', 'M')     /* Sliced HAM color chunk */
#define ID_ABIT     MAKE_ID('A', 'B', 'I', 'T')     /* ACBM body chunk */
#define ID_DCOL     MAKE_ID('D', 'C', 'O', 'L')     /* unofficial direct color */

// ==========================================================
// Plugin Interface
// ==========================================================

static int s_format_id;

// ==========================================================
// Plugin Implementation
// ==========================================================

static const char * DLL_CALLCONV
Format() {
	return "IFF";
}

static const char * DLL_CALLCONV
Description() {
	return "IFF Interleaved Bitmap";
}

static const char * DLL_CALLCONV
Extension() {
	return "iff,lbm";
}

static const char * DLL_CALLCONV
RegExpr() {
	return nullptr;
}

static const char * DLL_CALLCONV
MimeType() {
	return "image/x-iff";
}

static FIBOOL DLL_CALLCONV
Validate(FreeImageIO *io, fi_handle handle) {
	uint32_t type = 0;

	// read chunk type
	io->read_proc(&type, 4, 1, handle);
#ifndef FREEIMAGE_BIGENDIAN
	SwapLong(&type);
#endif

	if (type != ID_FORM)
		return FALSE;
		
	// skip 4 bytes
	io->read_proc(&type, 4, 1, handle);

	// read chunk type
	io->read_proc(&type, 4, 1, handle);
#ifndef FREEIMAGE_BIGENDIAN
	SwapLong(&type);
#endif

	// File format : ID_PBM = Packed Bitmap, ID_ILBM = Interleaved Bitmap
	return (type == ID_ILBM) || (type == ID_PBM);
}


static FIBOOL DLL_CALLCONV
SupportsExportDepth(int depth) {
	return FALSE;
}

static FIBOOL DLL_CALLCONV 
SupportsExportType(FREE_IMAGE_TYPE type) {
	return FALSE;
}

// ----------------------------------------------------------

static FIBITMAP * DLL_CALLCONV
Load(FreeImageIO *io, fi_handle handle, int page, int flags, void *data) {
	if (handle) {
		std::unique_ptr<FIBITMAP, decltype(&FreeImage_Unload)> dib(nullptr, &FreeImage_Unload);

		uint32_t type, size;

		io->read_proc(&type, 4, 1, handle);
#ifndef FREEIMAGE_BIGENDIAN
		SwapLong(&type);
#endif

		if (type != ID_FORM)
			return nullptr;

		io->read_proc(&size, 4, 1, handle);
#ifndef FREEIMAGE_BIGENDIAN
		SwapLong(&size);
#endif

		io->read_proc(&type, 4, 1, handle);
#ifndef FREEIMAGE_BIGENDIAN
		SwapLong(&type);
#endif

		if ((type != ID_ILBM) && (type != ID_PBM))
			return nullptr;

		size -= 4;

		unsigned width = 0, height = 0, planes = 0, depth = 0, comp = 0;

		while (size) {
			uint32_t ch_type,ch_size;

			io->read_proc(&ch_type, 4, 1, handle);
#ifndef FREEIMAGE_BIGENDIAN
			SwapLong(&ch_type);
#endif

			io->read_proc(&ch_size,4,1,handle );
#ifndef FREEIMAGE_BIGENDIAN
			SwapLong(&ch_size);
#endif

			unsigned ch_end = io->tell_proc(handle) + ch_size;

			if (ch_type == ID_BMHD) {			// Bitmap Header
				if (dib) {
					dib.reset();
				}

				BMHD bmhd;

				io->read_proc(&bmhd, sizeof(bmhd), 1, handle);
#ifndef FREEIMAGE_BIGENDIAN
				SwapHeader(&bmhd);
#endif

				width = bmhd.w;
				height = bmhd.h;
				planes = bmhd.nPlanes;
				comp = bmhd.compression;

				if (bmhd.masking & 1)
					planes++;	// there is a mask ( 'stencil' )

				if (planes > 8 && planes != 24)
					return nullptr;

				depth = planes > 8 ? 24 : 8;

				if ( depth == 24 ) {
					dib.reset(FreeImage_Allocate(width, height, depth, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK));
				} else {
					dib.reset(FreeImage_Allocate(width, height, depth));
				}
			} else if (ch_type == ID_CMAP) {	// Palette (Color Map)
				if (!dib) {
					return nullptr;
				}

				FIRGBA8 *pal = FreeImage_GetPalette(dib.get());
				if (pal) {
					unsigned palette_entries = MIN((unsigned)ch_size / 3, FreeImage_GetColorsUsed(dib.get()));
					for (unsigned k = 0; k < palette_entries; k++) {
						io->read_proc(&pal[k].red, 1, 1, handle );
						io->read_proc(&pal[k].green, 1, 1, handle );
						io->read_proc(&pal[k].blue, 1, 1, handle );
					}
				}
			} else if (ch_type == ID_BODY) {
				if (!dib) {
					return nullptr;
				}

				if (type == ID_PBM) {
					// NON INTERLACED (LBM)

					unsigned line = FreeImage_GetLine(dib.get()) + 1 & ~1;
					
					for (unsigned i = 0; i < FreeImage_GetHeight(dib.get()); i++) {
						uint8_t *bits = FreeImage_GetScanLine(dib.get(), FreeImage_GetHeight(dib.get()) - i - 1);

						if (comp == 1) {
							// use RLE compression

							uint32_t number_of_bytes_written = 0;
							uint8_t rle_count;
							uint8_t byte;

							while (number_of_bytes_written < line) {
								io->read_proc(&rle_count, 1, 1, handle);

								if (rle_count < 128) {
									for (int k = 0; k < rle_count + 1; k++) {
										io->read_proc(&byte, 1, 1, handle);

										bits[number_of_bytes_written++] += byte;
									}
								} else if (rle_count > 128) {
									io->read_proc(&byte, 1, 1, handle);

									for (int k = 0; k < 257 - rle_count; k++) {
										bits[number_of_bytes_written++] += byte;
									}
								}
							}
						} else {
							// don't use compression

							io->read_proc(bits, line, 1, handle);
						}
					}

					return dib.release();
				} else {
					// INTERLACED (ILBM)

					unsigned pixel_size = depth/8;
					unsigned n_width=(width+15)&~15;
					unsigned plane_size = n_width/8;
					unsigned src_size = plane_size * planes;
					auto src(std::make_unique<uint8_t[]>(src_size));
					uint8_t *dest = FreeImage_GetBits(dib.get());

					dest += FreeImage_GetPitch(dib.get()) * height;

					for (unsigned y = 0; y < height; y++) {
						dest -= FreeImage_GetPitch(dib.get());

						// read all planes in one hit,
						// 'coz PSP compresses across planes...

						if (comp) {
							// unpacker algorithm

							for (unsigned x = 0; x < src_size;) {
								// read the next source byte into t
								signed char t = 0;
								io->read_proc(&t, 1, 1, handle);
								
								if (t >= 0) {
									// t = [0..127] => copy the next t+1 bytes literally
									unsigned size_to_read = t + 1;

									if ((size_to_read + x) > src_size) {
										// sanity check for buffer overruns 
										size_to_read = src_size - x;
										io->read_proc(src.get() + x, size_to_read, 1, handle);
										x += (t + 1);
									} else {
										io->read_proc(src.get() + x, size_to_read, 1, handle);
										x += size_to_read;
									}
								} else if (t != -128) {
									// t = [-1..-127]  => replicate the next byte -t+1 times
									uint8_t b = 0;
									io->read_proc(&b, 1, 1, handle);
									unsigned size_to_copy = (unsigned)(-(int)t + 1);

									if ((size_to_copy + x) > src_size) {
										// sanity check for buffer overruns 
										size_to_copy = src_size - x;
										memset(src.get() + x, b, size_to_copy);
										x += (unsigned)(-(int)t + 1);
									} else {
										memset(src.get() + x, b, size_to_copy);
										x += size_to_copy;
									}
								}
								// t = -128 => noop
							}
						} else {
							io->read_proc(src.get(), src_size, 1, handle);
						}

						// lazy planar->chunky...

						for (unsigned x = 0; x < width; x++) {
							for (unsigned n = 0; n < planes; n++) {
								uint8_t bit = (uint8_t)( src[n * plane_size + (x / 8)] >> ((x^7) & 7) );

								dest[x * pixel_size + (n / 8)] |= (bit & 1) << (n & 7);
							}
						}

#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
						if (depth == 24) {
							for (unsigned x = 0; x < width; ++x){
								INPLACESWAP(dest[x * 3], dest[x * 3 + 2]);
							}
						}
#endif
					}

					return dib.release();
				}
			}

			// Every odd-length chunk is followed by a 0 pad byte.  This pad
			//  byte is not counted in ch_size.
			if (ch_size & 1) {
				ch_size++;
				ch_end++;
			}

			io->seek_proc(handle, ch_end - io->tell_proc(handle), SEEK_CUR);

			size -= ch_size + 8;
		}
	}

	return nullptr;
}

// ==========================================================
//   Init
// ==========================================================

void DLL_CALLCONV
InitIFF(Plugin *plugin, int format_id) {
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
	plugin->save_proc = nullptr;
	plugin->validate_proc = Validate;
	plugin->mime_proc = MimeType;
	plugin->supports_export_bpp_proc = SupportsExportDepth;
	plugin->supports_export_type_proc = SupportsExportType;
	plugin->supports_icc_profiles_proc = nullptr;
}
