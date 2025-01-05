// ==========================================================
// PFM Loader and Writer
//
// Design and implementation by
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

/** maximum size of a line in the header */
#define PFM_MAXLINE	256

/** Big endian / Little endian float conversion */
#define REVERSEBYTES(source, dest)		\
{										\
	char *j = (char *) source;			\
	char *dj = (char *) dest;			\
	dj[0] = j[3];						\
	dj[1] = j[2];						\
	dj[2] = j[1];						\
	dj[3] = j[0];						\
}

/**
Get a line from a ASCII io stream
*/
static FIBOOL 
pfm_get_line(FreeImageIO *io, fi_handle handle, char *buffer, int length) {
	int i;
	memset(buffer, 0, length);
	for (i = 0; i < length; i++) {
		if (!io->read_proc(&buffer[i], 1, 1, handle))
			return FALSE;
		if (buffer[i] == 0x0A)
			break;
	}
	
	return (i < length) ? TRUE : FALSE;
}

/**
Get an integer value from the actual position pointed by handle
*/
static int
pfm_get_int(FreeImageIO *io, fi_handle handle) {
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

// ==========================================================
// Plugin Interface
// ==========================================================

static int s_format_id;

// ==========================================================
// Plugin Implementation
// ==========================================================

static const char * DLL_CALLCONV
Format() {
	return "PFM";
}

static const char * DLL_CALLCONV
Description() {
	return "Portable floatmap";
}

static const char * DLL_CALLCONV
Extension() {
	return "pfm";
}

static const char * DLL_CALLCONV
RegExpr() {
	return nullptr;
}

static const char * DLL_CALLCONV
MimeType() {
	return "image/x-portable-floatmap";
}

static FIBOOL DLL_CALLCONV
Validate(FreeImageIO *io, fi_handle handle) {
	const uint8_t pfm_id1[] = { 0x50, 0x46 };
	const uint8_t pfm_id2[] = { 0x50, 0x66 };
	uint8_t signature[2] = { 0, 0 };

	io->read_proc(signature, 1, sizeof(pfm_id1), handle);

	if (memcmp(pfm_id1, signature, sizeof(pfm_id1)) == 0)
		return TRUE;

	if (memcmp(pfm_id2, signature, sizeof(pfm_id2)) == 0)
		return TRUE;

	return FALSE;
}

static FIBOOL DLL_CALLCONV
SupportsExportDepth(int depth) {
	return FALSE;
}

static FIBOOL DLL_CALLCONV 
SupportsExportType(FREE_IMAGE_TYPE type) {
	return (
		(type == FIT_FLOAT) ||
		(type == FIT_RGBF)
	);
}

static FIBOOL DLL_CALLCONV
SupportsNoPixels() {
	return TRUE;
}

// ----------------------------------------------------------

static FIBITMAP * DLL_CALLCONV
Load(FreeImageIO *io, fi_handle handle, int page, int flags, void *data) {
	char line_buffer[PFM_MAXLINE];
	char id_one = 0, id_two = 0;

	if (!handle) {
		return nullptr;
	}

	FIBOOL header_only = (flags & FIF_LOAD_NOPIXELS) == FIF_LOAD_NOPIXELS;

	try {
		FREE_IMAGE_TYPE image_type = FIT_UNKNOWN;

		// Read the first two bytes of the file to determine the file format
		// "PF" = color image
		// "Pf" = greyscale image

		io->read_proc(&id_one, 1, 1, handle);
		io->read_proc(&id_two, 1, 1, handle);

		if (id_one == 'P') {
			if (id_two == 'F') {
				image_type = FIT_RGBF;
			} else if (id_two == 'f') {
				image_type = FIT_FLOAT;
			}
		}
		if (image_type == FIT_UNKNOWN) {
			// signature error
			throw FI_MSG_ERROR_MAGIC_NUMBER;
		}

		// Read the header information: width, height and the scale value
		unsigned width  = (unsigned) pfm_get_int(io, handle);
		unsigned height = (unsigned) pfm_get_int(io, handle);
		float scalefactor = 1;

		FIBOOL bResult = pfm_get_line(io, handle, line_buffer, PFM_MAXLINE);
		if (bResult) {
			bResult = (sscanf(line_buffer, "%f", &scalefactor) == 1) ? TRUE : FALSE;
		}
		if (!bResult) {
			throw "Read error: invalid PFM header";
		}

		// Create a new DIB
		std::unique_ptr<FIBITMAP, decltype(&FreeImage_Unload)> dib(FreeImage_AllocateHeaderT(header_only, image_type, width, height), &FreeImage_Unload);
		if (!dib) {
			throw FI_MSG_ERROR_DIB_MEMORY;
		}

		if (header_only) {
			// header only mode
			return dib.release();
		}

		// Read the image...

		if (image_type == FIT_RGBF) {
			const unsigned lineWidth = 3 * width;
			auto lineBuffer(std::make_unique<float[]>(lineWidth));

			for (unsigned y = 0; y < height; y++) {	
				FIRGBF *bits = (FIRGBF*)FreeImage_GetScanLine(dib.get(), height - 1 - y);

				if (io->read_proc(lineBuffer.get(), sizeof(float), lineWidth, handle) != lineWidth) {
					throw "Read error";
				}
				const auto *channel = lineBuffer.get();
				if (scalefactor > 0) {
					// MSB
					for (unsigned x = 0; x < width; x++) {
						REVERSEBYTES(channel++, &bits[x].red);
						REVERSEBYTES(channel++, &bits[x].green);
						REVERSEBYTES(channel++, &bits[x].blue);
					}
				} else {
					// LSB					
					for (unsigned x = 0; x < width; x++) {
						bits[x].red		= *channel++;
						bits[x].green	= *channel++;
						bits[x].blue	= *channel++;
					}
				}
			}
		} else if (image_type == FIT_FLOAT) {
			const unsigned lineWidth = width;
			auto lineBuffer(std::make_unique<float[]>(lineWidth));

			for (unsigned y = 0; y < height; y++) {
				float *bits = (float*)FreeImage_GetScanLine(dib.get(), height - 1 - y);

				if (io->read_proc(lineBuffer.get(), sizeof(float), lineWidth, handle) != lineWidth) {
					throw "Read error";
				}
				auto *channel = static_cast<const float *>(lineBuffer.get());
				if (scalefactor > 0) {
					// MSB - File is Big endian
					for (unsigned x = 0; x < width; x++) {
						REVERSEBYTES(channel++, &bits[x]);
					}
				} else {
					// LSB - File is Little Endian
					for (unsigned x = 0; x < width; x++) {
						bits[x] = *channel++;
					}
				}
			}
		}
		
		return dib.release();

	}
	catch (const char *text)  {
		if (text) {
			FreeImage_OutputMessageProc(s_format_id, text);
		}
	}
	catch (const std::bad_alloc &) {
		FreeImage_OutputMessageProc(s_format_id, FI_MSG_ERROR_MEMORY);
	}
	return nullptr;
}

static FIBOOL DLL_CALLCONV
Save(FreeImageIO *io, FIBITMAP *dib, fi_handle handle, int page, int flags, void *data) {
	if (!dib || !handle) return FALSE;

	const FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(dib);
	if ((image_type != FIT_RGBF) && (image_type != FIT_FLOAT)) {
		return FALSE;
	}

	const unsigned width  = FreeImage_GetWidth(dib);
	const unsigned height = FreeImage_GetHeight(dib);
	const unsigned lineWidth = FreeImage_GetLine(dib);
	
	// save image as Little Endian
	const float scalefactor = -1.0F;

	char buffer[PFM_MAXLINE];	// temporary buffer whose size should be enough for what we need

	// Find the appropriate magic number for this file type

	char magic = 0;

	switch (image_type) {
		case FIT_RGBF:
			magic = 'F';	// RGBF
			break;	
		case FIT_FLOAT:
			magic = 'f';	// float greyscale
			break;
		default:
			return FALSE;
	}

	// Write the header info

	snprintf(buffer, std::size(buffer), "P%c\n%d %d\n%f\n", magic, width, height, scalefactor);
	io->write_proc(&buffer, (unsigned int)strlen(buffer), 1, handle);

	// Write the image data
	for (unsigned y = 0; y < height; y++) {	
		uint8_t *bits = FreeImage_GetScanLine(dib, height - 1 - y);
		io->write_proc(bits, 1, lineWidth, handle);
	}

	return TRUE;
}

// ==========================================================
//   Init
// ==========================================================

void DLL_CALLCONV
InitPFM(Plugin *plugin, int format_id) {
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
