// ==========================================================
// FreeImage 3 Test Script
//
// DDS: uncompressed RGB images
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

static void push32(std::vector<uint8_t>& v, unsigned value) {
	for (int i = 0; i < 4; i++) {
		v.push_back((uint8_t)((value >> (8 * i)) & 0xFF));
	}
}

/**
Build an uncompressed DDS file (DDPF_RGB) with the given bit count and masks
*/
static std::vector<uint8_t> makeRGBDds(unsigned width, unsigned height, unsigned bit_count, unsigned rmask, unsigned gmask, unsigned bmask, const std::vector<uint8_t>& data) {
	std::vector<uint8_t> file = { 'D', 'D', 'S', ' ' };
	push32(file, 124);			// dwSize
	push32(file, 0x1007);		// DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT
	push32(file, height);
	push32(file, width);
	push32(file, 0);			// pitch
	push32(file, 0);			// depth
	push32(file, 0);			// mip map count
	for (int i = 0; i < 11; i++) {
		push32(file, 0);		// reserved
	}
	push32(file, 32);			// pixel format: size
	push32(file, 0x40);			// DDPF_RGB
	push32(file, 0);			// four cc
	push32(file, bit_count);
	push32(file, rmask);
	push32(file, gmask);
	push32(file, bmask);
	push32(file, 0);			// alpha mask
	push32(file, 0x1000);		// DDSCAPS_TEXTURE
	push32(file, 0);
	push32(file, 0);
	push32(file, 0);
	push32(file, 0);			// reserved 2
	file.insert(file.end(), data.begin(), data.end());
	return file;
}

static FIBITMAP* loadDds(std::vector<uint8_t>& file) {
	FIMEMORY *hmem = FreeImage_OpenMemory(file.data(), (uint32_t)file.size());
	FIBITMAP *dib = FreeImage_LoadFromMemory(FIF_DDS, hmem, 0);
	FreeImage_CloseMemory(hmem);
	return dib;
}

static bool expect(bool condition, const char *what) {
	if (!condition) {
		printf("testDDS: FAILED: %s\n", what);
	}
	return condition;
}

// ----------------------------------------------------------

bool testDDSRGB() {
	printf("testDDSRGB ...\n");
	bool ok = true;

	// 24-bit 2x2, the file lines are 6 bytes
	{
		std::vector<uint8_t> data = { 10, 11, 12, 20, 21, 22, 30, 31, 32, 40, 41, 42 };	// B, G, R of each pixel
		std::vector<uint8_t> file = makeRGBDds(2, 2, 24, 0xFF0000, 0xFF00, 0xFF, data);
		FIBITMAP *dib = loadDds(file);
		ok &= expect(dib != nullptr && FreeImage_GetBPP(dib) == 24 && FreeImage_GetWidth(dib) == 2 && FreeImage_GetHeight(dib) == 2, "24-bit DDS is loaded");
		if (dib) {
			FIRGBA8 color;
			// the first row of the file is the top row of the image
			ok &= expect(FreeImage_GetPixelColor(dib, 1, 1, &color) && color.blue == 20 && color.green == 21 && color.red == 22, "24-bit DDS pixel");
			FreeImage_Unload(dib);
		}
	}

	// 16-bit R5G6B5 2x2: converted to a 24-bit dib, the file lines are 4 bytes. Pixels 0xF800 (the channel order of the result is not checked)
	{
		std::vector<uint8_t> data = { 0x00, 0xF8, 0x00, 0xF8, 0x00, 0xF8, 0x00, 0xF8 };
		std::vector<uint8_t> file = makeRGBDds(2, 2, 16, 0xF800, 0x07E0, 0x001F, data);
		FIBITMAP *dib = loadDds(file);
		ok &= expect(dib != nullptr && FreeImage_GetBPP(dib) == 24, "16-bit DDS is loaded as a 24-bit image");
		if (dib) {
			FIRGBA8 color;
			ok &= expect(FreeImage_GetPixelColor(dib, 1, 1, &color) && color.green == 0 && (color.red == 0) != (color.blue == 0), "16-bit DDS pixel (one outer channel set, green clear)");
			FreeImage_Unload(dib);
		}
	}

	// a bit count which is not a bit depth of an image: the file lines would be larger than the lines of the
	// 8-bit image to which FreeImage_Allocate falls back
	const unsigned bad[] = { 12, 20, 40 };
	for (unsigned bit_count : bad) {
		std::vector<uint8_t> data(8 * 16, 0x55);
		std::vector<uint8_t> file = makeRGBDds(8, 2, bit_count, 0xFF0000, 0xFF00, 0xFF, data);
		FIBITMAP *dib = loadDds(file);
		ok &= expect(dib == nullptr, "DDS with an unsupported bit count is rejected");
		if (dib) {
			FreeImage_Unload(dib);
		}
	}

	printf(ok ? "testDDSRGB: OK\n" : "testDDSRGB: FAILED\n");
	return ok;
}
