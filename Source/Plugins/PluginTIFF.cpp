// ==========================================================
// TIFF Loader and Writer
//
// Design and implementation by 
// - Floris van den Berg (flvdberg@wxs.nl)
// - Herve Drolon (drolon@infonie.fr)
// - Markus Loibl (markus.loibl@epost.de)
// - Luca Piergentili (l.pierge@terra.es)
// - Detlev Vendt (detlev.vendt@brillit.de)
// - Mihail Naydenov (mnaydenov@users.sourceforge.net)
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

#ifdef unix
#undef unix
#endif
#ifdef __unix
#undef __unix
#endif

#define TIFF_DISABLE_DEPRECATED
#ifdef _WIN32
#define NOMINMAX
#endif

#include "FreeImage.h"
#include "Utilities.h"
#include "tiffiop.h"
#include "../Metadata/FreeImageTag.h"
#include "half.h"

#include "FreeImageIO.h"
#include "PSDParser.h"

// --------------------------------------------------------------------------
// GeoTIFF profile (see XTIFF.cpp)
// --------------------------------------------------------------------------
void XTIFFInitialize();
FIBOOL tiff_read_geotiff_profile(TIFF *tif, FIBITMAP *dib);
FIBOOL tiff_write_geotiff_profile(TIFF *tif, FIBITMAP *dib);

// --------------------------------------------------------------------------
// TIFF Exif profile (see XTIFF.cpp)
// ----------------------------------------------------------
FIBOOL tiff_read_exif_tags(TIFF *tif, TagLib::MDMODEL md_model, FIBITMAP *dib);
FIBOOL tiff_write_exif_tags(TIFF *tif, TagLib::MDMODEL md_model, FIBITMAP *dib);

// --------------------------------------------------------------------------
//   LogLuv conversion functions interface (see TIFFLogLuv.cpp)
// --------------------------------------------------------------------------
void tiff_ConvertLineXYZToRGB(uint8_t *target, uint8_t *source, double stonits, int width_in_pixels);
void tiff_ConvertLineRGBToXYZ(uint8_t *target, uint8_t *source, int width_in_pixels);

// ----------------------------------------------------------

/** Supported loading methods */
typedef enum {
	LoadAsRBGA			= 0, 
	LoadAsCMYK			= 1, 
	LoadAs8BitTrns		= 2, 
	LoadAsGenericStrip	= 3, 
	LoadAsTiled			= 4,
	LoadAsLogLuv		= 5,
	LoadAsHalfFloat		= 6
} TIFFLoadMethod;

// ----------------------------------------------------------
//   local prototypes
// ----------------------------------------------------------

static tmsize_t _tiffReadProc(thandle_t handle, void* buf, tmsize_t size);
static tmsize_t _tiffWriteProc(thandle_t handle, void* buf, tmsize_t size);
static toff_t _tiffSeekProc(thandle_t handle, toff_t off, int whence);
static int _tiffCloseProc(thandle_t fd);
static int _tiffMapProc(thandle_t fd, void** pbase, toff_t* psize);
static void _tiffUnmapProc(thandle_t fd, void* base, toff_t size);

static uint16_t CheckColormap(int n, uint16_t* r, uint16_t* g, uint16_t* b);
static uint16_t GetPhotometric(FIBITMAP *dib);

static void ReadResolution(TIFF *tiff, FIBITMAP *dib);
static void WriteResolution(TIFF *tiff, FIBITMAP *dib);

static void ReadPalette(TIFF *tiff, uint16_t photometric, uint16_t bitspersample, FIBITMAP *dib);

static FIBITMAP* CreateImageType(FIBOOL header_only, FREE_IMAGE_TYPE fit, int width, int height, uint16_t bitspersample, uint16_t samplesperpixel);
static FREE_IMAGE_TYPE ReadImageType(TIFF *tiff, uint16_t bitspersample, uint16_t samplesperpixel);
static void WriteImageType(TIFF *tiff, FREE_IMAGE_TYPE fit);

static void WriteCompression(TIFF *tiff, uint16_t bitspersample, uint16_t samplesperpixel, uint16_t photometric, int flags);

static FIBOOL tiff_read_iptc_profile(TIFF *tiff, FIBITMAP *dib);
static FIBOOL tiff_read_xmp_profile(TIFF *tiff, FIBITMAP *dib);
static FIBOOL tiff_read_exif_profile(FreeImageIO *io, fi_handle handle, TIFF *tiff, FIBITMAP *dib);
static void ReadMetadata(FreeImageIO *io, fi_handle handle, TIFF *tiff, FIBITMAP *dib);

static FIBOOL tiff_write_iptc_profile(TIFF *tiff, FIBITMAP *dib);
static FIBOOL tiff_write_xmp_profile(TIFF *tiff, FIBITMAP *dib);
static void WriteMetadata(TIFF *tiff, FIBITMAP *dib);

static TIFFLoadMethod FindLoadMethod(TIFF *tif, uint16_t photometric, uint16_t bitspersample, uint16_t samplesperpixel, FREE_IMAGE_TYPE image_type, int flags);

static void ReadThumbnail(FreeImageIO *io, fi_handle handle, void *data, TIFF *tiff, FIBITMAP *dib);


// ==========================================================
// Plugin Interface
// ==========================================================

static int s_format_id;

typedef struct {
    FreeImageIO *io;
	fi_handle handle;
	TIFF *tif;
} fi_TIFFIO;

// ----------------------------------------------------------
//   libtiff interface 
// ----------------------------------------------------------

static tmsize_t 
_tiffReadProc(thandle_t handle, void *buf, tmsize_t size) {
	fi_TIFFIO *fio = (fi_TIFFIO*)handle;
	return fio->io->read_proc(buf, (unsigned)size, 1, fio->handle) * size;
}

static tmsize_t
_tiffWriteProc(thandle_t handle, void *buf, tmsize_t size) {
	fi_TIFFIO *fio = (fi_TIFFIO*)handle;
	return fio->io->write_proc(buf, (unsigned)size, 1, fio->handle) * size;
}

static toff_t
_tiffSeekProc(thandle_t handle, toff_t off, int whence) {
	fi_TIFFIO *fio = (fi_TIFFIO*)handle;
	fio->io->seek_proc(fio->handle, (long)off, whence);
	return fio->io->tell_proc(fio->handle);
}

static int
_tiffCloseProc(thandle_t fd) {
	return 0;
}

#include <sys/stat.h>

static toff_t
_tiffSizeProc(thandle_t handle) {
    fi_TIFFIO *fio = (fi_TIFFIO*)handle;
    long currPos = fio->io->tell_proc(fio->handle);
    fio->io->seek_proc(fio->handle, 0, SEEK_END);
    long fileSize = fio->io->tell_proc(fio->handle);
    fio->io->seek_proc(fio->handle, currPos, SEEK_SET);
    return fileSize;
}

static int
_tiffMapProc(thandle_t, void** base, toff_t* size) {
	return 0;
}

static void
_tiffUnmapProc(thandle_t, void* base, toff_t size) {
}

/**
Open a TIFF file descriptor for reading or writing
@param handle File handle
@param name Name of the file handle
@param mode Specifies if the file is to be opened for reading ("r") or writing ("w")
*/
TIFF *
TIFFFdOpen(thandle_t handle, const char *name, const char *mode) {
	// Open the file; the callback will set everything up
	TIFF *tif = TIFFClientOpen(name, mode, handle,
	    _tiffReadProc, _tiffWriteProc, _tiffSeekProc, _tiffCloseProc,
	    _tiffSizeProc, _tiffMapProc, _tiffUnmapProc);

	return tif;
}

// ----------------------------------------------------------
//   in FreeImage warnings and errors are disabled
// ----------------------------------------------------------

static void
msdosWarningHandler(const char* module, const char* fmt, va_list ap) {
}

static void
msdosErrorHandler(const char* module, const char* fmt, va_list ap) {
	
	// use this for diagnostic only (do not use otherwise, even in DEBUG mode)
	/*
	if (module) {
		char msg[1024];
		vsnprintf(msg, std::size(msg), fmt, ap);
		FreeImage_OutputMessageProc(s_format_id, "%s: %s", module, msg);
	}
	*/
}

// ----------------------------------------------------------

#define CVT(x)      (((x) * 255L) / ((1L<<16)-1))
#define	SCALE(x)	(((x)*((1L<<16)-1))/255)

// ==========================================================
// Internal functions
// ==========================================================

static uint16_t
CheckColormap(int n, uint16_t* r, uint16_t* g, uint16_t* b) {
    while (n-- > 0) {
        if (*r++ >= 256 || *g++ >= 256 || *b++ >= 256) {
			return 16;
		}
	}

    return 8;
}

/**
Get the TIFFTAG_PHOTOMETRIC value from the dib
*/
static uint16_t
GetPhotometric(FIBITMAP *dib) {
	const FREE_IMAGE_COLOR_TYPE color_type = FreeImage_GetColorType(dib);
	switch (color_type) {
		case FIC_MINISWHITE:	// min value is white
			return PHOTOMETRIC_MINISWHITE;
		case FIC_MINISBLACK:	// min value is black
			return PHOTOMETRIC_MINISBLACK;
		case FIC_PALETTE:		// color map indexed
			return PHOTOMETRIC_PALETTE;
		case FIC_RGB:			// RGB color model
		case FIC_RGBALPHA:		// RGB color model with alpha channel
			return PHOTOMETRIC_RGB;
		case FIC_CMYK:			// CMYK color model
			return PHOTOMETRIC_RGB;	// default to RGB unless the save flag is set to TIFF_CMYK
		default:
			return PHOTOMETRIC_MINISBLACK;
	}
}

/**
Get the resolution from the TIFF and fill the dib with universal units
*/
static void 
ReadResolution(TIFF *tiff, FIBITMAP *dib) {
	float fResX = 300.0;
	float fResY = 300.0;
	uint16_t resUnit = RESUNIT_INCH;

	TIFFGetField(tiff, TIFFTAG_RESOLUTIONUNIT, &resUnit);
	TIFFGetField(tiff, TIFFTAG_XRESOLUTION, &fResX);
	TIFFGetField(tiff, TIFFTAG_YRESOLUTION, &fResY);
	
	// If we don't have a valid resolution unit and valid resolution is specified then assume inch
	if (resUnit == RESUNIT_NONE && fResX > 0.0 && fResY > 0.0) {
		resUnit = RESUNIT_INCH;
	}
	if (resUnit == RESUNIT_INCH) {
		FreeImage_SetDotsPerMeterX(dib, (unsigned) (fResX/0.0254000 + 0.5));
		FreeImage_SetDotsPerMeterY(dib, (unsigned) (fResY/0.0254000 + 0.5));
	} else if (resUnit == RESUNIT_CENTIMETER) {
		FreeImage_SetDotsPerMeterX(dib, (unsigned) (fResX*100.0 + 0.5));
		FreeImage_SetDotsPerMeterY(dib, (unsigned) (fResY*100.0 + 0.5));
	}
}

/**
Set the resolution to the TIFF using english units
*/
static void 
WriteResolution(TIFF *tiff, FIBITMAP *dib) {
	double res;

	TIFFSetField(tiff, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);

	res = (unsigned long) (0.5 + 0.0254 * FreeImage_GetDotsPerMeterX(dib));
	TIFFSetField(tiff, TIFFTAG_XRESOLUTION, res);

	res = (unsigned long) (0.5 + 0.0254 * FreeImage_GetDotsPerMeterY(dib));
	TIFFSetField(tiff, TIFFTAG_YRESOLUTION, res);
}

/**
Fill the dib palette according to the TIFF photometric
*/
static void 
ReadPalette(TIFF *tiff, uint16_t photometric, uint16_t bitspersample, FIBITMAP *dib) {
	FIRGBA8 *pal = FreeImage_GetPalette(dib);

	switch (photometric) {
		case PHOTOMETRIC_MINISBLACK:	// bitmap and greyscale image types
		case PHOTOMETRIC_MINISWHITE:
			// Monochrome image

			if (bitspersample == 1) {
				if (photometric == PHOTOMETRIC_MINISWHITE) {
					pal[0].red = pal[0].green = pal[0].blue = 255;
					pal[1].red = pal[1].green = pal[1].blue = 0;
				} else {
					pal[0].red = pal[0].green = pal[0].blue = 0;
					pal[1].red = pal[1].green = pal[1].blue = 255;
				}

			} else if ((bitspersample == 2) || (bitspersample == 4) || (bitspersample == 8)) {
				// need to build the scale for greyscale images
				int ncolors = FreeImage_GetColorsUsed(dib);

				if (photometric == PHOTOMETRIC_MINISBLACK) {
					for (int i = 0; i < ncolors; i++) {
						pal[i].red	=
						pal[i].green =
						pal[i].blue	= (uint8_t)(i*(255/(ncolors-1)));
					}
				} else {
					for (int i = 0; i < ncolors; i++) {
						pal[i].red	=
						pal[i].green =
						pal[i].blue	= (uint8_t)(255-i*(255/(ncolors-1)));
					}
				}
			}

			break;

		case PHOTOMETRIC_PALETTE:	// color map indexed
			uint16_t *red;
			uint16_t *green;
			uint16_t *blue;

			TIFFGetField(tiff, TIFFTAG_COLORMAP, &red, &green, &blue); 

			// load the palette in the DIB

			if (CheckColormap(1<<bitspersample, red, green, blue) == 16) {
				for (int i = (1 << bitspersample) - 1; i >= 0; i--) {
					pal[i].red =(uint8_t) CVT(red[i]);
					pal[i].green = (uint8_t) CVT(green[i]);
					pal[i].blue = (uint8_t) CVT(blue[i]);           
				}
			} else {
				for (int i = (1 << bitspersample) - 1; i >= 0; i--) {
					pal[i].red = (uint8_t) red[i];
					pal[i].green = (uint8_t) green[i];
					pal[i].blue = (uint8_t) blue[i];        
				}
			}

			break;
	}
}

/** 
Allocate a FIBITMAP
@param header_only If TRUE, allocate a 'header only' FIBITMAP, otherwise allocate a full FIBITMAP
@param fit Image type
@param width Image width in pixels
@param height Image height in pixels
@param bitspersample # bits per sample
@param samplesperpixel # samples per pixel
@return Returns the allocated image if successful, returns NULL otherwise
*/
static FIBITMAP* 
CreateImageType(FIBOOL header_only, FREE_IMAGE_TYPE fit, int width, int height, uint16_t bitspersample, uint16_t samplesperpixel) {
	FIBITMAP *dib{};

	if ((width < 0) || (height < 0)) {
		// check for malicious images
		return nullptr;
	}

	int bpp = bitspersample * samplesperpixel;

	if (fit == FIT_BITMAP) {
		// standard bitmap type 

		if (bpp == 16) {

			if ((samplesperpixel == 2) && (bitspersample == 8)) {
				// 8-bit indexed + 8-bit alpha channel -> convert to 8-bit transparent
				dib = FreeImage_AllocateHeader(header_only, width, height, 8);
			} else {
				// 16-bit RGB -> expect it to be 565
				dib = FreeImage_AllocateHeader(header_only, width, height, bpp, FI16_565_RED_MASK, FI16_565_GREEN_MASK, FI16_565_BLUE_MASK);
			}

		}
		else {

			dib = FreeImage_AllocateHeader(header_only, width, height, std::min(bpp, 32), FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);
		}


	} else {
		// other bitmap types

		dib = FreeImage_AllocateHeaderT(header_only, fit, width, height, bpp);
	}

	return dib;
}

/** 
Read the TIFFTAG_SAMPLEFORMAT tag and convert to FREE_IMAGE_TYPE
@param tiff LibTIFF TIFF Handle
@param bitspersample # bit per sample
@param samplesperpixel # samples per pixel
@return Returns the image type as a FREE_IMAGE_TYPE value
*/
static FREE_IMAGE_TYPE 
ReadImageType(TIFF *tiff, uint16_t bitspersample, uint16_t samplesperpixel) {
	uint16_t sampleformat = 0;
	FREE_IMAGE_TYPE fit = FIT_BITMAP ; 

	uint16_t bpp = bitspersample * samplesperpixel;

	// try the sampleformat tag
    if (TIFFGetField(tiff, TIFFTAG_SAMPLEFORMAT, &sampleformat)) {

        switch (sampleformat) {
			case SAMPLEFORMAT_UINT:
				switch (bpp) {
					case 1:
					case 4:
					case 8:
					case 24:
						fit = FIT_BITMAP;
						break;
					case 16:
						// 8-bit + alpha or 16-bit greyscale
						if (samplesperpixel == 2) {
							fit = FIT_BITMAP;
						} else {
							fit = FIT_UINT16;
						}
						break;
					case 32:
						if (samplesperpixel == 4) {
							fit = FIT_BITMAP;
						} else {
							fit = FIT_UINT32;
						}
						break;
					case 48:
						if (samplesperpixel == 3) {
							fit = FIT_RGB16;
						}
						break;
					case 64:
						if (samplesperpixel == 4) {
							fit = FIT_RGBA16;
						}
						break;
					case 96:
						fit = FIT_RGB32;
						break;
					default:
						if (bpp >= 128) {
							fit = FIT_RGBA32;
						}
						break;
				}
				break;

			case SAMPLEFORMAT_INT:
				switch (bpp) {
					case 16:
						if (samplesperpixel == 3) {
							fit = FIT_BITMAP;
						} else {
							fit = FIT_INT16;
						}
						break;
					case 32:
						fit = FIT_INT32;
						break;
				}
				break;

			case SAMPLEFORMAT_IEEEFP:
				switch (bpp) {
					case 32:
						fit = FIT_FLOAT;
						break;
					case 48:
						// 3 x half float => convert to RGBF
						if ((samplesperpixel == 3) && (bitspersample == 16)) {
							fit = FIT_RGBF;
						}
						break;
					case 64:
						if (samplesperpixel == 2) {
							fit = FIT_FLOAT;
						} else {
							fit = FIT_DOUBLE;
						}
						break;
					case 96:
						fit = FIT_RGBF;
						break;
					default:
						if (bpp >= 128) {
							fit = FIT_RGBAF;
						}
					break;
				}
				break;
			case SAMPLEFORMAT_COMPLEXIEEEFP:
				switch (bpp) {
					case 64:
						break;
					case 128:
						fit = FIT_COMPLEX;
						break;
				}
				break;

			}
    }
	// no sampleformat tag : assume SAMPLEFORMAT_UINT
	else {
		if (samplesperpixel == 1) {
			switch (bpp) {
				case 16:
					fit = FIT_UINT16;
					break;

				case 32:
					fit = FIT_UINT32;
					break;
			}
		}
		else if (samplesperpixel == 3) {
			if (bitspersample > 8 && bitspersample <= 16) {
				fit = FIT_RGB16;
			}
		}
		else if (samplesperpixel >= 4) { 
			if (bitspersample > 8 && bitspersample <= 16) {
				fit = FIT_RGBA16;
			}
		}

	}

    return fit;
}

/** 
Convert FREE_IMAGE_TYPE and write TIFFTAG_SAMPLEFORMAT
@param tiff LibTIFF TIFF Handle
@param fit Image type as a FREE_IMAGE_TYPE value
*/
static void 
WriteImageType(TIFF *tiff, FREE_IMAGE_TYPE fit) {
	switch (fit) {
		case FIT_BITMAP:	// standard image: 1-, 4-, 8-, 16-, 24-, 32-bit
		case FIT_UINT16:	// array of unsigned short	: unsigned 16-bit
		case FIT_UINT32:	// array of unsigned long	: unsigned 32-bit
		case FIT_RGB16:		// 48-bit RGB image			: 3 x 16-bit
		case FIT_RGBA16:	// 64-bit RGBA image		: 4 x 16-bit
			TIFFSetField(tiff, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
			break;

		case FIT_INT16:		// array of short	: signed 16-bit
		case FIT_INT32:		// array of long	: signed 32-bit
			TIFFSetField(tiff, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT);
			break;

		case FIT_FLOAT:		// array of float	: 32-bit
		case FIT_DOUBLE:	// array of double	: 64-bit
		case FIT_RGBF:		// 96-bit RGB float image	: 3 x 32-bit IEEE floating point
		case FIT_RGBAF:		// 128-bit RGBA float image	: 4 x 32-bit IEEE floating point
			TIFFSetField(tiff, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
			break;

		case FIT_COMPLEX:	// array of COMPLEX : 2 x 64-bit
			TIFFSetField(tiff, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_COMPLEXIEEEFP);
			break;
	}
}

/**
Select the compression algorithm
@param tiff LibTIFF TIFF Handle
@param 
*/
static void 
WriteCompression(TIFF *tiff, uint16_t bitspersample, uint16_t samplesperpixel, uint16_t photometric, int flags) {
	uint16_t compression;
	uint16_t bitsperpixel = bitspersample * samplesperpixel;

	if (photometric == PHOTOMETRIC_LOGLUV) {
		compression = COMPRESSION_SGILOG;
	} else if ((flags & TIFF_PACKBITS) == TIFF_PACKBITS) {
		compression = COMPRESSION_PACKBITS;
	} else if ((flags & TIFF_DEFLATE) == TIFF_DEFLATE) {
		compression = COMPRESSION_DEFLATE;
	} else if ((flags & TIFF_ADOBE_DEFLATE) == TIFF_ADOBE_DEFLATE) {
		compression = COMPRESSION_ADOBE_DEFLATE;
	} else if ((flags & TIFF_NONE) == TIFF_NONE) {
		compression = COMPRESSION_NONE;
	} else if ((bitsperpixel == 1) && ((flags & TIFF_CCITTFAX3) == TIFF_CCITTFAX3)) {
		compression = COMPRESSION_CCITTFAX3;
	} else if ((bitsperpixel == 1) && ((flags & TIFF_CCITTFAX4) == TIFF_CCITTFAX4)) {
		compression = COMPRESSION_CCITTFAX4;
	} else if ((flags & TIFF_LZW) == TIFF_LZW) {
		compression = COMPRESSION_LZW;
	} else if ((flags & TIFF_JPEG) == TIFF_JPEG) {
		if (((bitsperpixel == 8) && (photometric != PHOTOMETRIC_PALETTE)) || (bitsperpixel == 24)) {
			compression = COMPRESSION_JPEG;
			// RowsPerStrip must be multiple of 8 for JPEG
			uint32_t rowsperstrip = (uint32_t) -1;
			rowsperstrip = TIFFDefaultStripSize(tiff, rowsperstrip);
            rowsperstrip = rowsperstrip + (8 - (rowsperstrip % 8));
			// overwrite previous RowsPerStrip
			TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, rowsperstrip);
		} else {
			// default to LZW
			compression = COMPRESSION_LZW;
		}
	}
	else {
		// default compression scheme

		switch (bitsperpixel) {
			case 1:
				compression = COMPRESSION_CCITTFAX4;
				break;

			case 4:
			case 8:
			case 16:
			case 24:
			case 32:
				compression = COMPRESSION_LZW;
				break;
			case 48:
			case 64:
			case 96:
			case 128:
				compression = COMPRESSION_LZW;
				break;

			default :
				compression = COMPRESSION_NONE;
				break;
		}
	}

	TIFFSetField(tiff, TIFFTAG_COMPRESSION, compression);

	if (compression == COMPRESSION_LZW) {
		// This option is only meaningful with LZW compression: a predictor value of 2 
		// causes each scanline of the output image to undergo horizontal differencing 
		// before it is encoded; a value of 1 forces each scanline to be encoded without differencing.

		// Found on LibTIFF mailing list : 
		// LZW without differencing works well for 1-bit images, 4-bit grayscale images, 
		// and many palette-color images. But natural 24-bit color images and some 8-bit 
		// grayscale images do much better with differencing.

		if ((bitspersample == 8) || (bitspersample == 16)) {
			if ((bitsperpixel >= 8) && (photometric != PHOTOMETRIC_PALETTE)) {
				TIFFSetField(tiff, TIFFTAG_PREDICTOR, 2);
			} else {
				TIFFSetField(tiff, TIFFTAG_PREDICTOR, 1);
			}
		} else {
			TIFFSetField(tiff, TIFFTAG_PREDICTOR, 1);
		}
	}
	else if ((compression == COMPRESSION_CCITTFAX3) || (compression == COMPRESSION_CCITTFAX4)) {
		uint32_t imageLength = 0;
		TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &imageLength);
		// overwrite previous RowsPerStrip
		TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, imageLength);

		if (compression == COMPRESSION_CCITTFAX3) {
			// try to be compliant with the TIFF Class F specification
			// that documents the TIFF tags specific to FAX applications
			// see http://palimpsest.stanford.edu/bytopic/imaging/std/tiff-f.html
			uint32_t group3options = GROUP3OPT_2DENCODING | GROUP3OPT_FILLBITS;	
			TIFFSetField(tiff, TIFFTAG_GROUP3OPTIONS, group3options);	// 2d-encoded, has aligned EOL
			TIFFSetField(tiff, TIFFTAG_FILLORDER, FILLORDER_LSB2MSB);	// lsb-to-msb fillorder
		}
	}
}

// ==========================================================
// TIFF metadata routines
// ==========================================================

/**
	Read the TIFFTAG_RICHTIFFIPTC tag (IPTC/NAA or Adobe Photoshop profile)
*/
static FIBOOL 
tiff_read_iptc_profile(TIFF *tiff, FIBITMAP *dib) {
	uint8_t *profile{};
	uint32_t profile_size = 0;

    if (TIFFGetField(tiff,TIFFTAG_RICHTIFFIPTC, &profile_size, &profile) == 1) {
		if (TIFFIsByteSwapped(tiff) != 0) {
			TIFFSwabArrayOfLong((uint32_t *) profile, (unsigned long)profile_size);
		}

		return read_iptc_profile(dib, profile, 4 * profile_size);
	}

	return FALSE;
}

/**
	Read the TIFFTAG_XMLPACKET tag (XMP profile)
	@param dib Input FIBITMAP
	@param tiff LibTIFF TIFF handle
	@return Returns TRUE if successful, FALSE otherwise
*/
static FIBOOL  
tiff_read_xmp_profile(TIFF *tiff, FIBITMAP *dib) {
	uint8_t *profile{};
	uint32_t profile_size = 0;

	bool bSuccess{};
	if (TIFFGetField(tiff, TIFFTAG_XMLPACKET, &profile_size, &profile) == 1) {
		// create a tag
		if (std::unique_ptr<FITAG, decltype(&FreeImage_DeleteTag)> tag(FreeImage_CreateTag(), &FreeImage_DeleteTag); tag) {
			bSuccess = FreeImage_SetTagID(tag.get(), TIFFTAG_XMLPACKET);	// 700
			bSuccess = bSuccess && FreeImage_SetTagKey(tag.get(), g_TagLib_XMPFieldName);
			bSuccess = bSuccess && FreeImage_SetTagLength(tag.get(), profile_size);
			bSuccess = bSuccess && FreeImage_SetTagCount(tag.get(), profile_size);
			bSuccess = bSuccess && FreeImage_SetTagType(tag.get(), FIDT_ASCII);
			bSuccess = bSuccess && FreeImage_SetTagValue(tag.get(), profile);

			// store the tag
			bSuccess = bSuccess && FreeImage_SetMetadata(FIMD_XMP, dib, FreeImage_GetTagKey(tag.get()), tag.get());
		}
	}

	return bSuccess ? TRUE : FALSE;
}

/**
	Read the Exif profile embedded in a TIFF
	@param dib Input FIBITMAP
	@param tiff LibTIFF TIFF handle
	@return Returns TRUE if successful, FALSE otherwise
*/
static FIBOOL 
tiff_read_exif_profile(FreeImageIO *io, fi_handle handle, TIFF *tiff, FIBITMAP *dib) {
	FIBOOL bResult = FALSE;
	toff_t exif_offset = 0;

	// read EXIF-TIFF tags
	bResult = tiff_read_exif_tags(tiff, TagLib::EXIF_MAIN, dib);

	// get the IFD offset
	if (TIFFGetField(tiff, TIFFTAG_EXIFIFD, &exif_offset)) {

		const long tell_pos = io->tell_proc(handle);
		const uint16_t cur_dir = TIFFCurrentDirectory(tiff);

		// read EXIF tags
		if (TIFFReadEXIFDirectory(tiff, exif_offset)) {
			// read all known exif tags
			bResult = tiff_read_exif_tags(tiff, TagLib::EXIF_EXIF, dib);
		}

		io->seek_proc(handle, tell_pos, SEEK_SET);
		TIFFSetDirectory(tiff, cur_dir);
	}

	return bResult;
}

/**
Read TIFF special profiles
*/
static void 
ReadMetadata(FreeImageIO *io, fi_handle handle, TIFF *tiff, FIBITMAP *dib) {

	// IPTC/NAA
	tiff_read_iptc_profile(tiff, dib);

	// Adobe XMP
	tiff_read_xmp_profile(tiff, dib);

	// GeoTIFF
	tiff_read_geotiff_profile(tiff, dib);

	// Exif-TIFF
	tiff_read_exif_profile(io, handle, tiff, dib);
}

// ----------------------------------------------------------

/**
	Write the TIFFTAG_RICHTIFFIPTC tag (IPTC/NAA or Adobe Photoshop profile)
*/
static FIBOOL 
tiff_write_iptc_profile(TIFF *tiff, FIBITMAP *dib) {
	if (FreeImage_GetMetadataCount(FIMD_IPTC, dib)) {
		uint8_t *profile{};
		uint32_t profile_size = 0;
		// create a binary profile
		if (write_iptc_profile(dib, &profile, &profile_size)) {
			uint32_t iptc_size = profile_size;
			iptc_size += (4-(iptc_size & 0x03)); // Round up for long word alignment
			auto *iptc_profile = static_cast<uint8_t*>(calloc(iptc_size, sizeof(uint8_t)));
			if (!iptc_profile) {
				free(profile);
				return FALSE;
			}
			memcpy(iptc_profile, profile, profile_size);
			if (TIFFIsByteSwapped(tiff)) {
				TIFFSwabArrayOfLong((uint32_t *) iptc_profile, (unsigned long)iptc_size/4);
			}
			// Tag is type TIFF_LONG so byte length is divided by four
			TIFFSetField(tiff, TIFFTAG_RICHTIFFIPTC, iptc_size/4, iptc_profile);
			// release the profile data
			free(iptc_profile);
			free(profile);

			return TRUE;
		}
	}

	return FALSE;
}

/**
	Write the TIFFTAG_XMLPACKET tag (XMP profile)
	@param dib Input FIBITMAP
	@param tiff LibTIFF TIFF handle
	@return Returns TRUE if successful, FALSE otherwise
*/
static FIBOOL  
tiff_write_xmp_profile(TIFF *tiff, FIBITMAP *dib) {
	FITAG *tag_xmp{};
	FreeImage_GetMetadata(FIMD_XMP, dib, g_TagLib_XMPFieldName, &tag_xmp);

	if (tag_xmp && FreeImage_GetTagValue(tag_xmp)) {

		TIFFSetField(tiff, TIFFTAG_XMLPACKET, (uint32_t)FreeImage_GetTagLength(tag_xmp), (uint8_t*)FreeImage_GetTagValue(tag_xmp));

		return TRUE;
	}

	return FALSE;
}

/**
	Write the Exif profile to TIFF
	@param dib Input FIBITMAP
	@param tiff LibTIFF TIFF handle
	@return Returns TRUE if successful, FALSE otherwise
*/
static FIBOOL
tiff_write_exif_profile(TIFF *tiff, FIBITMAP *dib) {
	FIBOOL bResult = FALSE;
	
	// write EXIF_MAIN tags, EXIF_EXIF not supported yet
	bResult = tiff_write_exif_tags(tiff, TagLib::EXIF_MAIN, dib);

	return bResult;
}

/**
Write TIFF special profiles
*/
static void 
WriteMetadata(TIFF *tiff, FIBITMAP *dib) {
	// IPTC
	tiff_write_iptc_profile(tiff, dib);

	// Adobe XMP
	tiff_write_xmp_profile(tiff, dib);

	// EXIF_MAIN tags
	tiff_write_exif_profile(tiff, dib);

	// GeoTIFF tags
	tiff_write_geotiff_profile(tiff, dib);
}

// ==========================================================
// Plugin Implementation
// ==========================================================

static const char * DLL_CALLCONV
Format() {
	return "TIFF";
}

static const char * DLL_CALLCONV
Description() {
	return "Tagged Image File Format";
}

static const char * DLL_CALLCONV
Extension() {
	return "tif,tiff";
}

static const char * DLL_CALLCONV
RegExpr() {
	return "^[MI][MI][\\x01*][\\x01*]";
}

static const char * DLL_CALLCONV
MimeType() {
	return "image/tiff";
}

static FIBOOL DLL_CALLCONV
Validate(FreeImageIO *io, fi_handle handle) {	
	const uint8_t tiff_id1[] = { 0x49, 0x49, 0x2A, 0x00 };	// Classic TIFF, little-endian
	const uint8_t tiff_id2[] = { 0x4D, 0x4D, 0x00, 0x2A };	// Classic TIFF, big-endian
	const uint8_t tiff_id3[] = { 0x49, 0x49, 0x2B, 0x00 };	// Big TIFF, little-endian
	const uint8_t tiff_id4[] = { 0x4D, 0x4D, 0x00, 0x2B };	// Big TIFF, big-endian
	uint8_t signature[4] = { 0, 0, 0, 0 };

	io->read_proc(signature, 1, 4, handle);

	if (memcmp(tiff_id1, signature, 4) == 0)
		return TRUE;
	if (memcmp(tiff_id2, signature, 4) == 0)
		return TRUE;
	if (memcmp(tiff_id3, signature, 4) == 0)
		return TRUE;
	if (memcmp(tiff_id4, signature, 4) == 0)
		return TRUE;

	return FALSE;
}

static FIBOOL DLL_CALLCONV
SupportsExportDepth(int depth) {
	return (
			(depth == 1)  ||
			(depth == 4)  ||
			(depth == 8)  ||
			(depth == 24) ||
			(depth == 32)
		);
}

static FIBOOL DLL_CALLCONV 
SupportsExportType(FREE_IMAGE_TYPE type) {
	return (
		(type == FIT_BITMAP)  ||
		(type == FIT_UINT16)  ||
		(type == FIT_INT16)   ||
		(type == FIT_UINT32)  ||
		(type == FIT_INT32)   ||
		(type == FIT_FLOAT)   ||
		(type == FIT_DOUBLE)  ||
		(type == FIT_COMPLEX) || 
		(type == FIT_RGB16)   || 
		(type == FIT_RGBA16)  || 
		(type == FIT_RGBF)    ||
		(type == FIT_RGBAF)
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

// ----------------------------------------------------------

static void * DLL_CALLCONV
Open(FreeImageIO *io, fi_handle handle, FIBOOL read) {
	// wrapper for TIFF I/O
	auto *fio = static_cast<fi_TIFFIO*>(malloc(sizeof(fi_TIFFIO)));
	if (!fio) return nullptr;
	fio->io = io;
	fio->handle = handle;

	if (read) {
		fio->tif = TIFFFdOpen((thandle_t)fio, "", "r");
	} else {
		// mode = "w"	: write Classic TIFF
		// mode = "w8"	: write Big TIFF
		fio->tif = TIFFFdOpen((thandle_t)fio, "", "w");
	}
	if (!fio->tif) {
		free(fio);
		fio = nullptr;
		FreeImage_OutputMessageProc(s_format_id, "Error while opening TIFF: data is invalid");
	}
	return fio;
}

static void DLL_CALLCONV
Close(FreeImageIO *io, fi_handle handle, void *data) {
	if (data) {
		fi_TIFFIO *fio = (fi_TIFFIO*)data;
		TIFFClose(fio->tif);
		free(fio);
	}
}

// ----------------------------------------------------------

static int DLL_CALLCONV
PageCount(FreeImageIO *io, fi_handle handle, void *data) {
	if (data) {
		fi_TIFFIO *fio = (fi_TIFFIO*)data;
		TIFF *tif = (TIFF *)fio->tif;
		int nr_ifd = 0;

		do {
			nr_ifd++;
		} while (TIFFReadDirectory(tif));

		return nr_ifd;
	}

	return 0;
}

// ----------------------------------------------------------

/**
check for uncommon bitspersample values (e.g. 10, 12, ...)
@param photometric TIFFTAG_PHOTOMETRIC tiff tag
@param bitspersample TIFFTAG_BITSPERSAMPLE tiff tag
@param samplesperpixel TIFFTAG_SAMPLESPERPIXEL tiff tag
@return Returns FALSE if a uncommon bit-depth is encountered, returns TRUE otherwise
*/
static FIBOOL 
IsValidBitsPerSample(uint16_t photometric, uint16_t bitspersample, uint16_t samplesperpixel) {

	switch (bitspersample) {
		case 1:
		case 2:
		case 4:
			if ((photometric == PHOTOMETRIC_MINISWHITE) || (photometric == PHOTOMETRIC_MINISBLACK) || (photometric == PHOTOMETRIC_PALETTE)) { 
				return TRUE;
			} else {
				return FALSE;
			}
			break;
		case 8:
			return TRUE;
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			if (photometric == PHOTOMETRIC_RGB) {
				return TRUE;
			}
			else {
				return FALSE;
			}
		case 16:
			if (photometric != PHOTOMETRIC_PALETTE) { 
				return TRUE;
			} else {
				return FALSE;
			}
			break;
		case 32:
			if ((photometric == PHOTOMETRIC_MINISWHITE) || (photometric == PHOTOMETRIC_MINISBLACK) || (photometric == PHOTOMETRIC_LOGLUV)) { 
				return TRUE;
			} else if ((photometric == PHOTOMETRIC_RGB) && (samplesperpixel == 3) || (samplesperpixel == 4)) {
				// RGB[A]F
				return TRUE;
			} else {
				return FALSE;
			}
			break;
		case 64:
		case 128:
			if (photometric == PHOTOMETRIC_MINISBLACK) { 
				return TRUE;
			} else {
				return FALSE;
			}
			break;
		default:
			return FALSE;
	}
}

static TIFFLoadMethod  
FindLoadMethod(TIFF *tif, FREE_IMAGE_TYPE image_type, int flags) {
	uint16_t bitspersample	= (uint16_t)-1;
	uint16_t samplesperpixel	= (uint16_t)-1;
	uint16_t photometric		= (uint16_t)-1;
	uint16_t planar_config	= (uint16_t)-1;

	TIFFLoadMethod loadMethod = LoadAsGenericStrip;

	TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);
	TIFFGetFieldDefaulted(tif, TIFFTAG_PLANARCONFIG, &planar_config);

	FIBOOL bIsTiled = (TIFFIsTiled(tif) == 0) ? FALSE:TRUE;

	switch (photometric) {
		// convert to 24 or 32 bits RGB if the image is full color
		case PHOTOMETRIC_RGB:
			if ((image_type == FIT_RGB16) || (image_type == FIT_RGBA16)) {
				// load 48-bit RGB and 64-bit RGBA without conversion 
				loadMethod = LoadAsGenericStrip;
			} 
			else if (image_type == FIT_RGBF) {
				if ((samplesperpixel == 3) && (bitspersample == 16)) {
					// load 3 x 16-bit half as RGBF
					loadMethod = LoadAsHalfFloat;
				}
			}
			break;
		case PHOTOMETRIC_YCBCR:
		case PHOTOMETRIC_CIELAB:
		case PHOTOMETRIC_ICCLAB:
		case PHOTOMETRIC_ITULAB:
			loadMethod = LoadAsRBGA;
			break;
		case PHOTOMETRIC_LOGLUV:
			loadMethod = LoadAsLogLuv;
			break;
		case PHOTOMETRIC_SEPARATED:
			// if image is PHOTOMETRIC_SEPARATED _and_ comes with an ICC profile, 
			// then the image should preserve its original (CMYK) colour model and 
			// should be read as CMYK (to keep the match of pixel and profile and 
			// to avoid multiple conversions. Conversion can be done by changing 
			// the profile from it's original CMYK to an RGB profile with an 
			// apropriate color management system. Works with non-tiled TIFFs.
			if (!bIsTiled) {
				loadMethod = LoadAsCMYK;
			}
			break;
		case PHOTOMETRIC_MINISWHITE:
		case PHOTOMETRIC_MINISBLACK:
		case PHOTOMETRIC_PALETTE:
			// When samplesperpixel = 2 and bitspersample = 8, set the image as a
			// 8-bit indexed image + 8-bit alpha layer image
			// and convert to a 8-bit image with a transparency table
			if ((samplesperpixel > 1) && (bitspersample == 8)) {
				loadMethod = LoadAs8BitTrns;
			} else {
				loadMethod = LoadAsGenericStrip;
			}
			break;
		default:
			loadMethod = LoadAsGenericStrip;
			break;
	}

	if ((loadMethod == LoadAsGenericStrip) && bIsTiled) {
		loadMethod = LoadAsTiled;
	}

	return loadMethod;
}

// ==========================================================
// TIFF thumbnail routines
// ==========================================================

static FIBITMAP * DLL_CALLCONV
Load(FreeImageIO *io, fi_handle handle, int page, int flags, void *data);

/**
Read embedded thumbnail
*/
static void 
ReadThumbnail(FreeImageIO *io, fi_handle handle, void *data, TIFF *tiff, FIBITMAP *dib) {
	std::unique_ptr<FIBITMAP, decltype(&FreeImage_Unload)> thumbnail(nullptr, &FreeImage_Unload);

	// read exif thumbnail (IFD 1) ...

	/*
	// this code can cause unwanted recursion causing an overflow, it is thus disabled until we have a better solution
	// do we really need to read a thumbnail from the Exif segment ? knowing that TIFF store the thumbnail in the subIFD ...
	// 
	toff_t exif_offset = 0;
	if (TIFFGetField(tiff, TIFFTAG_EXIFIFD, &exif_offset)) {
		
		if (!TIFFLastDirectory(tiff)) {
			// save current position
			const long tell_pos = io->tell_proc(handle);
			const uint16_t cur_dir = TIFFCurrentDirectory(tiff);
			
			// load the thumbnail
			int page = 1;
			int flags = TIFF_DEFAULT;
			thumbnail.reset(Load(io, handle, page, flags, data));
			// store the thumbnail (remember to release it before return)
			FreeImage_SetThumbnail(dib, thumbnail.get());
			
			// restore current position
			io->seek_proc(handle, tell_pos, SEEK_SET);
			TIFFSetDirectory(tiff, cur_dir);
		}
	}
	*/

	// ... or read the first subIFD

	if (!thumbnail) {
		uint16_t subIFD_count = 0;
		toff_t* subIFD_offsets{};

		// This will also read the first (and only) subIFD from a Photoshop-created "pyramid" file.
		// Subsequent, smaller images are 'nextIFD' in that subIFD. Currently we only load the first one. 
		
		if (TIFFGetField(tiff, TIFFTAG_SUBIFD, &subIFD_count, &subIFD_offsets)) {
			if (subIFD_count > 0) {
				// save current position
				const long tell_pos = io->tell_proc(handle);
				const uint16_t cur_dir = TIFFCurrentDirectory(tiff);
				
				if (TIFFSetSubDirectory(tiff, subIFD_offsets[0])) {
					// load the thumbnail
					int page = -1; 
					int flags = TIFF_DEFAULT;
					thumbnail.reset(Load(io, handle, page, flags, data));
					// store the thumbnail (remember to release it before return)
					FreeImage_SetThumbnail(dib, thumbnail.get());
				}

				// restore current position
				io->seek_proc(handle, tell_pos, SEEK_SET);
				TIFFSetDirectory(tiff, cur_dir);
			}
		}
	}
	
	// ... or read Photoshop thumbnail
	
	if (!thumbnail) {
		uint32_t ps_size = 0;
		void *ps_data{};
		
		if (TIFFGetField(tiff, TIFFTAG_PHOTOSHOP, &ps_size, &ps_data)) {
			FIMEMORY *handle = FreeImage_OpenMemory((uint8_t*)ps_data, ps_size);

			FreeImageIO io;
			SetMemoryIO(&io);

			psdParser parser;
			parser.ReadImageResources(&io, handle, ps_size);

			FreeImage_SetThumbnail(dib, parser.GetThumbnail());

			FreeImage_CloseMemory(handle);
		}
	}
}

// --------------------------------------------------------------------------

template <typename DT> static void DecodeStrip(const uint8_t *buf, const tmsize_t src_line, uint8_t *dst_line_begin, const unsigned dst_pitch, const uint32_t strips, const uint16_t sample, const uint16_t chCount, const uint16_t bitspersample) {
	const uint32_t bits_mask = (static_cast<uint32_t>(1) << bitspersample) - 1;
	const uint8_t *src_pixel = buf;

	for (uint32_t l{}; l < strips; ++l) {
		auto *dst_pixel = reinterpret_cast<DT*>(dst_line_begin) + sample;
		uint32_t t{};
		uint16_t stored_bits{};
		for (tmsize_t i{}; i < src_line; ++i) {
			t <<= 8;
			t |= *src_pixel++;
			stored_bits += 8;
			while (stored_bits >= bitspersample) {
				stored_bits -= bitspersample;
				*dst_pixel = static_cast<DT>((t >> stored_bits) & bits_mask);
				dst_pixel += chCount;
			}
		}
		dst_line_begin -= dst_pitch;
	}
}

// --------------------------------------------------------------------------

static FIBITMAP * DLL_CALLCONV
Load(FreeImageIO *io, fi_handle handle, int page, int flags, void *data) {
	if (!handle || !data ) {
		return nullptr;
	}

	TIFF   *tif{};
	uint32_t height = 0; 
	uint32_t width = 0; 
	uint16_t bitspersample = 1;
	uint16_t samplesperpixel = 1;
	uint32_t rowsperstrip = (uint32_t)-1;  
	uint16_t photometric = PHOTOMETRIC_MINISWHITE;
	uint16_t compression = (uint16_t)-1;
	uint16_t planar_config;

	uint32_t iccSize = 0;		// ICC profile length
	void *iccBuf{};	// ICC profile data		

	const FIBOOL header_only = (flags & FIF_LOAD_NOPIXELS) == FIF_LOAD_NOPIXELS;

	try {
		fi_TIFFIO *fio = static_cast<fi_TIFFIO*>(data);
		tif = fio->tif;

		if (page != -1) {
			if (!tif || !TIFFSetDirectory(tif, (uint16_t)page)) {
				throw "Error encountered while opening TIFF file";
			}
		}

		const FIBOOL asCMYK = (flags & TIFF_CMYK) == TIFF_CMYK;

		// first, get the photometric, the compression and basic metadata
		// ---------------------------------------------------------------------------------

		TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);
		TIFFGetField(tif, TIFFTAG_COMPRESSION, &compression);

		// check for HDR formats
		// ---------------------------------------------------------------------------------

		if (photometric == PHOTOMETRIC_LOGLUV) {
			// check the compression
			if (compression != COMPRESSION_SGILOG && compression != COMPRESSION_SGILOG24) {
				throw "Only support SGILOG compressed LogLuv data";
			}
			// set decoder to output in IEEE 32-bit float XYZ values
			TIFFSetField(tif, TIFFTAG_SGILOGDATAFMT, SGILOGDATAFMT_FLOAT);
		}

		// ---------------------------------------------------------------------------------

		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
		TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
		TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);
		TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);
		TIFFGetField(tif, TIFFTAG_ICCPROFILE, &iccSize, &iccBuf);
		TIFFGetFieldDefaulted(tif, TIFFTAG_PLANARCONFIG, &planar_config);

		// check for unsupported formats
		// ---------------------------------------------------------------------------------

		if (!IsValidBitsPerSample(photometric, bitspersample, samplesperpixel)) {
			FreeImage_OutputMessageProc(s_format_id, 
				"Unable to handle this format: bitspersample = %d, samplesperpixel = %d, photometric = %d", 
				(int)bitspersample, (int)samplesperpixel, (int)photometric);
			throw (char*)nullptr;
		}

		// ---------------------------------------------------------------------------------

		// get image data type

		FREE_IMAGE_TYPE image_type = ReadImageType(tif, bitspersample, samplesperpixel);

		// get the most appropriate loading method

		TIFFLoadMethod loadMethod = FindLoadMethod(tif, image_type, flags);

		// ---------------------------------------------------------------------------------

		std::unique_ptr<FIBITMAP, decltype(&FreeImage_Unload)> dib(nullptr, &FreeImage_Unload);
		if (loadMethod == LoadAsRBGA) {
			// ---------------------------------------------------------------------------------
			// RGB[A] loading using the TIFFReadRGBAImage() API
			// ---------------------------------------------------------------------------------

			FIBOOL has_alpha = FALSE;   

			// Read the whole image into one big RGBA buffer and then 
			// convert it to a DIB. This is using the traditional
			// TIFFReadRGBAImage() API that we trust.

			std::unique_ptr<uint32_t[]> safeRaster;

			if (!header_only) {

				safeRaster.reset(new uint32_t[static_cast<size_t>(width) * height]);

				// read the image in one chunk into an RGBA array

				if (!TIFFReadRGBAImage(tif, width, height, safeRaster.get(), 1)) {
					throw FI_MSG_ERROR_UNSUPPORTED_FORMAT;
				}
			}
			// TIFFReadRGBAImage always deliveres 3 or 4 samples per pixel images
			// (RGB or RGBA, see below). Cut-off possibly present channels (additional 
			// alpha channels) from e.g. Photoshop. Any CMYK(A..) is now treated as RGB,
			// any additional alpha channel on RGB(AA..) is lost on conversion to RGB(A)

			if (samplesperpixel > 4) { // TODO Write to Extra Channels
				FreeImage_OutputMessageProc(s_format_id, "Warning: %d additional alpha channel(s) ignored", samplesperpixel-4);
				samplesperpixel = 4;
			}

			// create a new DIB (take care of different samples-per-pixel in case 
			// of converted CMYK image (RGB conversion is on sample per pixel less)

			if (photometric == PHOTOMETRIC_SEPARATED && samplesperpixel == 4) {
				samplesperpixel = 3;
			}

			dib.reset(CreateImageType(header_only, image_type, width, height, bitspersample, samplesperpixel));
			if (!dib) {
				throw FI_MSG_ERROR_DIB_MEMORY;
			}
			
			// fill in the resolution (english or universal)

			ReadResolution(tif, dib.get());

			if (!header_only) {

				// read the raster lines and save them in the DIB
				// with RGB mode, we have to change the order of the 3 samples RGB
				// We use macros for extracting components from the packed ABGR 
				// form returned by TIFFReadRGBAImage.

				const auto *row = safeRaster.get();

				if (samplesperpixel == 4) {
					// 32-bit RGBA
					for (uint32_t y = 0; y < height; y++) {
						uint8_t *bits = FreeImage_GetScanLine(dib.get(), y);
						for (uint32_t x = 0; x < width; x++) {
							bits[FI_RGBA_BLUE]	= (uint8_t)TIFFGetB(row[x]);
							bits[FI_RGBA_GREEN] = (uint8_t)TIFFGetG(row[x]);
							bits[FI_RGBA_RED]	= (uint8_t)TIFFGetR(row[x]);
							bits[FI_RGBA_ALPHA] = (uint8_t)TIFFGetA(row[x]);

							if (bits[FI_RGBA_ALPHA] != 0) {
								has_alpha = TRUE;
							}

							bits += 4;
						}
						row += width;
					}
				} else {
					// 24-bit RGB
					for (uint32_t y = 0; y < height; y++) {
						uint8_t *bits = FreeImage_GetScanLine(dib.get(), y);
						for (uint32_t x = 0; x < width; x++) {
							bits[FI_RGBA_BLUE]	= (uint8_t)TIFFGetB(row[x]);
							bits[FI_RGBA_GREEN] = (uint8_t)TIFFGetG(row[x]);
							bits[FI_RGBA_RED]	= (uint8_t)TIFFGetR(row[x]);

							bits += 3;
						}
						row += width;
					}
				}

				safeRaster.reset();
			}

			// ### Not correct when header only
			FreeImage_SetTransparent(dib.get(), has_alpha);

		} else if (loadMethod == LoadAs8BitTrns) {
			// ---------------------------------------------------------------------------------
			// 8-bit + 8-bit alpha layer loading
			// ---------------------------------------------------------------------------------

			// create a new 8-bit DIB
			dib.reset(CreateImageType(header_only, image_type, width, height, bitspersample, MIN<uint16_t>(2, samplesperpixel)));
			if (!dib) {
				throw FI_MSG_ERROR_DIB_MEMORY;
			}

			// fill in the resolution (english or universal)

			ReadResolution(tif, dib.get());

			// set up the colormap based on photometric

			ReadPalette(tif, photometric, bitspersample, dib.get());

			// calculate the line + pitch (separate for scr & dest)

			const tmsize_t src_line = TIFFScanlineSize(tif);
			// here, the pitch is 2x less than the original as we only keep the first layer
			int dst_pitch = FreeImage_GetPitch(dib.get());

			// transparency table for 8-bit + 8-bit alpha images

			uint8_t trns[256]; 
			// clear the transparency table
			memset(trns, 0xFF, 256 * sizeof(uint8_t));

			// In the tiff file the lines are saved from up to down 
			// In a DIB the lines must be saved from down to up

			uint8_t *bits = FreeImage_GetScanLine(dib.get(), height - 1);

			// read the tiff lines and save them in the DIB

			if (planar_config == PLANARCONFIG_CONTIG && !header_only) {

				auto buf(std::make_unique<uint8_t[]>(TIFFStripSize(tif)));

				for (uint32_t y = 0; y < height; y += rowsperstrip) {
					const uint32_t nrow = std::min(height - y, rowsperstrip);

					if (TIFFReadEncodedStrip(tif, TIFFComputeStrip(tif, y, 0), buf.get(), nrow * src_line) == -1) {
						throw FI_MSG_ERROR_PARSING;
					}
					for (uint32_t l = 0; l < nrow; l++) {
						uint8_t *p = bits;
						const uint8_t *b = buf.get() + l * src_line;

						for (uint32_t x = 0; x < (uint32_t)(src_line / samplesperpixel); x++) {
							// copy the 8-bit layer
							*p = b[0];
							// convert the 8-bit alpha layer to a trns table
							trns[ b[0] ] = b[1];

							p++;
							b += samplesperpixel;
						}
						bits -= dst_pitch;
					}
				}
			}
			else if (planar_config == PLANARCONFIG_SEPARATE && !header_only) {
				const tmsize_t stripsize = TIFFStripSize(tif) * sizeof(uint8_t);
				auto buf(std::make_unique<uint8_t[]>(2 * stripsize));
				auto *grey = buf.get();
				auto *alpha = buf.get() + stripsize;

				for (uint32_t y = 0; y < height; y += rowsperstrip) {
					const uint32_t nrow = std::min(height - y, rowsperstrip);

					if (TIFFReadEncodedStrip(tif, TIFFComputeStrip(tif, y, 0), grey, nrow * src_line) == -1) {
						throw FI_MSG_ERROR_PARSING;
					}
					if (TIFFReadEncodedStrip(tif, TIFFComputeStrip(tif, y, 1), alpha, nrow * src_line) == -1) {
						throw FI_MSG_ERROR_PARSING;
					}

					for (uint32_t l = 0; l < nrow; l++) {
						uint8_t *p = bits;
						uint8_t *g = grey + l * src_line;
						uint8_t *a = alpha + l * src_line;

						for (uint32_t x = 0; x < (uint32_t)(src_line); x++) {
							// copy the 8-bit layer
							*p = g[0];
							// convert the 8-bit alpha layer to a trns table
							trns[ g[0] ] = a[0];

							p++;
							g++;
							a++;
						}
						bits -= dst_pitch;
					}
				}
			}
			
			FreeImage_SetTransparencyTable(dib.get(), &trns[0], 256);
			FreeImage_SetTransparent(dib.get(), TRUE);

		} else if (loadMethod == LoadAsCMYK) {
			// ---------------------------------------------------------------------------------
			// CMYK loading
			// ---------------------------------------------------------------------------------

			// At this place, samplesperpixel could be > 4, esp. when a CMYK(A) format
			// is recognized. Where all other formats are handled straight-forward, this
			// format has to be handled special 

			FIBOOL isCMYKA = (photometric == PHOTOMETRIC_SEPARATED) && (samplesperpixel > 4);

			// We use a temp dib to store the alpha for the CMYKA to RGBA conversion
			// NOTE this is until we have Extra channels implementation.
			// Also then it will be possible to merge LoadAsCMYK with LoadAsGenericStrip

			std::unique_ptr<FIBITMAP, decltype(&FreeImage_Unload)> alpha(nullptr, &FreeImage_Unload);
			unsigned alpha_pitch = 0;
			uint8_t *alpha_bits{};
			unsigned alpha_Bpp = 0;

			if (isCMYKA && !asCMYK && !header_only) {
				if (bitspersample == 16) {
					alpha.reset(FreeImage_AllocateT(FIT_UINT16, width, height));
				} else if (bitspersample == 8) {
					alpha.reset(FreeImage_Allocate(width, height, 8));
				}

				if (!alpha) {
					FreeImage_OutputMessageProc(s_format_id, "Failed to allocate temporary alpha channel");
				} else {
					alpha_bits = FreeImage_GetScanLine(alpha.get(), height - 1);
					alpha_pitch = FreeImage_GetPitch(alpha.get());
					alpha_Bpp = FreeImage_GetBPP(alpha.get()) / 8;
				}
				
			}

			// create a new DIB
			const uint16_t chCount = std::min<uint16_t>(samplesperpixel, 4);
			dib.reset(CreateImageType(header_only, image_type, width, height, bitspersample, chCount));
			if (!dib) {
				throw FI_MSG_ERROR_DIB_MEMORY;
			}

			// fill in the resolution (english or universal)

			ReadResolution(tif, dib.get());

			if (!header_only) {

				// calculate the line + pitch (separate for scr & dest)

				const tmsize_t src_line = TIFFScanlineSize(tif);
				const tmsize_t dst_line = FreeImage_GetLine(dib.get());
				const unsigned dib_pitch = FreeImage_GetPitch(dib.get());
				const unsigned dibBpp = FreeImage_GetBPP(dib.get()) / 8;
				const unsigned Bpc = dibBpp / chCount;
				const unsigned srcBpp = bitspersample * samplesperpixel / 8;

				assert(Bpc <= 2); //< CMYK is only uint8_t or SHORT 
				
				// In the tiff file the lines are save from up to down 
				// In a DIB the lines must be saved from down to up

				uint8_t *bits = FreeImage_GetScanLine(dib.get(), height - 1);

				// read the tiff lines and save them in the DIB

				auto buf(std::make_unique<uint8_t[]>(TIFFStripSize(tif)));

				if (planar_config == PLANARCONFIG_CONTIG) {

					// - loop for strip blocks -

					for (uint32_t y = 0; y < height; y += rowsperstrip) {
						const uint32_t strips = std::min(height - y, rowsperstrip);

						if (TIFFReadEncodedStrip(tif, TIFFComputeStrip(tif, y, 0), buf.get(), strips * src_line) == -1) {
							throw FI_MSG_ERROR_PARSING;
						}

						// - loop for strips -

						if (src_line != dst_line) {
							// CMYKA+
							if (alpha) {
								for (uint32_t l = 0; l < strips; l++) {
									for (uint8_t *pixel = bits, *al_pixel = alpha_bits, *src_pixel = buf.get() + l * src_line; pixel < bits + dib_pitch; pixel += dibBpp, al_pixel += alpha_Bpp, src_pixel += srcBpp) {
										// copy pixel byte by byte
										uint8_t b = 0;
										for ( ; b < dibBpp; ++b) {
											pixel[b] =  src_pixel[b];
										}
										// TODO write the remaining bytes to extra channel(s)
										
										// HACK write the first alpha to a separate dib (assume uint8_t or uint16_t)
										al_pixel[0] = src_pixel[b];
										if (Bpc > 1) {
											al_pixel[1] = src_pixel[b + 1];
										}
									}
									bits -= dib_pitch;
									alpha_bits -= alpha_pitch;
								}
							}
							else {
								// alpha/extra channels alloc failed
								for (uint32_t l = 0; l < strips; l++) {
									for (uint8_t* pixel = bits, * src_pixel = buf.get() + l * src_line; pixel < bits + dst_line; pixel += dibBpp, src_pixel += srcBpp) {
										AssignPixel(pixel, src_pixel, dibBpp);
									}
									bits -= dib_pitch;
								}
							}
						}
						else { 
							// CMYK to CMYK
							for (uint32_t l = 0; l < strips; l++) {
								uint8_t *b = buf.get() + l * src_line;
								memcpy(bits, b, src_line);
								bits -= dib_pitch;
							}
						}

					} // height
				
				}
				else if (planar_config == PLANARCONFIG_SEPARATE) {

					uint8_t *dib_strip = bits;
					uint8_t *al_strip = alpha_bits;

					// - loop for strip blocks -

					for (uint32_t y = 0; y < height; y += rowsperstrip) {
						const uint32_t strips = std::min(height - y, rowsperstrip);
						
						// - loop for channels (planes) -
						
						for (uint16_t sample = 0; sample < samplesperpixel; sample++) {
							
							if (TIFFReadEncodedStrip(tif, TIFFComputeStrip(tif, y, sample), buf.get(), strips * src_line) == -1) {
								throw FI_MSG_ERROR_PARSING;
							} 
									
							uint8_t *dst_strip = dib_strip;
							unsigned dst_pitch = dib_pitch;
							uint16_t ch = sample;
							unsigned Bpp = dibBpp;

							if (sample >= chCount) {
								// TODO Write to Extra Channel
								
								// HACK redirect write to temp alpha
								if (alpha && sample == chCount) {

									dst_strip = al_strip;
									dst_pitch = alpha_pitch;

									ch = 0;
									Bpp = alpha_Bpp;
								}
								else {
									break; 
								}
							}

							const unsigned channelOffset = ch * Bpc;

							// - loop for strips in block -

							uint8_t *src_line_begin = buf.get();
							uint8_t *dst_line_begin = dst_strip;
							for (uint32_t l = 0; l < strips; l++, src_line_begin += src_line, dst_line_begin -= dst_pitch ) {
								// - loop for pixels in strip -

								const uint8_t* const src_line_end = src_line_begin + src_line;
								for (uint8_t *src_bits = src_line_begin, * dst_bits = dst_line_begin; src_bits < src_line_end; src_bits += Bpc, dst_bits += Bpp) {
									AssignPixel(dst_bits + channelOffset, src_bits, Bpc);
								} // line
							} // strips
						} // channels

						// done with a strip block, incr to the next
						dib_strip -= strips * dib_pitch;
						al_strip -= strips * alpha_pitch;

					} //< height
				}

				if (!asCMYK) {
					ConvertCMYKtoRGBA(dib.get());

					// The ICC Profile is invalid, clear it
					iccSize = 0;
					iccBuf = nullptr;

					if (isCMYKA) {
						// HACK until we have Extra channels. (ConvertCMYKtoRGBA will then do the work)

						FreeImage_SetChannel(dib.get(), alpha.get(), FICC_ALPHA);
					}
					else {
						FIBITMAP *t = RemoveAlphaChannel(dib.get());
						if (t) {
							dib.reset(t);
						}
						else {
							FreeImage_OutputMessageProc(s_format_id, "Cannot allocate memory for buffer. CMYK image converted to RGB + pending Alpha");
						}
					}
				}
			} // !header_only

		} else if (loadMethod == LoadAsGenericStrip) {
			// ---------------------------------------------------------------------------------
			// Generic loading
			// ---------------------------------------------------------------------------------

			// create a new DIB
			const uint16_t chCount = std::min<uint16_t>(samplesperpixel, 4);
			dib.reset(CreateImageType(header_only, image_type, width, height, bitspersample, chCount));
			if (!dib) {
				throw FI_MSG_ERROR_DIB_MEMORY;
			}

			// fill in the resolution (english or universal)

			ReadResolution(tif, dib.get());

			// set up the colormap based on photometric

			ReadPalette(tif, photometric, bitspersample, dib.get());
	
			if (!header_only) {
				// calculate the line + pitch (separate for scr & dest)

				const tmsize_t src_line = TIFFScanlineSize(tif);
				const tmsize_t dst_line = FreeImage_GetLine(dib.get());
				const unsigned dst_pitch = FreeImage_GetPitch(dib.get());
				const unsigned Bpp = FreeImage_GetBPP(dib.get()) / 8;
				const unsigned srcBpp = bitspersample * samplesperpixel / 8;
				const unsigned srcBits = bitspersample * samplesperpixel;

				// In the tiff file the lines are save from up to down 
				// In a DIB the lines must be saved from down to up

				uint8_t *bits = FreeImage_GetScanLine(dib.get(), height - 1);

				// read the tiff lines and save them in the DIB

				auto buf(std::make_unique<uint8_t[]>(TIFFStripSize(tif)));

				FIBOOL bThrowMessage = FALSE;

				if (planar_config == PLANARCONFIG_CONTIG) {

					for (uint32_t y = 0; y < height; y += rowsperstrip) {
						const uint32_t rows = std::min(height - y, rowsperstrip);

						if (TIFFReadEncodedStrip(tif, TIFFComputeStrip(tif, y, 0), buf.get(), rows * src_line) == -1) {
							// ignore errors as they can be frequent and not really valid errors, especially with fax images
							bThrowMessage = TRUE;
							/*
							throw FI_MSG_ERROR_PARSING;
							*/
						}
						if (src_line == dst_line) {
							// channel count match
							for (uint32_t l = 0; l < rows; l++) {
								memcpy(bits, buf.get() + l * src_line, src_line);
								bits -= dst_pitch;
							}
						}
						else {
							if (srcBpp * 8 == srcBits) {
								for (uint32_t l = 0; l < rows; l++) {
									for (uint8_t* pixel = bits, *src_pixel = buf.get() + l * src_line; pixel < bits + dst_pitch; pixel += Bpp, src_pixel += srcBpp) {
										AssignPixel(pixel, src_pixel, Bpp);
									}
									bits -= dst_pitch;
								}
							}
							else { // not whole number of bytes
								if (bitspersample <= 8) {
									DecodeStrip<uint8_t>(buf.get(), src_line, bits, dst_pitch, rows, 0, 1, bitspersample);
								}
								else if (bitspersample <= 16) {
									DecodeStrip<uint16_t>(buf.get(), src_line, bits, dst_pitch, rows, 0, 1, bitspersample);
								}
								else {
									throw "Unsupported number of bits per sample";
								}
								bits -= rows * dst_pitch;
							}
						}
					}
				}
				else if (planar_config == PLANARCONFIG_SEPARATE) {

					const unsigned Bpc = bitspersample / 8;
					uint8_t* dib_strip = bits;
					// - loop for strip blocks -

					for (uint32_t y = 0; y < height; y += rowsperstrip) {
						const uint32_t strips = std::min(height - y, rowsperstrip);

						// - loop for channels (planes) -

						for (uint16_t sample = 0; sample < samplesperpixel; sample++) {

							if (TIFFReadEncodedStrip(tif, TIFFComputeStrip(tif, y, sample), buf.get(), strips * src_line) == -1) {
								// ignore errors as they can be frequent and not really valid errors, especially with fax images
								bThrowMessage = TRUE;
							}

							if (sample >= chCount) {
								// TODO Write to Extra Channel
								break;
							}

							if (src_line == dst_line && 1 == samplesperpixel) {
								uint8_t* dst_line_begin = dib_strip;
								for (uint32_t l = 0; l < strips; l++) {
									memcpy(dst_line_begin, buf.get() + l * src_line, src_line);
									dst_line_begin -= dst_pitch;
								}
							}
							else {
								if (0 == (bitspersample & 7)) {
									const unsigned channelOffset = sample * Bpc;

									// - loop for strips in block -

									uint8_t* src_line_begin = buf.get();
									uint8_t* dst_line_begin = dib_strip;
									for (uint32_t l = 0; l < strips; l++, src_line_begin += src_line, dst_line_begin -= dst_pitch) {

										// - loop for pixels in strip -

										const uint8_t* const src_line_end = src_line_begin + src_line;

										for (uint8_t* src_bits = src_line_begin, *dst_bits = dst_line_begin; src_bits < src_line_end; src_bits += Bpc, dst_bits += Bpp) {
											// actually assigns channel
											AssignPixel(dst_bits + channelOffset, src_bits, Bpc);
										} // line
									} // strips
								}
								else { // not whole number of bytes
									if (bitspersample <= 8) {
										DecodeStrip<uint8_t>(buf.get(), src_line,  dib_strip, dst_pitch, strips, sample, chCount, bitspersample);
									}
									else if (bitspersample <= 16) {
										DecodeStrip<uint16_t>(buf.get(), src_line, dib_strip, dst_pitch, strips, sample, chCount, bitspersample);
									}
									else {
										throw "Unsupported number of bits per sample";
									}
								}
							}
						} // channels
						// done with a strip block, incr to the next
						dib_strip -= strips * dst_pitch;
					} // height
				}

				if (bThrowMessage) {
					FreeImage_OutputMessageProc(s_format_id, "Warning: parsing error. Image may be incomplete or contain invalid data !");
				}

#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
				SwapRedBlue32(dib.get());
#endif

			} // !header only

		} else if (loadMethod == LoadAsTiled) {
			// ---------------------------------------------------------------------------------
			// Tiled image loading
			// ---------------------------------------------------------------------------------

			uint32_t tileWidth, tileHeight;
			uint32_t src_line = 0;

			// create a new DIB
			dib.reset(CreateImageType( header_only, image_type, width, height, bitspersample, samplesperpixel));
			if (!dib) {
				throw FI_MSG_ERROR_DIB_MEMORY;
			}

			// fill in the resolution (english or universal)

			ReadResolution(tif, dib.get());

			// set up the colormap based on photometric

			ReadPalette(tif, photometric, bitspersample, dib.get());

			// get the tile geometry
			if (!TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tileWidth) || !TIFFGetField(tif, TIFFTAG_TILELENGTH, &tileHeight)) {
				throw "Invalid tiled TIFF image";
			}

			// read the tiff lines and save them in the DIB

			if (planar_config == PLANARCONFIG_CONTIG && !header_only) {
				
				// get the maximum number of bytes required to contain a tile
				const tmsize_t tileSize = TIFFTileSize(tif);

				// allocate tile buffer
				auto tileBuffer(std::make_unique<uint8_t[]>(tileSize));

				// calculate src line and dst pitch
				const int dst_pitch = FreeImage_GetPitch(dib.get());
				const uint32_t tileRowSize = (uint32_t)TIFFTileRowSize(tif);
				const uint32_t imageRowSize = (uint32_t)TIFFScanlineSize(tif);


				// In the tiff file the lines are saved from up to down 
				// In a DIB the lines must be saved from down to up

				uint8_t *bits = FreeImage_GetScanLine(dib.get(), height - 1);
				
				for (uint32_t y = 0; y < height; y += tileHeight) {
					const uint32_t nrows = std::min(height - y, tileHeight);

					for (uint32_t x = 0, rowSize = 0; x < width; x += tileWidth, rowSize += tileRowSize) {
						memset(tileBuffer.get(), 0, tileSize);

						// read one tile
						if (TIFFReadTile(tif, tileBuffer.get(), x, y, 0, 0) < 0) {
							throw "Corrupted tiled TIFF file";
						}
						// convert to strip
						if (x + tileWidth > width) {
							src_line = imageRowSize - rowSize;
						} else {
							src_line = tileRowSize;
						}
						const uint8_t *src_bits = tileBuffer.get();
						uint8_t *dst_bits = bits + rowSize;
						for (uint32_t k = 0; k < nrows; k++) {
							memcpy(dst_bits, src_bits, src_line);
							src_bits += tileRowSize;
							dst_bits -= dst_pitch;
						}
					}

					bits -= nrows * dst_pitch;
				}

#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
				SwapRedBlue32(dib.get());
#endif
			}
			else if (planar_config == PLANARCONFIG_SEPARATE) {
				throw "Separated tiled TIFF images are not supported";
			}


		} else if (loadMethod == LoadAsLogLuv) {
			// ---------------------------------------------------------------------------------
			// RGBF LogLuv compressed loading
			// ---------------------------------------------------------------------------------

			double	stonits;	// input conversion to nits
			if (!TIFFGetField(tif, TIFFTAG_STONITS, &stonits)) {
				stonits = 1;
			}

			// create a new DIB
			dib.reset(CreateImageType(header_only, image_type, width, height, bitspersample, samplesperpixel));
			if (!dib) {
				throw FI_MSG_ERROR_DIB_MEMORY;
			}

			// fill in the resolution (english or universal)

			ReadResolution(tif, dib.get());

			if (planar_config == PLANARCONFIG_CONTIG && !header_only) {
				// calculate the line + pitch (separate for scr & dest)

				const tmsize_t src_line = TIFFScanlineSize(tif);
				const int dst_pitch = FreeImage_GetPitch(dib.get());

				// In the tiff file the lines are save from up to down 
				// In a DIB the lines must be saved from down to up

				uint8_t *bits = FreeImage_GetScanLine(dib.get(), height - 1);

				// read the tiff lines and save them in the DIB

				auto buf(std::make_unique<uint8_t[]>(TIFFStripSize(tif)));

				for (uint32_t y = 0; y < height; y += rowsperstrip) {
					const uint32_t nrow = std::min(height - y, rowsperstrip);

					if (TIFFReadEncodedStrip(tif, TIFFComputeStrip(tif, y, 0), buf.get(), nrow * src_line) == -1) {
						throw FI_MSG_ERROR_PARSING;
					}
					// convert from XYZ to RGB
					for (uint32_t l = 0; l < nrow; l++) {
						tiff_ConvertLineXYZToRGB(bits, buf.get() + l * src_line, stonits, width);
						bits -= dst_pitch;
					}
				}
			}
			else if (planar_config == PLANARCONFIG_SEPARATE) {
				// this cannot happen according to the LogLuv specification
				throw "Unable to handle PLANARCONFIG_SEPARATE LogLuv images";
			}

		} else if (loadMethod == LoadAsHalfFloat) {
			// ---------------------------------------------------------------------------------
			// RGBF loading from a half format
			// ---------------------------------------------------------------------------------

			// create a new DIB
			dib.reset(CreateImageType(header_only, image_type, width, height, bitspersample, samplesperpixel));
			if (!dib) {
				throw FI_MSG_ERROR_DIB_MEMORY;
			}

			// fill in the resolution (english or universal)

			ReadResolution(tif, dib.get());

			if (!header_only) {

				// calculate the line + pitch (separate for scr & dest)

				const tmsize_t src_line = TIFFScanlineSize(tif);
				const unsigned dst_pitch = FreeImage_GetPitch(dib.get());

				// In the tiff file the lines are save from up to down 
				// In a DIB the lines must be saved from down to up

				uint8_t *bits = FreeImage_GetScanLine(dib.get(), height - 1);

				// read the tiff lines and save them in the DIB

				if (planar_config == PLANARCONFIG_CONTIG) {

					auto buf(std::make_unique<uint8_t[]>(TIFFStripSize(tif)));

					for (uint32_t y = 0; y < height; y += rowsperstrip) {
						const uint32_t nrow = std::min(height - y, rowsperstrip);

						if (TIFFReadEncodedStrip(tif, TIFFComputeStrip(tif, y, 0), buf.get(), nrow * src_line) == -1) {
							throw FI_MSG_ERROR_PARSING;
						} 

						// convert from half (16-bit) to float (32-bit)
						// !!! use OpenEXR half helper class

						half half_value;

						for (uint32_t l = 0; l < nrow; l++) {
							uint16_t *src_pixel = (uint16_t*)(buf.get() + l * src_line);
							float *dst_pixel = (float*)bits;

							for (tmsize_t x = 0; x < (tmsize_t)(src_line / sizeof(uint16_t)); x++) {
								half_value.setBits(src_pixel[x]);
								dst_pixel[x] = half_value;
							}

							bits -= dst_pitch;
						}
					}
				}
				else if (planar_config == PLANARCONFIG_SEPARATE) {
					// this use case was never encountered yet
					throw "Unable to handle PLANARCONFIG_SEPARATE RGB half float images";
				}
				
			} // !header only

		} else {
			// ---------------------------------------------------------------------------------
			// Unknown or unsupported format
			// ---------------------------------------------------------------------------------

			throw FI_MSG_ERROR_UNSUPPORTED_FORMAT;
		}
		
		// copy TIFF metadata (must be done after FreeImage_Allocate)

		ReadMetadata(io, handle, tif, dib.get());

		// copy ICC profile data (must be done after FreeImage_Allocate)
		
		FreeImage_CreateICCProfile(dib.get(), iccBuf, iccSize);
		if (photometric == PHOTOMETRIC_SEPARATED) {
			if (asCMYK) {
				// set the ICC profile as CMYK
				FreeImage_GetICCProfile(dib.get())->flags |= FIICC_COLOR_IS_CMYK;
			}
			else {
				// if original image is CMYK but is converted to RGB, remove ICC profile from Exif-TIFF metadata
				FreeImage_SetMetadata(FIMD_EXIF_MAIN, dib.get(), "InterColorProfile", nullptr);
			}
		}

		// copy TIFF thumbnail (must be done after FreeImage_Allocate)
		
		ReadThumbnail(io, handle, data, tif, dib.get());

		return dib.release();

	}
	catch (const char *message) {
		if (message) {
			FreeImage_OutputMessageProc(s_format_id, message);
		}
	}
	catch (const std::bad_alloc &) {
		FreeImage_OutputMessageProc(s_format_id, FI_MSG_ERROR_MEMORY);
	}
	return nullptr;
}

// --------------------------------------------------------------------------

/**
Save a single image into a TIF

@param io FreeImage IO
@param dib The dib to be saved
@param handle FreeImage handle
@param page Page number
@param flags FreeImage TIFF save flag
@param data TIFF plugin context
@param ifd TIFF Image File Directory (0 means save image, > 0 && (page == -1) means save thumbnail)
@param ifdCount 1 if no thumbnail to save, 2 if image + thumbnail to save
@return Returns TRUE if successful, returns FALSE otherwise
*/
static FIBOOL 
SaveOneTIFF(FreeImageIO *io, FIBITMAP *dib, fi_handle handle, int page, int flags, void *data, unsigned ifd, unsigned ifdCount) {
	if (!dib || !handle || !data) {
		return FALSE;
	}

	try {
		fi_TIFFIO *fio = (fi_TIFFIO*)data;
		TIFF *out = fio->tif;

		const FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(dib);

		const uint32_t width = FreeImage_GetWidth(dib);
		const uint32_t height = FreeImage_GetHeight(dib);
		const uint16_t bitsperpixel = (uint16_t)FreeImage_GetBPP(dib);

		const FIICCPROFILE* iccProfile = FreeImage_GetICCProfile(dib);
		
		// setup out-variables based on dib and flag options
		
		uint16_t bitspersample;
		uint16_t samplesperpixel;
		uint16_t photometric;

		if (image_type == FIT_BITMAP) {
			// standard image: 1-, 4-, 8-, 16-, 24-, 32-bit

			samplesperpixel = ((bitsperpixel == 24) ? 3 : ((bitsperpixel == 32) ? 4 : 1));
			bitspersample = bitsperpixel / samplesperpixel;
			photometric	= GetPhotometric(dib);

			if ((bitsperpixel == 8) && FreeImage_IsTransparent(dib)) {
				// 8-bit transparent picture : convert later to 8-bit + 8-bit alpha
				samplesperpixel = 2;
				bitspersample = 8;
			}
			else if (bitsperpixel == 32) {
				// 32-bit images : check for CMYK or alpha transparency

				if ((((iccProfile->flags & FIICC_COLOR_IS_CMYK) == FIICC_COLOR_IS_CMYK) || ((flags & TIFF_CMYK) == TIFF_CMYK))) {
					// CMYK support
					photometric = PHOTOMETRIC_SEPARATED;
					TIFFSetField(out, TIFFTAG_INKSET, INKSET_CMYK);
					TIFFSetField(out, TIFFTAG_NUMBEROFINKS, 4);
				}
				else if (photometric == PHOTOMETRIC_RGB) {
					// transparency mask support
					uint16_t sampleinfo[1]; 
					// unassociated alpha data is transparency information
					sampleinfo[0] = EXTRASAMPLE_UNASSALPHA;
					TIFFSetField(out, TIFFTAG_EXTRASAMPLES, 1, sampleinfo);
				}
			}
		} else if (image_type == FIT_RGB16) {
			// 48-bit RGB

			samplesperpixel = 3;
			bitspersample = bitsperpixel / samplesperpixel;
			photometric	= PHOTOMETRIC_RGB;
		} else if (image_type == FIT_RGBA16) {
			// 64-bit RGBA

			samplesperpixel = 4;
			bitspersample = bitsperpixel / samplesperpixel;
			if ((((iccProfile->flags & FIICC_COLOR_IS_CMYK) == FIICC_COLOR_IS_CMYK) || ((flags & TIFF_CMYK) == TIFF_CMYK))) {
				// CMYK support
				photometric = PHOTOMETRIC_SEPARATED;
				TIFFSetField(out, TIFFTAG_INKSET, INKSET_CMYK);
				TIFFSetField(out, TIFFTAG_NUMBEROFINKS, 4);
			}
			else {
				photometric	= PHOTOMETRIC_RGB;
				// transparency mask support
				uint16_t sampleinfo[1]; 
				// unassociated alpha data is transparency information
				sampleinfo[0] = EXTRASAMPLE_UNASSALPHA;
				TIFFSetField(out, TIFFTAG_EXTRASAMPLES, 1, sampleinfo);
			}
		} else if (image_type == FIT_RGBF) {
			// 96-bit RGBF => store with a LogLuv encoding ?

			samplesperpixel = 3;
			bitspersample = bitsperpixel / samplesperpixel;
			// the library converts to and from floating-point XYZ CIE values
			if ((flags & TIFF_LOGLUV) == TIFF_LOGLUV) {
				photometric	= PHOTOMETRIC_LOGLUV;
				TIFFSetField(out, TIFFTAG_SGILOGDATAFMT, SGILOGDATAFMT_FLOAT);
				// TIFFSetField(out, TIFFTAG_STONITS, 1.0);   // assume unknown 
			}
			else {
				// store with default compression (LZW) or with input compression flag
				photometric	= PHOTOMETRIC_RGB;
			}

		} else if (image_type == FIT_RGBAF) {
			// 128-bit RGBAF => store with default compression (LZW) or with input compression flag

			samplesperpixel = 4;
			bitspersample = bitsperpixel / samplesperpixel;
			photometric	= PHOTOMETRIC_RGB;
		} else {
			// special image type (int, long, double, ...)

			samplesperpixel = 1;
			bitspersample = bitsperpixel;
			photometric	= PHOTOMETRIC_MINISBLACK;
		}

		// set image data type

		WriteImageType(out, image_type);
		
		// write possible ICC profile

		if (iccProfile->size && iccProfile->data) {
			TIFFSetField(out, TIFFTAG_ICCPROFILE, iccProfile->size, iccProfile->data);
		}

		// handle standard width/height/bpp stuff

		TIFFSetField(out, TIFFTAG_IMAGEWIDTH, width);
		TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);
		TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, samplesperpixel);
		TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, bitspersample);
		TIFFSetField(out, TIFFTAG_PHOTOMETRIC, photometric);
		TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);	// single image plane 
		TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(out, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
		TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, (uint32_t) -1)); 

		// handle metrics

		WriteResolution(out, dib);

		// multi-paging

		if (page >= 0) {
			char page_number[20];
			snprintf(page_number, std::size(page_number), "Page %d", page);

			TIFFSetField(out, TIFFTAG_SUBFILETYPE, (uint32_t)FILETYPE_PAGE);
			TIFFSetField(out, TIFFTAG_PAGENUMBER, (uint16_t)page, (uint16_t)0);
			TIFFSetField(out, TIFFTAG_PAGENAME, page_number);

		} else {
			// is it a thumbnail ? 
			TIFFSetField(out, TIFFTAG_SUBFILETYPE, (ifd == 0) ? (uint32_t)0 : (uint32_t)FILETYPE_REDUCEDIMAGE);
		}

		// palettes (image colormaps are automatically scaled to 16-bits)

		if (photometric == PHOTOMETRIC_PALETTE) {
			uint16_t nColors = (uint16_t)FreeImage_GetColorsUsed(dib);
			FIRGBA8 *pal = FreeImage_GetPalette(dib);

			auto safeBuffer(std::make_unique<uint16_t[]>(3 * nColors));
			auto *r = safeBuffer.get();
			auto *g = r + nColors;
			auto *b = g + nColors;

			for (int i = nColors; i-- > 0; ) {
				r[i] = SCALE((uint16_t)pal[i].red);
				g[i] = SCALE((uint16_t)pal[i].green);
				b[i] = SCALE((uint16_t)pal[i].blue);
			}

			TIFFSetField(out, TIFFTAG_COLORMAP, r, g, b);
		}

		// compression tag

		WriteCompression(out, bitspersample, samplesperpixel, photometric, flags);

		// metadata

		WriteMetadata(out, dib);

		// thumbnail tag

		if ((ifd == 0) && (ifdCount > 1)) {
			uint16_t nsubifd = 1;
			uint64_t subifd[1];
			subifd[0] = 0;
			TIFFSetField(out, TIFFTAG_SUBIFD, nsubifd, subifd);
		}

		// read the DIB lines from bottom to top
		// and save them in the TIF
		// -------------------------------------

		const uint32_t pitch = FreeImage_GetPitch(dib);

		if (image_type == FIT_BITMAP) {
			// standard bitmap type

			switch (bitsperpixel) {
				case 1 :
				case 4 :
				case 8 :
				{
					if ((bitsperpixel == 8) && FreeImage_IsTransparent(dib)) {
						// 8-bit transparent picture : convert to 8-bit + 8-bit alpha

						// get the transparency table
						uint8_t *trns = FreeImage_GetTransparencyTable(dib);

						auto buffer(std::make_unique<uint8_t[]>(static_cast<size_t>(width) * 2));

						for (int y = height - 1; y >= 0; y--) {
							const uint8_t *bits = FreeImage_GetScanLine(dib, y);

							auto *b = buffer.get();

							for (uint32_t x = 0; x < width; x++) {
								// copy the 8-bit layer
								b[0] = *bits;
								// convert the trns table to a 8-bit alpha layer
								b[1] = trns[ b[0] ];

								bits++;
								b += samplesperpixel;
							}

							// write the scanline to disc

							TIFFWriteScanline(out, buffer.get(), height - y - 1, 0);
						}
					}
					else {
						// other cases
						auto buffer(std::make_unique<uint8_t[]>(pitch));

						for (uint32_t y = 0; y < height; y++) {
							// get a copy of the scanline
							memcpy(buffer.get(), FreeImage_GetScanLine(dib, height - y - 1), pitch);
							// write the scanline to disc
							TIFFWriteScanline(out, buffer.get(), y, 0);
						}
					}

					break;
				}

				case 24:
				case 32:
				{
					auto buffer(std::make_unique<uint8_t[]>(pitch));

					for (uint32_t y = 0; y < height; y++) {
						// get a copy of the scanline

						memcpy(buffer.get(), FreeImage_GetScanLine(dib, height - y - 1), pitch);

#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
						if (photometric != PHOTOMETRIC_SEPARATED) {
							// TIFFs store color data RGB(A) instead of BGR(A)
		
							auto *pBuf = static_cast<uint8_t*>(buffer.get());
		
							for (uint32_t x = 0; x < width; x++) {
								INPLACESWAP(pBuf[0], pBuf[2]);
								pBuf += samplesperpixel;
							}
						}
#endif
						// write the scanline to disc

						TIFFWriteScanline(out, buffer.get(), y, 0);
					}

					break;
				}
			}//< switch (bitsperpixel)

		} else if (image_type == FIT_RGBF && (flags & TIFF_LOGLUV) == TIFF_LOGLUV) {
			// RGBF image => store as XYZ using a LogLuv encoding

			auto buffer(std::make_unique<uint8_t[]>(pitch));

			for (uint32_t y = 0; y < height; y++) {
				// get a copy of the scanline and convert from RGB to XYZ
				tiff_ConvertLineRGBToXYZ(static_cast<uint8_t*>(buffer.get()), FreeImage_GetScanLine(dib, height - y - 1), width);
				// write the scanline to disc
				TIFFWriteScanline(out, buffer.get(), y, 0);
			}
		} else {
			// just dump the dib (tiff supports all dib types)

			auto buffer(std::make_unique<uint8_t[]>(pitch));

			for (uint32_t y = 0; y < height; y++) {
				// get a copy of the scanline
				memcpy(buffer.get(), FreeImage_GetScanLine(dib, height - y - 1), pitch);
				// write the scanline to disc
				TIFFWriteScanline(out, buffer.get(), y, 0);
			}
		}

		// write out the directory tag if we wrote a page other than -1 or if we have a thumbnail to write later

		if ((page >= 0) || ((ifd == 0) && (ifdCount > 1))) {
			TIFFWriteDirectory(out);
			// else: TIFFClose will WriteDirectory
		}

		return TRUE;

	}
	catch(const char *text) {
		FreeImage_OutputMessageProc(s_format_id, text);
	}
	catch (const std::bad_alloc &) {
		FreeImage_OutputMessageProc(s_format_id, FI_MSG_ERROR_MEMORY);
	}
	return FALSE;
}

static FIBOOL DLL_CALLCONV
Save(FreeImageIO *io, FIBITMAP *dib, fi_handle handle, int page, int flags, void *data) {
	FIBOOL bResult = FALSE;

	// handle thumbnail as SubIFD
	const FIBOOL bHasThumbnail = (FreeImage_GetThumbnail(dib) != nullptr);
	const unsigned ifdCount = bHasThumbnail ? 2 : 1;

	FIBITMAP *bitmap = dib;

	for (unsigned ifd = 0; ifd < ifdCount; ifd++) {
		// redirect dib to thumbnail for the second pass
		if (ifd == 1) {
			bitmap = FreeImage_GetThumbnail(dib);
		}

		bResult = SaveOneTIFF(io, bitmap, handle, page, flags, data, ifd, ifdCount);
		if (!bResult) {
			return FALSE;
		}
	}

	return bResult;
}

// ==========================================================
//   Init
// ==========================================================

void DLL_CALLCONV
InitTIFF(Plugin *plugin, int format_id) {
	_TIFFwarningHandler = msdosWarningHandler;
	_TIFFerrorHandler = msdosErrorHandler;
	s_format_id = format_id;

    // Set up the callback for extended TIFF directory tag support (see XTIFF.cpp)
	// Must be called before using libtiff
    XTIFFInitialize();	

	plugin->format_proc = Format;
	plugin->description_proc = Description;
	plugin->extension_proc = Extension;
	plugin->regexpr_proc = RegExpr;
	plugin->open_proc = Open;
	plugin->close_proc = Close;
	plugin->pagecount_proc = PageCount;
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


std::unique_ptr<FIDEPENDENCY> MakeTiffDependencyInfo() {
	auto info = std::make_unique<FIDEPENDENCY>();
	info->name = "LibTIFF";
	info->fullVersion = TIFFLIB_VERSION_STR_MAJ_MIN_MIC;
	info->majorVersion = TIFFLIB_MAJOR_VERSION;
	info->minorVersion = TIFFLIB_MINOR_VERSION;
	return info;
}
