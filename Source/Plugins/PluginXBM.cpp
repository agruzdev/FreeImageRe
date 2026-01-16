// ==========================================================
// XBM Loader
//
// Design and implementation by
// - Herve Drolon <drolon@infonie.fr>
// part of the code adapted from the netPBM package (xbmtopbm.c)
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
//
//
// ==========================================================

#include "FreeImage.h"
#include "Utilities.h"

#include <cstring>

// ==========================================================
// Internal functions
// ==========================================================

#define MAX_LINE 512
#define LINE_FORMAT_WIDTH_OR_HEIGHT  "#define %" FI_QUOTE(MAX_LINE) "s %d"
#define LINE_FORMAT_BITS_V10 "static short %" FI_QUOTE(MAX_LINE) "s = {"
#define LINE_FORMAT_BITS_V11_OPT1 "static char %" FI_QUOTE(MAX_LINE) "s = {"
#define LINE_FORMAT_BITS_V11_OPT2 "static unsigned char %" FI_QUOTE(MAX_LINE) "s = {"

static const char *ERR_XBM_SYNTAX	= "Syntax error";
static const char *ERR_XBM_LINE		= "Line too long";
static const char *ERR_XBM_DECL		= "Unable to find a line in the file containing the start of C array declaration (\"static char\" or whatever)";
static const char *ERR_XBM_EOFREAD	= "EOF / read error";
static const char *ERR_XBM_WIDTH	= "Invalid width";
static const char *ERR_XBM_HEIGHT	= "Invalid height";
static const char *ERR_XBM_MEMORY	= "Out of memory";

/**
Get a string from a stream. 
Read the string from the current stream to the first newline character. 
The result stored in str is appended with a null character.
@param str Storage location for data 
@param n Maximum number of characters to read 
@param io Pointer to the FreeImageIO structure
@param handle Handle to the stream
@return Returns str. NULL is returned to indicate an error or an end-of-file condition.
*/
static char* 
readLine(char *str, int n, FreeImageIO *io, fi_handle handle) {
	char c;
	int count, i = 0;
	do {
		count = io->read_proc(&c, 1, 1, handle);
		str[i++] = c;
	} while ((c != '\n') && (i < n - 1));
	if (count <= 0)
		return nullptr;
	str[i] = '\0';
	return str;
}

/**
Get a char from the stream
@param io Pointer to the FreeImageIO structure
@param handle Handle to the stream
@return Returns the next character in the stream
*/
static int 
readChar(FreeImageIO *io, fi_handle handle) {
	uint8_t c;
	io->read_proc(&c, 1, 1, handle);
	return c;
}

/**
Read an XBM file into a buffer
@param io Pointer to the FreeImageIO structure
@param handle Handle to the stream
@param widthP (return value) Pointer to the bitmap width
@param heightP (return value) Pointer to the bitmap height
@param dataP (return value) Pointer to the bitmap buffer
@return Returns NULL if OK, returns an error message otherwise
*/
static const char* 
readXBMFile(FreeImageIO *io, fi_handle handle, int *widthP, int *heightP, std::unique_ptr<void, decltype(&free)> &dataP) {
	std::string line(MAX_LINE, '\0'), name(MAX_LINE + 1, '\0');
	char *ptr{};
	int version = 0;
	size_t bytes, bytes_per_line, raster_length;
	int c1, c2, value1, value2;
	int hex_table[256];
	bool found_declaration{}; // haven't found it yet; haven't even looked
	/* in scanning through the bitmap file, we have found the first
	 line of the C declaration of the array (the "static char ..." or whatever line)
	 */

	*widthP = *heightP = -1;

	for (;;) {
		if (!readLine(line.data(), MAX_LINE, io, handle)) {
			break;
		}

		if (strlen(line.c_str()) >= MAX_LINE - 1) {
			return ERR_XBM_LINE;
		}

		int val{};
		if (2 == std::sscanf(line.c_str(), LINE_FORMAT_WIDTH_OR_HEIGHT, name.data(), &val)) {
			if (const auto suffix = std::strrchr(name.c_str(), '_')) {
				if (0 == std::strcmp("_width", suffix)) {
					*widthP = val;
					continue;
				}
				if (0 == std::strcmp("_height", suffix)) {
					*heightP = val;
					continue;
				}
			}
		}
		
		if (1 == std::sscanf(line.c_str(), LINE_FORMAT_BITS_V10, name.data())) {
			version = 10;
			found_declaration = true;
			break;
		}

		if (1 == std::sscanf(line.c_str(), LINE_FORMAT_BITS_V11_OPT1, name.data()) || 
				 1 == std::sscanf(line.c_str(), LINE_FORMAT_BITS_V11_OPT2, name.data())) {
			version = 11;
			found_declaration = true;
			break;
		}
	}

	if (!found_declaration) {
		return ERR_XBM_DECL;
	}

	if (*widthP <= 0) {
		return ERR_XBM_WIDTH;
	}
	if (*heightP <= 0) {
		return ERR_XBM_HEIGHT;
	}

	int padding = 0;
	if (((*widthP % 16) >= 1) && ((*widthP % 16) <= 8) && (version == 10)) {
		padding = 1;
	}

	bytes_per_line = (*widthP + 7) / 8 + padding;

	raster_length =  bytes_per_line * *heightP;
	dataP.reset(malloc(raster_length));
	if (!dataP) {
		return ERR_XBM_MEMORY;
	}

	// initialize hex_table
	for ( c1 = 0; c1 < 256; c1++ ) {
		hex_table[c1] = 256;
	}
	hex_table['0'] = 0;
	hex_table['1'] = 1;
	hex_table['2'] = 2;
	hex_table['3'] = 3;
	hex_table['4'] = 4;
	hex_table['5'] = 5;
	hex_table['6'] = 6;
	hex_table['7'] = 7;
	hex_table['8'] = 8;
	hex_table['9'] = 9;
	hex_table['A'] = 10;
	hex_table['B'] = 11;
	hex_table['C'] = 12;
	hex_table['D'] = 13;
	hex_table['E'] = 14;
	hex_table['F'] = 15;
	hex_table['a'] = 10;
	hex_table['b'] = 11;
	hex_table['c'] = 12;
	hex_table['d'] = 13;
	hex_table['e'] = 14;
	hex_table['f'] = 15;

	if (version == 10) {
		for (bytes = 0, ptr = static_cast<char *>(dataP.get()); bytes < raster_length; bytes += 2) {
			while (( c1 = readChar(io, handle) ) != 'x') {
				if (c1 == EOF)
					return( ERR_XBM_EOFREAD );
			}

			c1 = readChar(io, handle);
			c2 = readChar(io, handle);
			if (c1 == EOF || c2 == EOF)
				return( ERR_XBM_EOFREAD );
			value1 = ( hex_table[c1] << 4 ) + hex_table[c2];
			if (value1 >= 256)
				return( ERR_XBM_SYNTAX );
			c1 = readChar(io, handle);
			c2 = readChar(io, handle);
			if (c1 == EOF || c2 == EOF)
				return( ERR_XBM_EOFREAD );
			value2 = ( hex_table[c1] << 4 ) + hex_table[c2];
			if (value2 >= 256)
				return( ERR_XBM_SYNTAX );
			*ptr++ = (char)value2;
			if ((!padding) || (( bytes + 2 ) % bytes_per_line))
				*ptr++ = (char)value1;
		}
	}
	else {
		for (bytes = 0, ptr = static_cast<char *>(dataP.get()); bytes < raster_length; bytes++) {
			/*
			** skip until digit is found
			*/
			for (; ;) {
				c1 = readChar(io, handle);
				if (c1 == EOF)
					return( ERR_XBM_EOFREAD );
				value1 = hex_table[c1];
				if (value1 != 256)
					break;
			}
			/*
			** loop on digits
			*/
			for (; ;) {
				c2 = readChar(io, handle);
				if (c2 == EOF)
					return( ERR_XBM_EOFREAD );
				value2 = hex_table[c2];
				if (value2 != 256) {
					value1 = (value1 << 4) | value2;
					if (value1 >= 256)
						return( ERR_XBM_SYNTAX );
				}
				else if (c2 == 'x' || c2 == 'X') {
					if (value1 == 0)
						continue;
					else return( ERR_XBM_SYNTAX );
				}
				else break;
			}
			*ptr++ = (char)value1;
		}
	}

	return nullptr;
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
	return "XBM";
}

static const char * DLL_CALLCONV
Description() {
	return "X11 Bitmap Format";
}

static const char * DLL_CALLCONV
Extension() {
	return "xbm";
}

static const char * DLL_CALLCONV
RegExpr() {
	return nullptr;
}

static const char * DLL_CALLCONV
MimeType() {
	return "image/x-xbitmap";
}

static FIBOOL DLL_CALLCONV
Validate(FreeImageIO *io, fi_handle handle) {
	char magic[8];
	if (readLine(magic, 8, io, handle)) {
		if (strcmp(magic, "#define") == 0)
			return TRUE;
	}
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

// ----------------------------------------------------------

static FIBITMAP * DLL_CALLCONV
Load(FreeImageIO *io, fi_handle handle, int page, int flags, void *data) {
	int width, height;

	try {
		std::unique_ptr<void, decltype(&free)> buffer(nullptr, &free);
		// load the bitmap data
		const char* error = readXBMFile(io, handle, &width, &height, buffer);
		// Microsoft doesn't implement throw between functions :(
		if (error) throw (char*)error;


		// allocate a new dib
		std::unique_ptr<FIBITMAP, decltype(&FreeImage_Unload)> dib(FreeImage_Allocate(width, height, 1), &FreeImage_Unload);
		if (!dib) throw (char*)ERR_XBM_MEMORY;

		// write the palette data
		FIRGBA8 *pal = FreeImage_GetPalette(dib.get());
		pal[0].red = pal[0].green = pal[0].blue = 0;
		pal[1].red = pal[1].green = pal[1].blue = 255;

		// copy the bitmap
		auto *bP = static_cast<uint8_t *>(buffer.get());
		for (int y = 0; y < height; y++) {
			uint8_t count = 0;
			uint8_t mask = 1;
			uint8_t *bits = FreeImage_GetScanLine(dib.get(), height - 1 - y);

			for (int x = 0; x < width; x++) {
				if (count >= 8) {
					bP++;
					count = 0;
					mask = 1;
				}
				if (*bP & mask) {
					// Set bit(x, y) to 0
					bits[x >> 3] &= (0xFF7F >> (x & 0x7));
				} else {
					// Set bit(x, y) to 1
					bits[x >> 3] |= (0x80 >> (x & 0x7));
				}
				count++;
				mask <<= 1;
			}
			bP++;
		}

		return dib.release();

	} catch(const char *text) {
		FreeImage_OutputMessageProc(s_format_id, text);
	}
	return nullptr;
}


// ==========================================================
//   Init
// ==========================================================

void DLL_CALLCONV
InitXBM(Plugin *plugin, int format_id) {
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

