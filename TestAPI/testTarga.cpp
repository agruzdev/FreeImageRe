// ==========================================================
// FreeImage 3 Test Script
//
// TARGA: RLE decoding
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

#include <vector>
#include <cstdint>

// ----------------------------------------------------------

static void push16(std::vector<uint8_t>& v, unsigned value) {
	v.push_back((uint8_t)(value & 0xFF));
	v.push_back((uint8_t)(value >> 8));
}

/**
Build a TARGA file with an RLE compressed image (image type 10 for true-color, 11 for black-and-white),
origin at the bottom left, followed by the given pixel data.
*/
static std::vector<uint8_t> makeRLETga(unsigned type, unsigned width, unsigned height, unsigned depth, const std::vector<uint8_t>& data) {
	std::vector<uint8_t> file;
	file.push_back(0);			// id length
	file.push_back(0);			// no color map
	file.push_back((uint8_t)type);
	push16(file, 0);			// color map: first entry
	push16(file, 0);			// color map: length
	file.push_back(0);			// color map: entry size
	push16(file, 0);			// x origin
	push16(file, 0);			// y origin
	push16(file, width);
	push16(file, height);
	file.push_back((uint8_t)depth);
	file.push_back(0);			// image descriptor
	file.insert(file.end(), data.begin(), data.end());
	return file;
}

static FIBITMAP* loadTgaFromBuffer(std::vector<uint8_t>& file) {
	FIMEMORY *hmem = FreeImage_OpenMemory(file.data(), (uint32_t)file.size());
	FIBITMAP *dib = FreeImage_LoadFromMemory(FIF_TARGA, hmem, 0);
	FreeImage_CloseMemory(hmem);
	return dib;
}

static bool expect(bool condition, const char *what) {
	if (!condition) {
		printf("testTargaRLE: FAILED: %s\n", what);
	}
	return condition;
}

// ----------------------------------------------------------

bool testTargaRLE() {
	printf("testTargaRLE ...\n");
	bool ok = true;

	// A valid 24-bit image 4x2: the bottom row is one run, the top row is a raw packet of 4 pixels
	{
		std::vector<uint8_t> data = {
			0x83, 1, 2, 3,								// run of 4 pixels: blue 1, green 2, red 3
			0x03, 10, 11, 12, 20, 21, 22, 30, 31, 32, 40, 41, 42	// 4 raw pixels
		};
		std::vector<uint8_t> file = makeRLETga(10, 4, 2, 24, data);
		FIBITMAP *dib = loadTgaFromBuffer(file);
		ok &= expect(dib != nullptr, "valid RLE image is loaded");
		if (dib) {
			ok &= expect(FreeImage_GetWidth(dib) == 4 && FreeImage_GetHeight(dib) == 2 && FreeImage_GetBPP(dib) == 24, "valid RLE image dimensions");
			bool pixels = true;
			for (unsigned x = 0; x < 4; x++) {
				FIRGBA8 bottom, top;
				pixels &= FreeImage_GetPixelColor(dib, x, 0, &bottom) != 0 && bottom.red == 3 && bottom.green == 2 && bottom.blue == 1;
				pixels &= FreeImage_GetPixelColor(dib, x, 1, &top) != 0 && top.blue == 10 + 10 * x && top.green == 11 + 10 * x && top.red == 12 + 10 * x;
			}
			ok &= expect(pixels, "valid RLE image pixels");
			FreeImage_Unload(dib);
		}
	}

	// The file ends before there is one byte for each line: the image is rejected
	{
		std::vector<uint8_t> data = { 0x83, 1, 2, 3, 0x80, 4, 5, 6, 0, 0 };
		std::vector<uint8_t> file = makeRLETga(10, 4, 64, 24, data);
		FIBITMAP *dib = loadTgaFromBuffer(file);
		ok &= expect(dib == nullptr, "RLE image with less data than lines is rejected");
		if (dib) {
			FreeImage_Unload(dib);
		}
	}

	// Only a few bytes of data per line: the block which is read from the file has to be able to hold a pixel.
	// The pixel data is short, the image may be rejected or loaded, it must not touch memory outside of the buffers.
	{
		const unsigned depths[] = { 8, 24, 32 };
		const unsigned types[] = { 11, 10, 10 };
		for (int i = 0; i < 3; i++) {
			std::vector<uint8_t> data = { 0x01, 1, 2, 3, 4, 5, 6, 7 };
			data.resize(2 + (i == 0 ? 0 : 1), 0x80);	// 2 or 3 bytes: less than one pixel for each of the 4 lines of 32 bit
			std::vector<uint8_t> file = makeRLETga(types[i], 4, 2, depths[i], data);
			FIBITMAP *dib = loadTgaFromBuffer(file);
			ok &= expect(dib == nullptr || (FreeImage_GetWidth(dib) == 4 && FreeImage_GetHeight(dib) == 2), "RLE image with data shorter than a pixel is handled");
			if (dib) {
				FreeImage_Unload(dib);
			}
		}
	}
	{
		// one byte for each line
		std::vector<uint8_t> data = { 0x80, 7, 0x80, 8, 0x80, 9, 0x80, 10 };
		std::vector<uint8_t> file = makeRLETga(11, 4, 4, 8, data);
		FIBITMAP *dib = loadTgaFromBuffer(file);
		ok &= expect(dib == nullptr || FreeImage_GetHeight(dib) == 4, "8-bit RLE image with one byte for each line is handled");
		if (dib) {
			FreeImage_Unload(dib);
		}
	}

	printf(ok ? "testTargaRLE: OK\n" : "testTargaRLE: FAILED\n");
	return ok;
}
