// ==========================================================
// FreeImage 3 Test Script
//
// PCX: decoding of planar images
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

#if FREEIMAGE_WITH_PLUGIN_PCX

#include <vector>
#include <cstdint>

// ----------------------------------------------------------

/**
Build a PCX file in memory (version 5, RLE encoded, 16 color header palette, no 256 color palette).
The data of each plane line is given by getByte(row, plane, byte_index).
*/
template <typename Func>
static std::vector<uint8_t> makePCX(int width, int height, int bitsPerPlane, int planes, int bytesPerLine, Func getByte) {
	std::vector<uint8_t> file(128, 0);
	file[0] = 10;
	file[1] = 5;
	file[2] = 1;
	file[3] = (uint8_t)bitsPerPlane;
	file[8] = (uint8_t)((width - 1) & 0xFF);
	file[9] = (uint8_t)((width - 1) >> 8);
	file[10] = (uint8_t)((height - 1) & 0xFF);
	file[11] = (uint8_t)((height - 1) >> 8);
	file[12] = 72;
	file[14] = 72;
	for (int i = 0; i < 16; i++) {
		file[16 + 3 * i] = (uint8_t)(i * 16);
		file[17 + 3 * i] = (uint8_t)(255 - i * 16);
		file[18 + 3 * i] = (uint8_t)(i * 8);
	}
	file[65] = (uint8_t)planes;
	file[66] = (uint8_t)(bytesPerLine & 0xFF);
	file[67] = (uint8_t)(bytesPerLine >> 8);
	file[68] = 1;
	for (int y = 0; y < height; y++) {
		for (int p = 0; p < planes; p++) {
			for (int i = 0; i < bytesPerLine; i++) {
				const uint8_t b = getByte(y, p, i);
				if (b >= 0xC0) {
					file.push_back(0xC1);
				}
				file.push_back(b);
			}
		}
	}
	return file;
}

static FIBITMAP* loadPcxFromBuffer(std::vector<uint8_t>& file) {
	FIMEMORY *hmem = FreeImage_OpenMemory(file.data(), (uint32_t)file.size());
	FIBITMAP *dib = FreeImage_LoadFromMemory(FIF_PCX, hmem, 0);
	FreeImage_CloseMemory(hmem);
	return dib;
}

static bool expect(bool condition, const char *what) {
	if (!condition) {
		printf("testPCX: FAILED: %s\n", what);
	}
	return condition;
}

// ----------------------------------------------------------

bool testPCX() {
	printf("testPCX ...\n");
	bool ok = true;

	// 16 colors (4 planes of 1 bit), odd width: the last pixel must not be lost
	{
		const int width = 5, height = 2;
		auto pixel = [](int y, int x) { return (uint8_t)((x * 3 + y + 1) & 15); };
		auto getByte = [&](int y, int plane, int i) {
			uint8_t b = 0;
			for (int bit = 0; bit < 8; bit++) {
				const int x = i * 8 + bit;
				if (x < width && (pixel(y, x) & (1 << plane))) {
					b |= (uint8_t)(0x80 >> bit);
				}
			}
			return b;
		};
		std::vector<uint8_t> file = makePCX(width, height, 1, 4, 2, getByte);
		FIBITMAP *dib = loadPcxFromBuffer(file);
		ok &= expect(dib != nullptr, "4 planes: loaded");
		if (dib) {
			ok &= expect(FreeImage_GetWidth(dib) == (unsigned)width && FreeImage_GetBPP(dib) == 4, "4 planes: dimensions");
			bool same = true;
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					uint8_t index = 255;
					// the first row of the file is the top row of the image
					FreeImage_GetPixelIndex(dib, (unsigned)x, (unsigned)(height - 1 - y), &index);
					same &= (index == pixel(y, x));
				}
			}
			ok &= expect(same, "4 planes: all pixels, including the last column");
			FreeImage_Unload(dib);
		}
	}

	// 24 bits (3 planes of 8 bit)
	for (int bytesPerLine = 8; bytesPerLine >= 4; bytesPerLine -= 4) {
		// 8: regular. 4: smaller than the width, the missing pixels are 0 and nothing is read behind the line
		const int width = 8, height = 2;
		auto getByte = [](int y, int plane, int i) { return (uint8_t)(10 + 40 * plane + 5 * i + y); };
		std::vector<uint8_t> file = makePCX(width, height, 8, 3, bytesPerLine, getByte);
		FIBITMAP *dib = loadPcxFromBuffer(file);
		ok &= expect(dib != nullptr, "24 bit: loaded");
		if (dib) {
			ok &= expect(FreeImage_GetWidth(dib) == (unsigned)width && FreeImage_GetBPP(dib) == 24, "24 bit: dimensions");
			bool same = true;
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					FIRGBA8 c = {};
					FreeImage_GetPixelColor(dib, (unsigned)x, (unsigned)(height - 1 - y), &c);
					const bool present = x < bytesPerLine;
					same &= (c.red == (present ? getByte(y, 0, x) : 0));
					same &= (c.green == (present ? getByte(y, 1, x) : 0));
					same &= (c.blue == (present ? getByte(y, 2, x) : 0));
				}
			}
			ok &= expect(same, bytesPerLine == 8 ? "24 bit: pixels" : "24 bit: pixels with short bytes per line");
			FreeImage_Unload(dib);
		}
	}

	// 4 planes with a short bytes per line: the rest of the row is 0
	{
		const int width = 24, height = 2;
		auto getByte = [](int y, int plane, int i) { return (uint8_t)(0xFF); };
		std::vector<uint8_t> file = makePCX(width, height, 1, 4, 2, getByte);
		FIBITMAP *dib = loadPcxFromBuffer(file);
		ok &= expect(dib != nullptr, "4 planes, short line: loaded");
		if (dib) {
			uint8_t first = 0, last = 255;
			FreeImage_GetPixelIndex(dib, 15, 0, &first);
			FreeImage_GetPixelIndex(dib, 23, 0, &last);
			ok &= expect(first == 15 && last == 0, "4 planes, short line: pixels");
			FreeImage_Unload(dib);
		}
	}

	// 256 colors (1 plane of 8 bit, palette at the end of the file), even and odd width
	for (int width = 6; width >= 5; width--) {
		const int height = 3, bytesPerLine = 6; // bytes per line is always even
		auto getByte = [&](int y, int plane, int i) { return (uint8_t)(i < width ? 20 * y + 7 * i + 1 : 0); };
		std::vector<uint8_t> file = makePCX(width, height, 8, 1, bytesPerLine, getByte);
		file.push_back(0x0C);
		for (int i = 0; i < 256; i++) {
			file.push_back((uint8_t)i);
			file.push_back((uint8_t)(255 - i));
			file.push_back((uint8_t)(i / 2));
		}
		FIBITMAP *dib = loadPcxFromBuffer(file);
		ok &= expect(dib != nullptr, "8 bit: loaded");
		if (dib) {
			ok &= expect(FreeImage_GetWidth(dib) == (unsigned)width && FreeImage_GetHeight(dib) == (unsigned)height && FreeImage_GetBPP(dib) == 8, "8 bit: dimensions");
			const FIRGBA8 *pal = FreeImage_GetPalette(dib);
			bool same = (pal != nullptr);
			for (int i = 0; same && i < 256; i++) {
				same = (pal[i].red == i && pal[i].green == 255 - i && pal[i].blue == i / 2);
			}
			ok &= expect(same, "8 bit: palette");
			same = true;
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					uint8_t index = 255;
					FreeImage_GetPixelIndex(dib, (unsigned)x, (unsigned)(height - 1 - y), &index);
					same &= (index == getByte(y, 0, x));
				}
			}
			ok &= expect(same, width % 2 ? "8 bit, odd width: pixels" : "8 bit: pixels");
			FreeImage_Unload(dib);
		}
	}

	// A file which ends in the middle of the pixel data: it is either rejected or
	// loaded with the expected size, but it must not be read beyond its end
	{
		auto getByte = [](int y, int plane, int i) { return (uint8_t)(i + 1); };
		std::vector<uint8_t> file = makePCX(8, 4, 8, 3, 8, getByte);
		file.resize(128 + 40);
		FIBITMAP *dib = loadPcxFromBuffer(file);
		ok &= expect(dib == nullptr || (FreeImage_GetWidth(dib) == 8 && FreeImage_GetHeight(dib) == 4), "truncated file: rejected or loaded with the expected size");
		if (dib) {
			FreeImage_Unload(dib);
		}
	}

	// Invalid headers must be rejected
	{
		auto getByte = [](int y, int plane, int i) { return (uint8_t)(i); };
		std::vector<uint8_t> file = makePCX(8, 2, 8, 3, 8, getByte);
		// left >= right
		file[4] = 9;
		FIBITMAP *dib = loadPcxFromBuffer(file);
		ok &= expect(dib == nullptr, "empty window is rejected");
		if (dib) {
			FreeImage_Unload(dib);
		}

		// unsupported combination of planes and bits
		file = makePCX(8, 2, 1, 2, 2, getByte);
		dib = loadPcxFromBuffer(file);
		ok &= expect(dib == nullptr, "unsupported number of planes is rejected");
		if (dib) {
			FreeImage_Unload(dib);
		}
	}

	printf(ok ? "testPCX: OK\n" : "testPCX: FAILED\n");
	return ok;
}

#endif // FREEIMAGE_WITH_PLUGIN_PCX
