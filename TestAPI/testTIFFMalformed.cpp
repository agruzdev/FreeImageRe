// ==========================================================
// FreeImage 3 Test Script
//
// TIFF: malformed sample layouts
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


#include "TestSuite.h"

#if FREEIMAGE_WITH_LIBTIFF

#include <vector>
#include <cstdint>
#include <cstdlib>
#ifdef _WIN32
#include <thread>
#include <future>
#include <chrono>
#endif

// ----------------------------------------------------------

static void push16(std::vector<uint8_t>& v, unsigned value) {
	v.push_back((uint8_t)(value & 0xFF));
	v.push_back((uint8_t)(value >> 8));
}

static void push32(std::vector<uint8_t>& v, unsigned value) {
	push16(v, value & 0xFFFF);
	push16(v, value >> 16);
}

// TIFF tag types, photometric interpretations, planar configurations and sample formats
enum { TT_SHORT = 3, TT_LONG = 4 };
enum { PHOTOMETRIC_MINISWHITE = 0, PHOTOMETRIC_MINISBLACK = 1, PHOTOMETRIC_RGB = 2, PHOTOMETRIC_SEPARATED = 5 };
enum { PLANARCONFIG_CONTIG = 1, PLANARCONFIG_SEPARATE = 2 };
enum { SAMPLEFORMAT_INT = 2, SAMPLEFORMAT_IEEEFP = 3 };

struct IfdEntry {
	unsigned tag;
	unsigned type;
	std::vector<unsigned> values;
};

static IfdEntry entry(unsigned tag, unsigned type, std::vector<unsigned> values) {
	IfdEntry e;
	e.tag = tag;
	e.type = type;
	e.values = values;
	return e;
}

/**
Build a little endian, uncompressed 4 x 4 TIFF with one strip per plane (rowsperstrip = 4)
@param photometric PhotometricInterpretation
@param bitspersample BitsPerSample of every sample
@param samplesperpixel SamplesPerPixel
@param planar PlanarConfiguration (1 = contiguous, 2 = separate)
@param sampleformat SampleFormat of every sample, 0 to omit the tag
@param tiled use one 16 x 16 tile per plane instead of one strip per plane
*/
static std::vector<uint8_t> makeTiff(unsigned photometric, unsigned bitspersample, unsigned samplesperpixel, unsigned planar, unsigned sampleformat, bool tiled = false) {
	const unsigned width = tiled ? 16 : 4, height = tiled ? 16 : 4;
	const unsigned strips = (planar == 2) ? samplesperpixel : 1;
	const unsigned strip_size = (tiled ? 16 * 16 : width * height) * bitspersample / 8 * ((planar == 2) ? 1 : samplesperpixel);

	std::vector<IfdEntry> e;
	e.push_back(entry(256, TT_SHORT, { width }));
	e.push_back(entry(257, TT_SHORT, { height }));
	e.push_back(entry(258, TT_SHORT, std::vector<unsigned>(samplesperpixel, bitspersample)));
	e.push_back(entry(259, TT_SHORT, { 1 }));
	e.push_back(entry(262, TT_SHORT, { photometric }));
	if (!tiled) {
		e.push_back(entry(273, TT_LONG, std::vector<unsigned>(strips, 0))); // offsets are patched below
	}
	e.push_back(entry(277, TT_SHORT, { samplesperpixel }));
	if (!tiled) {
		e.push_back(entry(278, TT_SHORT, { height }));
		e.push_back(entry(279, TT_LONG, std::vector<unsigned>(strips, strip_size)));
	}
	e.push_back(entry(284, TT_SHORT, { planar }));
	if (tiled) {
		e.push_back(entry(322, TT_SHORT, { 16 }));
		e.push_back(entry(323, TT_SHORT, { 16 }));
		e.push_back(entry(324, TT_LONG, std::vector<unsigned>(strips, 0))); // offsets are patched below
		e.push_back(entry(325, TT_LONG, std::vector<unsigned>(strips, strip_size)));
	}
	if (sampleformat) {
		e.push_back(entry(339, TT_SHORT, std::vector<unsigned>(samplesperpixel, sampleformat)));
	}

	// header, IFD, then the values which do not fit into the entry, then the pixel data
	unsigned extra_offset = 8 + 2 + 12 * (unsigned)e.size() + 4;
	std::vector<unsigned> value_offsets;
	unsigned extra_size = 0;
	for (size_t i = 0; i < e.size(); i++) {
		const unsigned size = (unsigned)e[i].values.size() * ((e[i].type == TT_SHORT) ? 2 : 4);
		value_offsets.push_back(extra_offset + extra_size);
		if (size > 4) {
			extra_size += size;
		}
	}
	const unsigned data_offset = extra_offset + extra_size;
	for (size_t i = 0; i < e.size(); i++) {
		if (e[i].tag == 273 || e[i].tag == 324) {
			for (unsigned s = 0; s < strips; s++) {
				e[i].values[s] = data_offset + s * strip_size;
			}
		}
	}

	std::vector<uint8_t> t;
	t.push_back('I'); t.push_back('I'); push16(t, 42); push32(t, 8);
	push16(t, (unsigned)e.size());
	for (size_t i = 0; i < e.size(); i++) {
		const bool is_short = (e[i].type == TT_SHORT);
		const unsigned size = (unsigned)e[i].values.size() * (is_short ? 2 : 4);
		push16(t, e[i].tag);
		push16(t, e[i].type);
		push32(t, (unsigned)e[i].values.size());
		if (size > 4) {
			push32(t, value_offsets[i]);
		} else {
			unsigned inline_size = 0;
			for (size_t k = 0; k < e[i].values.size(); k++) {
				if (is_short) push16(t, e[i].values[k]); else push32(t, e[i].values[k]);
				inline_size += is_short ? 2 : 4;
			}
			for ( ; inline_size < 4; inline_size++) {
				t.push_back(0);
			}
		}
	}
	push32(t, 0);
	for (size_t i = 0; i < e.size(); i++) {
		const bool is_short = (e[i].type == TT_SHORT);
		if (e[i].values.size() * (is_short ? 2 : 4) > 4) {
			for (size_t k = 0; k < e[i].values.size(); k++) {
				if (is_short) push16(t, e[i].values[k]); else push32(t, e[i].values[k]);
			}
		}
	}
	for (unsigned i = 0; i < strips * strip_size; i++) {
		t.push_back((uint8_t)(i * 7 + 1));
	}
	return t;
}

/**
Load the TIFF from memory, returns the bits per pixel of the result (0 if the file is rejected)
*/
static unsigned loadTiff(const std::vector<uint8_t>& tiff) {
	FIMEMORY *hmem = FreeImage_OpenMemory(const_cast<uint8_t*>(tiff.data()), (uint32_t)tiff.size());
	FIBITMAP *dib = FreeImage_LoadFromMemory(FIF_TIFF, hmem, 0);
	FreeImage_CloseMemory(hmem);
	if (!dib) {
		return 0;
	}
	const unsigned bpp = FreeImage_GetBPP(dib);
	FreeImage_Unload(dib);
	return bpp;
}

/**
Same as loadTiff, but a file which makes the loader spin forever is reported as a failure instead of hanging the test suite
*/
static unsigned loadTiffGuarded(const std::vector<uint8_t>& tiff) {
#ifdef _WIN32
	std::packaged_task<unsigned()> task([&tiff]() { return loadTiff(tiff); });
	std::future<unsigned> result = task.get_future();
	std::thread worker(std::move(task));
	if (result.wait_for(std::chrono::seconds(20)) != std::future_status::ready) {
		printf("testTIFFMalformed: FAILED: the loader does not return\n");
		fflush(stdout);
		std::_Exit(1);
	}
	worker.join();
	return result.get();
#else
	return loadTiff(tiff);
#endif
}

static bool expect(bool condition, const char *what) {
	if (!condition) {
		printf("testTIFFMalformed: FAILED: %s\n", what);
	}
	return condition;
}

// ----------------------------------------------------------

bool testTIFFMalformed() {
	printf("testTIFFMalformed ...\n");
	bool ok = true;

	// valid files must still load
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_RGB, 8, 3, PLANARCONFIG_CONTIG, 0)) == 24, "RGB, contiguous");
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_RGB, 8, 3, PLANARCONFIG_SEPARATE, 0)) == 24, "RGB, separate planes");
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_RGB, 8, 4, PLANARCONFIG_SEPARATE, 0)) == 32, "RGBA, separate planes");
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_RGB, 16, 3, PLANARCONFIG_SEPARATE, 0)) == 48, "RGB16, separate planes");

	// 2 samples of 8 bits are loaded as an 8-bit DIB, so the second plane has no room in the DIB pixel
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_RGB, 8, 2, PLANARCONFIG_SEPARATE, 0)) == 0, "RGB with 2 separate samples");

	// 3 x 16-bit float samples are loaded as RGBF (12 bytes per pixel), but a source pixel has 6 bytes
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_MINISWHITE, 16, 3, PLANARCONFIG_CONTIG, SAMPLEFORMAT_IEEEFP)) == 0, "3 x half float, not RGB, contiguous");
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_SEPARATED, 16, 3, PLANARCONFIG_CONTIG, SAMPLEFORMAT_IEEEFP)) == 0, "3 x half float, CMYK, contiguous");

	// what is supported has to stay loadable: half float RGB (as RGBF) and integer CMYK (converted to RGB)
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_RGB, 16, 3, PLANARCONFIG_CONTIG, SAMPLEFORMAT_IEEEFP)) == 96, "3 x half float, RGB, contiguous");
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_SEPARATED, 8, 4, PLANARCONFIG_CONTIG, 0)) == 24, "CMYK, 8-bit, contiguous");
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_SEPARATED, 16, 4, PLANARCONFIG_CONTIG, 0)) == 48, "CMYK, 16-bit, contiguous");

	// 3 x 16-bit signed samples are loaded as a 32-bit DIB, the channels do not fit into a pixel
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_RGB, 16, 3, PLANARCONFIG_SEPARATE, SAMPLEFORMAT_INT)) == 0, "3 x int16, separate planes");

	// tiled files: 16 x 16 pixels in a single tile
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_RGB, 8, 3, PLANARCONFIG_CONTIG, 0, true)) == 24, "tiled RGB, contiguous");
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_RGB, 8, 3, PLANARCONFIG_SEPARATE, 0, true)) == 24, "tiled RGB, separate planes");
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_MINISBLACK, 8, 1, PLANARCONFIG_CONTIG, 0, true)) == 8, "tiled 8-bit grey");
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_MINISBLACK, 4, 1, PLANARCONFIG_CONTIG, 0, true)) == 4, "tiled 4-bit grey");
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_MINISBLACK, 4, 1, PLANARCONFIG_CONTIG, 0)) == 4, "4-bit grey");

	// the tile loader copies a row of DIB pixels from a row of source pixels, which are smaller (6 bytes, DIB is RGBF)
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_MINISWHITE, 16, 3, PLANARCONFIG_CONTIG, SAMPLEFORMAT_IEEEFP, true)) == 0, "tiled 3 x half float, contiguous");

	// the tile loader writes every sample as an element of the DIB pixel, but the DIB pixel is smaller
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_RGB, 8, 2, PLANARCONFIG_SEPARATE, 0, true)) == 0, "tiled RGB with 2 separate samples");
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_RGB, 16, 3, PLANARCONFIG_SEPARATE, SAMPLEFORMAT_INT, true)) == 0, "tiled 3 x int16, separate planes");
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_MINISBLACK, 4, 1, PLANARCONFIG_SEPARATE, 0, true)) == 0, "tiled 4-bit grey, separate planes");
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_MINISBLACK, 4, 2, PLANARCONFIG_CONTIG, 0, true)) == 0, "tiled 4-bit grey with 2 samples, contiguous");
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_MINISBLACK, 4, 2, PLANARCONFIG_SEPARATE, 0)) == 0, "4-bit grey with 2 samples, separate planes");

	// CMYK with separate planes: the 8-bit DIB has less bytes than channels, so the channel width is 0 and the loader does not advance;
	// 3 x half float are loaded as RGBF, the channels are wider than the samples
	ok &= expect(loadTiffGuarded(makeTiff(PHOTOMETRIC_SEPARATED, 8, 2, PLANARCONFIG_SEPARATE, 0)) == 0, "CMYK with 2 separate samples");
	ok &= expect(loadTiffGuarded(makeTiff(PHOTOMETRIC_SEPARATED, 16, 3, PLANARCONFIG_SEPARATE, SAMPLEFORMAT_IEEEFP)) == 0, "3 x half float, CMYK, separate planes");

	// 32-bit samples with 4 channels are only supported for RGBA (and grey / LogLuv), not for CMYK
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_RGB, 32, 4, PLANARCONFIG_CONTIG, 0)) == 32, "RGBA with 32-bit samples");
	ok &= expect(loadTiff(makeTiff(PHOTOMETRIC_SEPARATED, 32, 4, PLANARCONFIG_CONTIG, 0)) == 0, "CMYK with 32-bit samples");

	printf(ok ? "testTIFFMalformed: OK\n" : "testTIFFMalformed: FAILED\n");
	return ok;
}

#endif // FREEIMAGE_WITH_LIBTIFF
