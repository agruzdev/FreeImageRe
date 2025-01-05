// ==========================================================
// G3 Fax Loader
//
// Design and implementation by
// - Herve Drolon (drolon@infonie.fr)
// - Petr Pytelka (pyta@lightcomp.com)
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

#include "tiffiop.h"

#include "FreeImage.h"
#include "Utilities.h"

// ==========================================================
// Plugin Interface
// ==========================================================

static int s_format_id;

// ==========================================================
//   Constant/Macro declarations
// ==========================================================

#define G3_DEFAULT_WIDTH	1728

#define TIFFhowmany8(x) (((x)&0x07)?((uint32_t)(x)>>3)+1:(uint32_t)(x)>>3)

// ==========================================================
//   libtiff interface 
// ==========================================================

static tmsize_t 
_g3ReadProc(thandle_t handle, void *buf, tmsize_t size) {
	// returns an error when reading the TIFF header
	return 0;
}

static tmsize_t
_g3WriteProc(thandle_t handle, void *buf, tmsize_t size) {
	// returns ok when writing the TIFF header
	return size;
}

static toff_t
_g3SeekProc(thandle_t handle, toff_t off, int whence) {
	return 0;
}

static int
_g3CloseProc(thandle_t handle) {
	return 0;
}

static toff_t
_g3SizeProc(thandle_t handle) {
	return 0;
}

static int
_g3MapProc(thandle_t, void** base, toff_t* size) {
	return 0;
}

static void
_g3UnmapProc(thandle_t, void* base, toff_t size) {
}

// --------------------------------------------------------------

static tmsize_t
G3GetFileSize(FreeImageIO *io, fi_handle handle) {
    long currentPos = io->tell_proc(handle);
    io->seek_proc(handle, 0, SEEK_END);
    long fileSize = io->tell_proc(handle);
    io->seek_proc(handle, currentPos, SEEK_SET);
    return fileSize;
}

static FIBOOL 
G3ReadFile(FreeImageIO *io, fi_handle handle, uint8_t *tif_rawdata, tmsize_t tif_rawdatasize) {
	return ((tmsize_t)(io->read_proc(tif_rawdata, (unsigned)tif_rawdatasize, 1, handle) * tif_rawdatasize) == tif_rawdatasize);
}

// ==========================================================
// Internal functions
// ==========================================================

static int 
copyFaxFile(FreeImageIO *io, fi_handle handle, TIFF* tifin, uint32_t xsize, int stretch, FIMEMORY *memory) {
	uint32_t row;
	uint16_t badrun;
	uint16_t	badfaxrun;
	uint32_t	badfaxlines;
	int ok;

	try {

		const uint32_t linesize = TIFFhowmany8(xsize);
		std::unique_ptr<void, decltype(&_TIFFfree)> safeRowbuf(_TIFFmalloc(linesize), &_TIFFfree);
		std::unique_ptr<void, decltype(&_TIFFfree)> safeRefbuf(_TIFFmalloc(linesize), &_TIFFfree);
		if (!safeRowbuf || !safeRefbuf) {
			throw FI_MSG_ERROR_MEMORY;
		}
		auto *rowbuf = static_cast<uint8_t*>(safeRowbuf.get());
		auto *refbuf = static_cast<uint8_t*>(safeRefbuf.get());

		tifin->tif_rawdatasize = G3GetFileSize(io, handle);
		tifin->tif_rawdata = (tidata_t) _TIFFmalloc(tifin->tif_rawdatasize);
		if (!tifin->tif_rawdata) {
			throw FI_MSG_ERROR_MEMORY;
		}
		std::unique_ptr<void, decltype(&_TIFFfree)> safeRawdata(tifin->tif_rawdata, &_TIFFfree);

		if (!G3ReadFile(io, handle, tifin->tif_rawdata, tifin->tif_rawdatasize)) {
			throw "Read error at scanline 0";
		}
		tifin->tif_rawcp = tifin->tif_rawdata;
		tifin->tif_rawcc = tifin->tif_rawdatasize;

		(*tifin->tif_setupdecode)(tifin);
		(*tifin->tif_predecode)(tifin, (uint16_t) 0);
		tifin->tif_row = 0;
		badfaxlines = 0;
		badfaxrun = 0;

		_TIFFmemset(refbuf, 0, linesize);
		row = 0;
		badrun = 0;		// current run of bad lines 
		while (tifin->tif_rawcc > 0) {
			ok = (*tifin->tif_decoderow)(tifin, rowbuf, linesize, 0);
			if (!ok) {
				badfaxlines++;
				badrun++;
				// regenerate line from previous good line 
				_TIFFmemcpy(rowbuf, refbuf, linesize);
			} else {
				if (badrun > badfaxrun)
					badfaxrun = badrun;
				badrun = 0;
				_TIFFmemcpy(refbuf, rowbuf, linesize);
			}
			tifin->tif_row++;

			FreeImage_WriteMemory(rowbuf, linesize, 1, memory);
			row++;
			if (stretch) {
				FreeImage_WriteMemory(rowbuf, linesize, 1, memory);
				row++;
			}
		}
		if (badrun > badfaxrun) {
			badfaxrun = badrun;
		}

		tifin->tif_rawdata = nullptr;

		/*
		if (verbose) {
			fprintf(stderr, "%d rows in input\n", rows);
			fprintf(stderr, "%ld total bad rows\n", (long) badfaxlines);
			fprintf(stderr, "%d max consecutive bad rows\n", badfaxrun);
		}
		*/

	} catch(const char *message) {
		tifin->tif_rawdata = nullptr;
		FreeImage_OutputMessageProc(s_format_id, message);

		return -1;
	}

	return (row);
}


// ==========================================================
// Plugin Implementation
// ==========================================================

static const char * DLL_CALLCONV
Format() {
	return "G3";
}

static const char * DLL_CALLCONV 
Description() {
	return "Raw fax format CCITT G.3";
}

static const char * DLL_CALLCONV 
Extension() {
	return "g3";
}

static const char * DLL_CALLCONV 
RegExpr() {
	return nullptr; // there is now reasonable regexp for raw G3
}

static const char * DLL_CALLCONV 
MimeType() {
	return "image/fax-g3";
}

static FIBOOL DLL_CALLCONV 
SupportsExportDepth(int depth) {
	return	FALSE;
}

// ----------------------------------------------------------

static FIBITMAP * DLL_CALLCONV
Load(FreeImageIO *io, fi_handle handle, int page, int flags, void *data) {
	//int verbose = 0;
	int	stretch = 0;
	int rows;
	float resX = 204.0;
	float resY = 196.0;

	uint32_t xsize = G3_DEFAULT_WIDTH;
	int compression_in = COMPRESSION_CCITTFAX3;
	int fillorder_in = FILLORDER_LSB2MSB;
	uint32_t group3options_in = 0;	// 1d-encoded 
	uint32_t group4options_in = 0;	// compressed 
	int photometric_in = PHOTOMETRIC_MINISWHITE;

	if (!handle) return nullptr;

	try {
		// set default load options

		compression_in = COMPRESSION_CCITTFAX3;			// input is g3-encoded 
		group3options_in &= ~GROUP3OPT_2DENCODING;		// input is 1d-encoded (g3 only) 
		fillorder_in = FILLORDER_MSB2LSB;				// input has msb-to-lsb fillorder 

		/*
		Original input-related fax2tiff options

		while ((c = getopt(argc, argv, "R:X:o:1234ABLMPUW5678abcflmprsuvwz?")) != -1) {
			switch (c) {
					// input-related options 
				case '3':		// input is g3-encoded 
					compression_in = COMPRESSION_CCITTFAX3;
					break;
				case '4':		// input is g4-encoded 
					compression_in = COMPRESSION_CCITTFAX4;
					break;
				case 'U':		// input is uncompressed (g3 and g4) 
					group3options_in |= GROUP3OPT_UNCOMPRESSED;
					group4options_in |= GROUP4OPT_UNCOMPRESSED;
					break;
				case '1':		// input is 1d-encoded (g3 only) 
					group3options_in &= ~GROUP3OPT_2DENCODING;
					break;
				case '2':		// input is 2d-encoded (g3 only) 
					group3options_in |= GROUP3OPT_2DENCODING;
					break;
				case 'P':	// input has not-aligned EOL (g3 only) 
					group3options_in &= ~GROUP3OPT_FILLBITS;
					break;
				case 'A':		// input has aligned EOL (g3 only) 
					group3options_in |= GROUP3OPT_FILLBITS;
					break;
				case 'W':		// input has 0 mean white 
					photometric_in = PHOTOMETRIC_MINISWHITE;
					break;
				case 'B':		// input has 0 mean black 
					photometric_in = PHOTOMETRIC_MINISBLACK;
					break;
				case 'L':		// input has lsb-to-msb fillorder 
					fillorder_in = FILLORDER_LSB2MSB;
					break;
				case 'M':		// input has msb-to-lsb fillorder 
					fillorder_in = FILLORDER_MSB2LSB;
					break;
				case 'R':		// input resolution 
					resY = (float) atof(optarg);
					break;
				case 'X':		// input width 
					xsize = (uint32_t) atoi(optarg);
					break;

					// output-related options 
				case 's':		// stretch image by dup'ng scanlines 
					stretch = 1;
					break;
				case 'v':		// -v for info 
					verbose++;
					break;
			}
		}
		*/

		// open a temporary memory buffer to save decoded scanlines
		FIMEMORY *memory = FreeImage_OpenMemory();
		if (!memory) throw FI_MSG_ERROR_MEMORY;
		std::unique_ptr<FIMEMORY, decltype(&FreeImage_CloseMemory)> safeMemory(memory, &FreeImage_CloseMemory);

		// wrap the raw fax file
		TIFF *faxTIFF = TIFFClientOpen("(FakeInput)", "w",
			// TIFFClientOpen() fails if we don't set existing value here 
			nullptr,
			_g3ReadProc, _g3WriteProc,
			_g3SeekProc, _g3CloseProc,
			_g3SizeProc, _g3MapProc,
			_g3UnmapProc);

		if (!faxTIFF) {
			throw "Can not create fake input file";
		}
		std::unique_ptr<TIFF, decltype(&TIFFClose)> safeFaxTiff(faxTIFF, &TIFFClose);

		TIFFSetMode(faxTIFF, O_RDONLY);
		TIFFSetField(faxTIFF, TIFFTAG_IMAGEWIDTH, xsize);
		TIFFSetField(faxTIFF, TIFFTAG_SAMPLESPERPIXEL, 1);
		TIFFSetField(faxTIFF, TIFFTAG_BITSPERSAMPLE, 1);
		TIFFSetField(faxTIFF, TIFFTAG_FILLORDER, fillorder_in);
		TIFFSetField(faxTIFF, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(faxTIFF, TIFFTAG_PHOTOMETRIC, photometric_in);
		TIFFSetField(faxTIFF, TIFFTAG_YRESOLUTION, resY);
		TIFFSetField(faxTIFF, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);

		// NB: this must be done after directory info is setup 
		TIFFSetField(faxTIFF, TIFFTAG_COMPRESSION, compression_in);
		if (compression_in == COMPRESSION_CCITTFAX3)
			TIFFSetField(faxTIFF, TIFFTAG_GROUP3OPTIONS, group3options_in);
		else if (compression_in == COMPRESSION_CCITTFAX4)
			TIFFSetField(faxTIFF, TIFFTAG_GROUP4OPTIONS, group4options_in);
		
		resX = 204;
		if (!stretch) {
			TIFFGetField(faxTIFF, TIFFTAG_YRESOLUTION, &resY);
		} else {
			resY = 196;
		}

		// decode the raw fax data
		rows = copyFaxFile(io, handle, faxTIFF, xsize, stretch, memory);
		if (rows <= 0) throw "Error when decoding raw fax file : check the decoder options";

		// allocate the output dib
		std::unique_ptr<FIBITMAP, decltype(&FreeImage_Unload)> dib(FreeImage_Allocate(xsize, rows, 1), &FreeImage_Unload);
		const uint32_t linesize = TIFFhowmany8(xsize);

		// fill the bitmap structure ...
		// ... palette
		FIRGBA8 *pal = FreeImage_GetPalette(dib.get());
		if (photometric_in == PHOTOMETRIC_MINISWHITE) {
			pal[0].red = pal[0].green = pal[0].blue = 255;
			pal[1].red = pal[1].green = pal[1].blue = 0;
		} else {
			pal[0].red = pal[0].green = pal[0].blue = 0;
			pal[1].red = pal[1].green = pal[1].blue = 255;
		}
		// ... resolution
		FreeImage_SetDotsPerMeterX(dib.get(), (unsigned)(resX/0.0254000 + 0.5));
		FreeImage_SetDotsPerMeterY(dib.get(), (unsigned)(resY/0.0254000 + 0.5));

		// read the decoded scanline and fill the bitmap data
		FreeImage_SeekMemory(memory, 0, SEEK_SET);
		for (int k = 0; k < rows; k++) {
			uint8_t *bits = FreeImage_GetScanLine(dib.get(), rows - 1 - k);
			FreeImage_ReadMemory(bits, linesize, 1, memory);
		}

		return dib.release();

	} catch(const char *message) {
		FreeImage_OutputMessageProc(s_format_id, message);
	}

	return nullptr;

}

// ==========================================================
//   Init
// ==========================================================

void DLL_CALLCONV
InitG3(Plugin *plugin, int format_id) {
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
	plugin->validate_proc = nullptr;
	plugin->mime_proc = MimeType;
	plugin->supports_export_bpp_proc = SupportsExportDepth;
	plugin->supports_export_type_proc = nullptr;
	plugin->supports_icc_profiles_proc = nullptr;
}
