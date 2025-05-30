// ==========================================================
// CUT Loader
//
// Design and implementation by
// - Floris van den Berg (flvdberg@wxs.nl)
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
//   Constants + headers
// ----------------------------------------------------------

#ifdef _WIN32
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif

typedef struct tagCUTHEADER {
	uint16_t width;
	uint16_t height;
	int32_t dummy;
} CUTHEADER;

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

// ==========================================================
// Plugin Interface
// ==========================================================

static int s_format_id;

// ==========================================================
// Plugin Implementation
// ==========================================================

static const char * DLL_CALLCONV
Format() {
	return "CUT";
}

static const char * DLL_CALLCONV
Description() {
	return "Dr. Halo";
}

static const char * DLL_CALLCONV
Extension() {
	return "cut";
}

static const char * DLL_CALLCONV
RegExpr() {
	return nullptr;
}

static const char * DLL_CALLCONV
MimeType() {
	return "image/x-cut";
}

static FIBOOL DLL_CALLCONV
Validate(FreeImageIO *io, fi_handle handle) {
	return FALSE;
}

static FIBOOL DLL_CALLCONV
SupportsExportDepth(int depth) {
	return FALSE;
}

static FIBOOL DLL_CALLCONV 
SupportsExportType(FREE_IMAGE_TYPE type) {
	return FALSE;
}

static FIBOOL DLL_CALLCONV
SupportsNoPixels() {
	return TRUE;
}

// ----------------------------------------------------------

static FIBITMAP * DLL_CALLCONV
Load(FreeImageIO *io, fi_handle handle, int page, int flags, void *data) {

	if (!handle) {
		return nullptr;
	}

	try {
		CUTHEADER header;		

		FIBOOL header_only = (flags & FIF_LOAD_NOPIXELS) == FIF_LOAD_NOPIXELS;

		// read the cut header

		if (io->read_proc(&header, 1, sizeof(CUTHEADER), handle) != sizeof(CUTHEADER)) {
			throw FI_MSG_ERROR_PARSING;
		}

#ifdef FREEIMAGE_BIGENDIAN
		SwapShort((uint16_t *)&header.width);
		SwapShort((uint16_t *)&header.height);
#endif

		if ((header.width == 0) || (header.height == 0)) {
			return nullptr;
		}

		// allocate a new bitmap

		std::unique_ptr<FIBITMAP, decltype(&FreeImage_Unload)> dib(FreeImage_AllocateHeader(header_only, header.width, header.height, 8), &FreeImage_Unload);

		if (!dib) {
			throw FI_MSG_ERROR_DIB_MEMORY;
		}

		// stuff it with a palette

		FIRGBA8 *palette = FreeImage_GetPalette(dib.get());

		for (int j = 0; j < 256; ++j) {
			palette[j].blue = palette[j].green = palette[j].red = (uint8_t)j;
		}
		
		if (header_only) {
			// header only mode
			return dib.release();
		}

		// unpack the RLE bitmap bits

		uint8_t *bits = FreeImage_GetScanLine(dib.get(), header.height - 1);

		unsigned i = 0, k = 0;
		unsigned pitch = FreeImage_GetPitch(dib.get());
		unsigned size = header.width * header.height;
		uint8_t count = 0, run = 0;

		while (i < size) {
			if (io->read_proc(&count, 1, sizeof(uint8_t), handle) != 1) {
				throw FI_MSG_ERROR_PARSING;
			}

			if (count == 0) {
				k = 0;
				bits -= pitch;

				// paint shop pro adds two useless bytes here...

				io->read_proc(&count, 1, sizeof(uint8_t), handle);
				io->read_proc(&count, 1, sizeof(uint8_t), handle);

				continue;
			}

			if (count & 0x80) {
				count &= ~(0x80);

				if (io->read_proc(&run, 1, sizeof(uint8_t), handle) != 1) {
					throw FI_MSG_ERROR_PARSING;
				}

				if (k + count <= header.width) {
					memset(bits + k, run, count);
				} else {
					throw FI_MSG_ERROR_PARSING;
				}
			} else {
				if (k + count <= header.width) {
					if (io->read_proc(&bits[k], count, sizeof(uint8_t), handle) != 1) {
						throw FI_MSG_ERROR_PARSING;
					}
				} else {
					throw FI_MSG_ERROR_PARSING;
				}
			}

			k += count;
			i += count;
		}

		return dib.release();

	} catch(const char* text) {
		FreeImage_OutputMessageProc(s_format_id, text);
	}
	return nullptr;
}

// ==========================================================
//   Init
// ==========================================================

void DLL_CALLCONV
InitCUT(Plugin *plugin, int format_id) {
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
	plugin->supports_no_pixels_proc = SupportsNoPixels;
}
