// ==========================================================
// HDR Loader and writer
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
// Plugin Interface
// ==========================================================

static int s_format_id;

// ==========================================================
// RGBE library
// ==========================================================

// ----------------------------------------------------------

// maximum size of a line in the header
#define HDR_MAXLINE	256

// flags indicating which fields in an rgbeHeaderInfo are valid
#define RGBE_VALID_PROGRAMTYPE	0x01
#define RGBE_VALID_COMMENT		0x02
#define RGBE_VALID_GAMMA		0x04
#define RGBE_VALID_EXPOSURE		0x08

// offsets to red, green, and blue components in a data (float) pixel
#define RGBE_DATA_RED    0
#define RGBE_DATA_GREEN  1
#define RGBE_DATA_BLUE   2

// ----------------------------------------------------------
#ifdef _WIN32
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif

typedef struct tagHeaderInfo {
	int valid;					// indicate which fields are valid
	char programtype[16];		// listed at beginning of file to identify it after "#?". defaults to "RGBE"
	char comment[HDR_MAXLINE];	// comment beginning with "# " 
	float gamma;				// image has already been gamma corrected with given gamma. defaults to 1.0 (no correction)
	float exposure;				// a value of 1.0 in an image corresponds to <exposure> watts/steradian/m^2. defaults to 1.0
} rgbeHeaderInfo;

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

typedef enum {
	rgbe_read_error,
	rgbe_write_error,
	rgbe_format_error,
	rgbe_memory_error
} rgbe_error_code;

// ----------------------------------------------------------
// Prototypes
// ----------------------------------------------------------

static FIBOOL rgbe_Error(rgbe_error_code error_code, const char *msg);
static FIBOOL rgbe_GetLine(FreeImageIO *io, fi_handle handle, char *buffer, int length);
static inline void rgbe_FloatToRGBE(uint8_t rgbe[4], FIRGBF *rgbf);
static inline void rgbe_RGBEToFloat(FIRGBF *rgbf, uint8_t rgbe[4]);
static FIBOOL rgbe_ReadHeader(FreeImageIO *io, fi_handle handle, unsigned *width, unsigned *height, rgbeHeaderInfo *header_info);
static FIBOOL rgbe_WriteHeader(FreeImageIO *io, fi_handle handle, unsigned width, unsigned height, rgbeHeaderInfo *info);
static FIBOOL rgbe_ReadPixels(FreeImageIO *io, fi_handle handle, FIRGBF *data, unsigned numpixels);
static FIBOOL rgbe_WritePixels(FreeImageIO *io, fi_handle handle, FIRGBF *data, unsigned numpixels);
static FIBOOL rgbe_ReadPixels_RLE(FreeImageIO *io, fi_handle handle, FIRGBF *data, int scanline_width, unsigned num_scanlines);
static FIBOOL rgbe_WriteBytes_RLE(FreeImageIO *io, fi_handle handle, uint8_t *data, int numbytes);
static FIBOOL rgbe_WritePixels_RLE(FreeImageIO *io, fi_handle handle, FIRGBF *data, unsigned scanline_width, unsigned num_scanlines);
static FIBOOL rgbe_ReadMetadata(FIBITMAP *dib, rgbeHeaderInfo *header_info);
static FIBOOL rgbe_WriteMetadata(FIBITMAP *dib, rgbeHeaderInfo *header_info);

// ----------------------------------------------------------

/**
Default error routine.  change this to change error handling 
*/
static FIBOOL  
rgbe_Error(rgbe_error_code error_code, const char *msg) {
	switch (error_code) {
		case rgbe_read_error:
			FreeImage_OutputMessageProc(s_format_id, "RGBE read error");
			break;
		case rgbe_write_error:
			FreeImage_OutputMessageProc(s_format_id, "RGBE write error");
			break;
		case rgbe_format_error:
			FreeImage_OutputMessageProc(s_format_id, "RGBE bad file format: %s\n", msg);
			break;
		case rgbe_memory_error:
		default:
			FreeImage_OutputMessageProc(s_format_id, "RGBE error: %s\n",msg);
	}

	return FALSE;
}

/**
Get a line from a ASCII io stream
*/
static FIBOOL 
rgbe_GetLine(FreeImageIO *io, fi_handle handle, char *buffer, int length) {
	int i;
	memset(buffer, 0, length);
	for (i = 0; i < length; i++) {
		if (!io->read_proc(&buffer[i], 1, 1, handle)) {
			return FALSE;
		}
		if (buffer[i] == 0x0A) {
			break;
		}
	}
	
	return (i < length) ? TRUE : FALSE;
}

/**
Standard conversion from float pixels to rgbe pixels. 
Note: you can remove the "inline"s if your compiler complains about it 
*/
static inline void 
rgbe_FloatToRGBE(uint8_t rgbe[4], FIRGBF *rgbf) {
	float v = rgbf->red;
	if (rgbf->green > v) {
		v = rgbf->green;
	}
	if (rgbf->blue > v) {
		v = rgbf->blue;
	}
	if (v < 1e-32) {
		rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
	}
	else {
		int e;
		v = (float)(frexp(v, &e) * 256.0 / v);
		rgbe[0] = (uint8_t) (rgbf->red * v);
		rgbe[1] = (uint8_t) (rgbf->green * v);
		rgbe[2] = (uint8_t) (rgbf->blue * v);
		rgbe[3] = (uint8_t) (e + 128);
	}
}

/**
Standard conversion from rgbe to float pixels. 
Note: Ward uses ldexp(col+0.5,exp-(128+8)). 
However we wanted pixels in the range [0,1] to map back into the range [0,1].
*/
static inline void 
rgbe_RGBEToFloat(FIRGBF *rgbf, uint8_t rgbe[4]) {
	if (rgbe[3]) {   // nonzero pixel
		const float f = (float)(ldexp(1.0, rgbe[3] - (int)(128+8)));
		rgbf->red   = rgbe[0] * f;
		rgbf->green = rgbe[1] * f;
		rgbf->blue  = rgbe[2] * f;
	}
	else {
		rgbf->red = rgbf->green = rgbf->blue = 0;
	}
}

/**
Minimal header reading. Modify if you want to parse more information 
*/
static FIBOOL 
rgbe_ReadHeader(FreeImageIO *io, fi_handle handle, unsigned *width, unsigned *height, rgbeHeaderInfo *header_info) {
	char buf[HDR_MAXLINE];
	float tempf;
	int i;
	FIBOOL bFormatFound = FALSE;
	FIBOOL bHeaderFound = FALSE;

	header_info->valid = 0;
	header_info->programtype[0] = 0;
	header_info->gamma = 1.0;
	header_info->exposure = 1.0;

	// get the first line
	if (!rgbe_GetLine(io, handle, buf, HDR_MAXLINE)) {
		return rgbe_Error(rgbe_read_error, nullptr);
	}

	// check the signature

	if ((buf[0] != '#')||(buf[1] != '?')) {
		// if you don't want to require the magic token then comment the next line
		return rgbe_Error(rgbe_format_error,"bad initial token");
	}
	else {
		header_info->valid |= RGBE_VALID_PROGRAMTYPE;
		for (i = 0; i < sizeof(header_info->programtype) - 1; i++) {
			if ((buf[i + 2] == 0) || isspace(buf[i + 2])) {
				break;
			}
			header_info->programtype[i] = buf[i+2];
		}
		header_info->programtype[i] = 0;
	}

	for (;;) {
		// get next line
		if (!rgbe_GetLine(io, handle, buf, HDR_MAXLINE)) {
			return rgbe_Error(rgbe_read_error, nullptr);
		}

		if ((buf[0] == 0) || (buf[0] == '\n')) {
			// end of header so break out of loop
			bHeaderFound = TRUE;
			break;
		}
		else if (strcmp(buf,"FORMAT=32-bit_rle_rgbe\n") == 0) {
			bFormatFound = TRUE;
		}
		else if (sscanf(buf, "GAMMA=%g", &tempf) == 1) {
			header_info->gamma = tempf;
			header_info->valid |= RGBE_VALID_GAMMA;
		}
		else if (sscanf(buf,"EXPOSURE=%g",&tempf) == 1) {
			header_info->exposure = tempf;
			header_info->valid |= RGBE_VALID_EXPOSURE;
		}
		else if ((buf[0] == '#') && (buf[1] == 0x20)) {
			header_info->valid |= RGBE_VALID_COMMENT;
			strcpy(header_info->comment, buf);
		}
	}
	if (!bHeaderFound || !bFormatFound) {
		return rgbe_Error(rgbe_format_error, "invalid header");
	}

	// get next line
	if (!rgbe_GetLine(io, handle, buf, HDR_MAXLINE)) {
		return rgbe_Error(rgbe_read_error, nullptr);
	}

	// get the image width & height
	if (sscanf(buf,"-Y %d +X %d", height, width) < 2) {
		if (sscanf(buf,"+X %d +Y %d", height, width) < 2) {
			return rgbe_Error(rgbe_format_error, "missing image size specifier");
		}
	}

	return TRUE;
}

/**
 default minimal header. modify if you want more information in header 
*/
static FIBOOL 
rgbe_WriteHeader(FreeImageIO *io, fi_handle handle, unsigned width, unsigned height, rgbeHeaderInfo *info) {
	char buffer[HDR_MAXLINE];

	const char *programtype = "RADIANCE";

	if (info && (info->valid & RGBE_VALID_PROGRAMTYPE)) {
		programtype = info->programtype;
	}
	// The #? is to identify file type, the programtype is optional
	snprintf(buffer, std::size(buffer), "#?%s\n", programtype);
	if (io->write_proc(buffer, 1, (unsigned int)strlen(buffer), handle) < 1) {
		return rgbe_Error(rgbe_write_error, nullptr);
	}
	snprintf(buffer, std::size(buffer), "%s\n", info->comment);
	if (io->write_proc(buffer, 1, (unsigned int)strlen(buffer), handle) < 1) {
		return rgbe_Error(rgbe_write_error, nullptr);
	}
	snprintf(buffer, std::size(buffer), "FORMAT=32-bit_rle_rgbe\n");
	if (io->write_proc(buffer, 1, (unsigned int)strlen(buffer), handle) < 1) {
		return rgbe_Error(rgbe_write_error, nullptr);
	}
	if (info && (info->valid & RGBE_VALID_GAMMA)) {
		snprintf(buffer, std::size(buffer), "GAMMA=%g\n", info->gamma);
		if (io->write_proc(buffer, 1, (unsigned int)strlen(buffer), handle) < 1) {
			return rgbe_Error(rgbe_write_error, nullptr);
		}
	}
	if (info && (info->valid & RGBE_VALID_EXPOSURE)) {
		snprintf(buffer,std::size(buffer), "EXPOSURE=%g\n", info->exposure);
		if (io->write_proc(buffer, 1, (unsigned int)strlen(buffer), handle) < 1) {
			return rgbe_Error(rgbe_write_error, nullptr);
		}
	}
	snprintf(buffer, std::size(buffer), "\n-Y %d +X %d\n", height, width);
	if (io->write_proc(buffer, 1, (unsigned int)strlen(buffer), handle) < 1) {
		return rgbe_Error(rgbe_write_error, nullptr);
	}

	return TRUE;
}

static FIBOOL 
rgbe_ReadMetadata(FIBITMAP *dib, rgbeHeaderInfo *header_info) {
	return TRUE;
}
static FIBOOL 
rgbe_WriteMetadata(FIBITMAP *dib, rgbeHeaderInfo *header_info) {
	header_info->gamma = 1;
	header_info->valid |= RGBE_VALID_GAMMA;
	header_info->exposure = 1;	// default value
	header_info->valid |= RGBE_VALID_EXPOSURE;

	return TRUE;
}

/** 
Simple read routine. Will not correctly handle run length encoding 
*/
static FIBOOL 
rgbe_ReadPixels(FreeImageIO *io, fi_handle handle, FIRGBF *data, unsigned numpixels) {
  uint8_t rgbe[4];

  for (unsigned x = 0; x < numpixels; x++) {
	if (io->read_proc(rgbe, 1, sizeof(rgbe), handle) < 1) {
		return rgbe_Error(rgbe_read_error, nullptr);
	}
	rgbe_RGBEToFloat(&data[x], rgbe);
  }

  return TRUE;
}

/**
 Simple write routine that does not use run length encoding. 
 These routines can be made faster by allocating a larger buffer and
 fread-ing and fwrite-ing the data in larger chunks.
*/
static FIBOOL 
rgbe_WritePixels(FreeImageIO *io, fi_handle handle, FIRGBF *data, unsigned numpixels) {
  uint8_t rgbe[4];

  for (unsigned x = 0; x < numpixels; x++) {
	  rgbe_FloatToRGBE(rgbe, &data[x]);
	  if (io->write_proc(rgbe, sizeof(rgbe), 1, handle) < 1) {
		  return rgbe_Error(rgbe_write_error, nullptr);
	  }
  }

  return TRUE;
}

static FIBOOL 
rgbe_ReadPixels_RLE(FreeImageIO *io, fi_handle handle, FIRGBF *data, int scanline_width, unsigned num_scanlines) {
	uint8_t rgbe[4], *scanline_buffer{}, *ptr, *ptr_end;
	int i, count;
	uint8_t buf[2];
	
	if ((scanline_width < 8)||(scanline_width > 0x7fff)) {
		// run length encoding is not allowed so read flat
		return rgbe_ReadPixels(io, handle, data, scanline_width * num_scanlines);
	}
	std::unique_ptr<void, decltype(&free)> safeScanlineBuffer(nullptr, &free);
	// read in each successive scanline 
	while (num_scanlines > 0) {
		if (io->read_proc(rgbe, 1, sizeof(rgbe), handle) < 1) {
			return rgbe_Error(rgbe_read_error, nullptr);
		}
		if ((rgbe[0] != 2) || (rgbe[1] != 2) || (rgbe[2] & 0x80)) {
			// this file is not run length encoded
			rgbe_RGBEToFloat(data, rgbe);
			data ++;
			return rgbe_ReadPixels(io, handle, data, scanline_width * num_scanlines - 1);
		}
		if ((((int)rgbe[2]) << 8 | rgbe[3]) != scanline_width) {
			return rgbe_Error(rgbe_format_error,"wrong scanline width");
		}
		if (!scanline_buffer) {
			safeScanlineBuffer.reset(malloc(sizeof(uint8_t) * 4 * scanline_width));
			if (!safeScanlineBuffer) {
				return rgbe_Error(rgbe_memory_error, "unable to allocate buffer space");
			}
			scanline_buffer = static_cast<uint8_t*>(safeScanlineBuffer.get());
		}
		
		ptr = &scanline_buffer[0];
		// read each of the four channels for the scanline into the buffer
		for (i = 0; i < 4; i++) {
			ptr_end = &scanline_buffer[(i+1)*scanline_width];
			while (ptr < ptr_end) {
				if (io->read_proc(buf, 1, 2 * sizeof(uint8_t), handle) < 1) {
					return rgbe_Error(rgbe_read_error, nullptr);
				}
				if (buf[0] > 128) {
					// a run of the same value
					count = buf[0] - 128;
					if ((count == 0) || (count > ptr_end - ptr)) {
						return rgbe_Error(rgbe_format_error, "bad scanline data");
					}
					while (count-- > 0) {
						*ptr++ = buf[1];
					}
				}
				else {
					// a non-run
					count = buf[0];
					if ((count == 0) || (count > ptr_end - ptr)) {
						return rgbe_Error(rgbe_format_error, "bad scanline data");
					}
					*ptr++ = buf[1];
					if (--count > 0) {
						if (io->read_proc(ptr, 1, sizeof(uint8_t) * count, handle) < 1) {
							return rgbe_Error(rgbe_read_error, nullptr);
						}
						ptr += count;
					}
				}
			}
		}
		// now convert data from buffer into floats
		for (i = 0; i < scanline_width; i++) {
			rgbe[0] = scanline_buffer[i];
			rgbe[1] = scanline_buffer[i+scanline_width];
			rgbe[2] = scanline_buffer[i+2*scanline_width];
			rgbe[3] = scanline_buffer[i+3*scanline_width];
			rgbe_RGBEToFloat(data, rgbe);
			data ++;
		}

		num_scanlines--;
	}

	return TRUE;
}

/**
 The code below is only needed for the run-length encoded files.
 Run length encoding adds considerable complexity but does 
 save some space.  For each scanline, each channel (r,g,b,e) is 
 encoded separately for better compression. 
 @return Returns TRUE if successful, returns FALSE otherwise
*/
static FIBOOL 
rgbe_WriteBytes_RLE(FreeImageIO *io, fi_handle handle, uint8_t *data, int numbytes) {
	static const int MINRUNLENGTH = 4;
	int cur, beg_run, run_count, old_run_count, nonrun_count;
	uint8_t buf[2];
	
	cur = 0;
	while (cur < numbytes) {
		beg_run = cur;
		// find next run of length at least 4 if one exists 
		run_count = old_run_count = 0;
		while ((run_count < MINRUNLENGTH) && (beg_run < numbytes)) {
			beg_run += run_count;
			old_run_count = run_count;
			run_count = 1;
			while ((beg_run + run_count < numbytes) && (run_count < 127) && (data[beg_run] == data[beg_run + run_count])) {
				run_count++;
			}
		}
		// if data before next big run is a short run then write it as such 
		if ((old_run_count > 1)&&(old_run_count == beg_run - cur)) {
			buf[0] = (uint8_t)(128 + old_run_count);   // write short run
			buf[1] = data[cur];
			if (io->write_proc(buf, 2 * sizeof(uint8_t), 1, handle) < 1) {
				return rgbe_Error(rgbe_write_error, nullptr);
			}
			cur = beg_run;
		}
		// write out bytes until we reach the start of the next run 
		while (cur < beg_run) {
			nonrun_count = beg_run - cur;
			if (nonrun_count > 128) {
				nonrun_count = 128;
			}
			buf[0] = (uint8_t)nonrun_count;
			if (io->write_proc(buf, sizeof(buf[0]), 1, handle) < 1) {
				return rgbe_Error(rgbe_write_error, nullptr);
			}
			if (io->write_proc(&data[cur], sizeof(data[0]) * nonrun_count, 1, handle) < 1) {
				return rgbe_Error(rgbe_write_error, nullptr);
			}
			cur += nonrun_count;
		}
		// write out next run if one was found 
		if (run_count >= MINRUNLENGTH) {
			buf[0] = (uint8_t)(128 + run_count);
			buf[1] = data[beg_run];
			if (io->write_proc(buf, sizeof(buf[0]) * 2, 1, handle) < 1) {
				return rgbe_Error(rgbe_write_error, nullptr);
			}
			cur += run_count;
		}
	}
	
	return TRUE;
}

static FIBOOL 
rgbe_WritePixels_RLE(FreeImageIO *io, fi_handle handle, FIRGBF *data, unsigned scanline_width, unsigned num_scanlines) {
	uint8_t rgbe[4];

	if ((scanline_width < 8)||(scanline_width > 0x7fff)) {
		// run length encoding is not allowed so write flat
		return rgbe_WritePixels(io, handle, data, scanline_width * num_scanlines);
	}
	std::unique_ptr<void, decltype(&free)> safeBuffer(malloc(sizeof(uint8_t) * 4 * scanline_width), &free);;
	if (!safeBuffer) {
		// no buffer space so write flat 
		return rgbe_WritePixels(io, handle, data, scanline_width * num_scanlines);
	}
	auto *buffer = static_cast<uint8_t*>(safeBuffer.get());

	while (num_scanlines-- > 0) {
		rgbe[0] = (uint8_t)2;
		rgbe[1] = (uint8_t)2;
		rgbe[2] = (uint8_t)(scanline_width >> 8);
		rgbe[3] = (uint8_t)(scanline_width & 0xFF);
		if (io->write_proc(rgbe, sizeof(rgbe), 1, handle) < 1) {
			return rgbe_Error(rgbe_write_error, nullptr);
		}
		for (unsigned x = 0; x < scanline_width; x++) {
			rgbe_FloatToRGBE(rgbe, data);
			buffer[x] = rgbe[0];
			buffer[x+scanline_width] = rgbe[1];
			buffer[x+2*scanline_width] = rgbe[2];
			buffer[x+3*scanline_width] = rgbe[3];
			data ++;
		}
		// write out each of the four channels separately run length encoded
		// first red, then green, then blue, then exponent
		for (int i = 0; i < 4; i++) {
			FIBOOL bOK = rgbe_WriteBytes_RLE(io, handle, &buffer[i*scanline_width], scanline_width);
			if (!bOK) {
				return bOK;
			}
		}
	}

	return TRUE;
}


// ----------------------------------------------------------



// ==========================================================
// Plugin Implementation
// ==========================================================

static const char * DLL_CALLCONV
Format() {
	return "HDR";
}

static const char * DLL_CALLCONV
Description() {
	return "High Dynamic Range Image";
}

static const char * DLL_CALLCONV
Extension() {
	return "hdr";
}

static const char * DLL_CALLCONV
RegExpr() {
	return nullptr;
}

static const char * DLL_CALLCONV
MimeType() {
	return "image/vnd.radiance";
}

static FIBOOL DLL_CALLCONV
Validate(FreeImageIO *io, fi_handle handle) {
	const uint8_t hdr_signature[] = { '#', '?' };
	uint8_t signature[] = { 0, 0 };

	io->read_proc(signature, 1, 2, handle);

	return (memcmp(hdr_signature, signature, 2) == 0);
}

static FIBOOL DLL_CALLCONV
SupportsExportDepth(int depth) {
	return FALSE;
}

static FIBOOL DLL_CALLCONV 
SupportsExportType(FREE_IMAGE_TYPE type) {
	return (type == FIT_RGBF) ? TRUE : FALSE;
}

static FIBOOL DLL_CALLCONV
SupportsNoPixels() {
	return TRUE;
}

// --------------------------------------------------------------------------

static FIBITMAP * DLL_CALLCONV
Load(FreeImageIO *io, fi_handle handle, int page, int flags, void *data) {
	if (!handle) {
		return nullptr;
	}

	FIBOOL header_only = (flags & FIF_LOAD_NOPIXELS) == FIF_LOAD_NOPIXELS;

	try {

		rgbeHeaderInfo header_info;
		unsigned width, height;

		// Read the header
		if (!rgbe_ReadHeader(io, handle, &width, &height, &header_info)) {
			return nullptr;
		}

		// allocate a RGBF image
		std::unique_ptr<FIBITMAP, decltype(&FreeImage_Unload)> dib(FreeImage_AllocateHeaderT(header_only, FIT_RGBF, width, height), &FreeImage_Unload);
		if (!dib) {
			throw FI_MSG_ERROR_MEMORY;
		}

		// set the metadata as comments
		rgbe_ReadMetadata(dib.get(), &header_info);

		if (header_only) {
			// header only mode
			return dib.release();
		}

		// read the image pixels and fill the dib
		
		for (unsigned y = 0; y < height; y++) {
			FIRGBF *scanline = (FIRGBF*)FreeImage_GetScanLine(dib.get(), height - 1 - y);
			if (!rgbe_ReadPixels_RLE(io, handle, scanline, width, 1)) {
				return nullptr;
			}
		}

		return dib.release();
	}
	catch(const char *text) {
		FreeImage_OutputMessageProc(s_format_id, text);
	}

	return nullptr;
}

static FIBOOL DLL_CALLCONV
Save(FreeImageIO *io, FIBITMAP *dib, fi_handle handle, int page, int flags, void *data) {
	if (!dib) return FALSE;

	FREE_IMAGE_TYPE src_type = FreeImage_GetImageType(dib);
	if (src_type != FIT_RGBF) {
		FreeImage_OutputMessageProc(s_format_id, "FREE_IMAGE_TYPE: Unable to convert from type %d to type %d.\n No such conversion exists.", src_type, FIT_RGBF);
		return FALSE;
	}

	unsigned width  = FreeImage_GetWidth(dib);
	unsigned height = FreeImage_GetHeight(dib);

	// write the header

	rgbeHeaderInfo header_info;
	memset(&header_info, 0, sizeof(rgbeHeaderInfo));
	// fill the header with correct gamma and exposure
	rgbe_WriteMetadata(dib, &header_info);
	// fill a comment
	snprintf(header_info.comment, std::size(header_info.comment), "# Made with FreeImage %s", FreeImage_GetVersion());
	if (!rgbe_WriteHeader(io, handle, width, height, &header_info)) {
		return FALSE;
	}

	// write each scanline

	for (unsigned y = 0; y < height; y++) {
		auto *scanline = (FIRGBF*)FreeImage_GetScanLine(dib, height - 1 - y);
		if (!rgbe_WritePixels_RLE(io, handle, scanline, width, 1)) {
			return FALSE;
		}
	}

	return TRUE;
}

// ==========================================================
//   Init
// ==========================================================

void DLL_CALLCONV
InitHDR(Plugin *plugin, int format_id) {
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
