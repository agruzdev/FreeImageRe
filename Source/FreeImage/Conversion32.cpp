// ==========================================================
// Bitmap conversion routines
//
// Design and implementation by
// - Floris van den Berg (flvdberg@wxs.nl)
// - Herve Drolon (drolon@infonie.fr)
// - Jani Kajala (janik@remedy.fi)
// - Detlev Vendt (detlev.vendt@brillit.de)
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
//  internal conversions X to 32 bits
// ----------------------------------------------------------

void DLL_CALLCONV
FreeImage_ConvertLine1To32(uint8_t *target, uint8_t *source, int width_in_pixels, FIRGBA8 *palette) {
	for (int cols = 0; cols < width_in_pixels; cols++) {
		const int index = ((source[cols >> 3] >> (7 - (cols & 7))) & 1);

		target[FI_RGBA_BLUE]	= palette[index].blue;
		target[FI_RGBA_GREEN]	= palette[index].green;
		target[FI_RGBA_RED]		= palette[index].red;
		target[FI_RGBA_ALPHA]	= 0xFF;
		target += 4;
	}
}

void DLL_CALLCONV
FreeImage_ConvertLine2To32(uint8_t* target, uint8_t* source, int width_in_pixels, FIRGBA8* palette) {
	FreeImage_ConvertLine2To32MapTransparency(target, source, width_in_pixels, palette, nullptr, 0);
}

void DLL_CALLCONV
FreeImage_ConvertLine4To32(uint8_t *target, uint8_t *source, int width_in_pixels, FIRGBA8 *palette) {
	FreeImage_ConvertLine4To32MapTransparency(target, source, width_in_pixels, palette, nullptr, 0);
}

void DLL_CALLCONV
FreeImage_ConvertLine8To32(uint8_t *target, uint8_t *source, int width_in_pixels, FIRGBA8 *palette) {
	for (int cols = 0; cols < width_in_pixels; cols++) {
		const uint8_t idx = source[cols];

		target[FI_RGBA_BLUE]	= palette[idx].blue;
		target[FI_RGBA_GREEN]	= palette[idx].green;
		target[FI_RGBA_RED]		= palette[idx].red;
		target[FI_RGBA_ALPHA]	= 0xFF;
		target += 4;
	}
}

void DLL_CALLCONV
FreeImage_ConvertLine16To32_555(uint8_t *target, uint8_t *source, int width_in_pixels) {
	uint16_t *bits = (uint16_t *)source;

	for (int cols = 0; cols < width_in_pixels; cols++) {
		target[FI_RGBA_RED]   = (uint8_t)((((bits[cols] & FI16_555_RED_MASK) >> FI16_555_RED_SHIFT) * 0xFF) / 0x1F);
		target[FI_RGBA_GREEN] = (uint8_t)((((bits[cols] & FI16_555_GREEN_MASK) >> FI16_555_GREEN_SHIFT) * 0xFF) / 0x1F);
		target[FI_RGBA_BLUE]  = (uint8_t)((((bits[cols] & FI16_555_BLUE_MASK) >> FI16_555_BLUE_SHIFT) * 0xFF) / 0x1F);
		target[FI_RGBA_ALPHA] = 0xFF;
		target += 4;
	}
}

void DLL_CALLCONV
FreeImage_ConvertLine16To32_565(uint8_t *target, uint8_t *source, int width_in_pixels) {
	uint16_t *bits = (uint16_t *)source;

	for (int cols = 0; cols < width_in_pixels; cols++) {
		target[FI_RGBA_RED]   = (uint8_t)((((bits[cols] & FI16_565_RED_MASK) >> FI16_565_RED_SHIFT) * 0xFF) / 0x1F);
		target[FI_RGBA_GREEN] = (uint8_t)((((bits[cols] & FI16_565_GREEN_MASK) >> FI16_565_GREEN_SHIFT) * 0xFF) / 0x3F);
		target[FI_RGBA_BLUE]  = (uint8_t)((((bits[cols] & FI16_565_BLUE_MASK) >> FI16_565_BLUE_SHIFT) * 0xFF) / 0x1F);
		target[FI_RGBA_ALPHA] = 0xFF;
		target += 4;
	}
}
/*
void DLL_CALLCONV
FreeImage_ConvertLine24To32(uint8_t *target, uint8_t *source, int width_in_pixels) {
	for (int cols = 0; cols < width_in_pixels; cols++) {
		*(uint32_t *)target = (*(uint32_t *) source & FI_RGBA_RGB_MASK) | FI_RGBA_ALPHA_MASK;
		target += 4;
		source += 3;
	}
}
*/
/**
This unoptimized version of the conversion function avoid an undetermined bug with VC++ SP6. 
The bug occurs in release mode only, when the image height is equal to 537 
(try e.g. a size of 432x537 to reproduce the bug with the optimized function).
*/
void DLL_CALLCONV
FreeImage_ConvertLine24To32(uint8_t *target, uint8_t *source, int width_in_pixels) {
	for (int cols = 0; cols < width_in_pixels; cols++) {
		target[FI_RGBA_RED]   = source[FI_RGBA_RED];
		target[FI_RGBA_GREEN] = source[FI_RGBA_GREEN];
		target[FI_RGBA_BLUE]  = source[FI_RGBA_BLUE];
		target[FI_RGBA_ALPHA] = 0xFF;
		target += 4;
		source += 3;
	}
}

// ----------------------------------------------------------

void DLL_CALLCONV
FreeImage_ConvertLine1To32MapTransparency(uint8_t *target, uint8_t *source, int width_in_pixels, FIRGBA8 *palette, uint8_t *table, int transparent_pixels) {
	for (int cols = 0; cols < width_in_pixels; cols++) {
		const int index = ((source[cols >> 3] >> (7 - (cols & 7))) & 1);

		target[FI_RGBA_BLUE]	= palette[index].blue;
		target[FI_RGBA_GREEN]	= palette[index].green;
		target[FI_RGBA_RED]		= palette[index].red;
		target[FI_RGBA_ALPHA] = (index < transparent_pixels) ? table[index] : 255;		
		target += 4;
	}
}

void DLL_CALLCONV
FreeImage_ConvertLine2To32MapTransparency(uint8_t* target, uint8_t* source, int width_in_pixels, FIRGBA8* palette, uint8_t* table, int transparent_pixels) {
	for (int cols = 0; cols < width_in_pixels; ++cols) {
		const uint8_t idx = ((source[cols >> 2] >> (6 - ((cols & 3) << 1))) & 3);

		target[FI_RGBA_BLUE] = palette[idx].blue;
		target[FI_RGBA_GREEN] = palette[idx].green;
		target[FI_RGBA_RED] = palette[idx].red;
		target[FI_RGBA_ALPHA] = (idx < transparent_pixels) ? table[idx] : 255;
		target += 4;
	}
}

void DLL_CALLCONV
FreeImage_ConvertLine4To32MapTransparency(uint8_t *target, uint8_t *source, int width_in_pixels, FIRGBA8 *palette, uint8_t *table, int transparent_pixels) {
	for (int cols = 0; cols < width_in_pixels; ++cols) {
		const uint8_t idx = ((source[cols >> 1] >> (4 - ((cols & 1) << 2))) & 0x0F);

		target[FI_RGBA_BLUE] = palette[idx].blue;
		target[FI_RGBA_GREEN] = palette[idx].green;
		target[FI_RGBA_RED] = palette[idx].red;
		target[FI_RGBA_ALPHA]	= (idx < transparent_pixels) ? table[idx] : 255;
		target += 4;
	}
}

void DLL_CALLCONV
FreeImage_ConvertLine8To32MapTransparency(uint8_t *target, uint8_t *source, int width_in_pixels, FIRGBA8 *palette, uint8_t *table, int transparent_pixels) {
	for (int cols = 0; cols < width_in_pixels; cols++) {
		const uint8_t idx = source[cols];

		target[FI_RGBA_BLUE]	= palette[idx].blue;
		target[FI_RGBA_GREEN]	= palette[idx].green;
		target[FI_RGBA_RED]		= palette[idx].red;
		target[FI_RGBA_ALPHA] = (idx < transparent_pixels) ? table[idx] : 255;
		target += 4;
	}
}

// ----------------------------------------------------------

FIBITMAP * DLL_CALLCONV
FreeImage_ConvertTo32Bits(FIBITMAP *dib) {
	if (!FreeImage_HasPixels(dib)) return nullptr;

	const int bpp = FreeImage_GetBPP(dib);
	const FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(dib);
	
	if ((image_type != FIT_BITMAP) && (image_type != FIT_RGB16) && (image_type != FIT_RGBA16)) {
		return nullptr;
	}
	
	const int width = FreeImage_GetWidth(dib);
	const int height = FreeImage_GetHeight(dib);

	if (image_type == FIT_BITMAP) {

		if (bpp == 32) {
			return FreeImage_Clone(dib);
		}

		FIBITMAP *new_dib = FreeImage_Allocate(width, height, 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);
		if (!new_dib) {
			return nullptr;
		}

		// copy metadata from src to dst
		FreeImage_CloneMetadata(new_dib, dib);

		FIBOOL bIsTransparent = FreeImage_IsTransparent(dib);

		switch (bpp) {
			case 1:
			{
				if (bIsTransparent) {
					for (int rows = 0; rows < height; rows++) {
						FreeImage_ConvertLine1To32MapTransparency(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width, FreeImage_GetPalette(dib), FreeImage_GetTransparencyTable(dib), FreeImage_GetTransparencyCount(dib));
					}
				} else {
					for (int rows = 0; rows < height; rows++) {
						FreeImage_ConvertLine1To32(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width, FreeImage_GetPalette(dib));
					}
				}

				return new_dib;
			}

			case 2:
			{
				if (bIsTransparent) {
					for (int rows = 0; rows < height; rows++) {
						FreeImage_ConvertLine2To32MapTransparency(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width, FreeImage_GetPalette(dib), FreeImage_GetTransparencyTable(dib), FreeImage_GetTransparencyCount(dib));
					}
				}
				else {
					for (int rows = 0; rows < height; rows++) {
						FreeImage_ConvertLine2To32(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width, FreeImage_GetPalette(dib));
					}
				}

				return new_dib;
			}

			case 4:
			{
				if (bIsTransparent) {
					for (int rows = 0; rows < height; rows++) {
						FreeImage_ConvertLine4To32MapTransparency(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width, FreeImage_GetPalette(dib), FreeImage_GetTransparencyTable(dib), FreeImage_GetTransparencyCount(dib));
					}
				} else {
					for (int rows = 0; rows < height; rows++) {
						FreeImage_ConvertLine4To32(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width, FreeImage_GetPalette(dib));
					}
				}

				return new_dib;
			}
				
			case 8:
			{
				if (bIsTransparent) {
					for (int rows = 0; rows < height; rows++) {
						FreeImage_ConvertLine8To32MapTransparency(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width, FreeImage_GetPalette(dib), FreeImage_GetTransparencyTable(dib), FreeImage_GetTransparencyCount(dib));
					}
				} else {
					for (int rows = 0; rows < height; rows++) {
						FreeImage_ConvertLine8To32(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width, FreeImage_GetPalette(dib));
					}
				}

				return new_dib;
			}

			case 16:
			{
				for (int rows = 0; rows < height; rows++) {
					if ((FreeImage_GetRedMask(dib) == FI16_565_RED_MASK) && (FreeImage_GetGreenMask(dib) == FI16_565_GREEN_MASK) && (FreeImage_GetBlueMask(dib) == FI16_565_BLUE_MASK)) {
						FreeImage_ConvertLine16To32_565(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width);
					} else {
						// includes case where all the masks are 0
						FreeImage_ConvertLine16To32_555(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width);
					}
				}

				return new_dib;
			}

			case 24:
			{
				for (int rows = 0; rows < height; rows++) {
					FreeImage_ConvertLine24To32(FreeImage_GetScanLine(new_dib, rows), FreeImage_GetScanLine(dib, rows), width);
				}

				return new_dib;
			}
		}

	} else if (image_type == FIT_RGB16) {
		FIBITMAP *new_dib = FreeImage_Allocate(width, height, 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);
		if (!new_dib) {
			return nullptr;
		}

		// copy metadata from src to dst
		FreeImage_CloneMetadata(new_dib, dib);

		const unsigned src_pitch = FreeImage_GetPitch(dib);
		const unsigned dst_pitch = FreeImage_GetPitch(new_dib);
		const uint8_t *src_bits = FreeImage_GetBits(dib);
		uint8_t *dst_bits = FreeImage_GetBits(new_dib);
		for (int rows = 0; rows < height; rows++) {
			const FIRGB16 *src_pixel = (FIRGB16*)src_bits;
			FIRGBA8 *dst_pixel = (FIRGBA8*)dst_bits;
			for (int cols = 0; cols < width; cols++) {
				dst_pixel[cols].red		= (uint8_t)(src_pixel[cols].red   >> 8);
				dst_pixel[cols].green	= (uint8_t)(src_pixel[cols].green >> 8);
				dst_pixel[cols].blue		= (uint8_t)(src_pixel[cols].blue  >> 8);
				dst_pixel[cols].alpha = (uint8_t)0xFF;
			}
			src_bits += src_pitch;
			dst_bits += dst_pitch;
		}

		return new_dib;

	} else if (image_type == FIT_RGBA16) {
		FIBITMAP *new_dib = FreeImage_Allocate(width, height, 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);
		if (!new_dib) {
			return nullptr;
		}

		// copy metadata from src to dst
		FreeImage_CloneMetadata(new_dib, dib);

		const unsigned src_pitch = FreeImage_GetPitch(dib);
		const unsigned dst_pitch = FreeImage_GetPitch(new_dib);
		const uint8_t *src_bits = FreeImage_GetBits(dib);
		uint8_t *dst_bits = FreeImage_GetBits(new_dib);
		for (int rows = 0; rows < height; rows++) {
			const FIRGBA16 *src_pixel = (FIRGBA16*)src_bits;
			FIRGBA8 *dst_pixel = (FIRGBA8*)dst_bits;
			for (int cols = 0; cols < width; cols++) {
				dst_pixel[cols].red		= (uint8_t)(src_pixel[cols].red   >> 8);
				dst_pixel[cols].green	= (uint8_t)(src_pixel[cols].green >> 8);
				dst_pixel[cols].blue		= (uint8_t)(src_pixel[cols].blue  >> 8);
				dst_pixel[cols].alpha = (uint8_t)(src_pixel[cols].alpha >> 8);
			}
			src_bits += src_pitch;
			dst_bits += dst_pitch;
		}

		return new_dib;
	}
	
	return nullptr;
}
