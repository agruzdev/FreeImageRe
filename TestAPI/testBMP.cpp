// ==========================================================
// FreeImage 3 Test Script
//
// BMP: RLE8 decoding
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

#if TEST_WITH_BMP

#include <vector>
#include <cstdint>

// ----------------------------------------------------------

static void push16(std::vector<uint8_t>& v, unsigned value) {
	v.push_back((uint8_t)(value & 0xFF));
	v.push_back((uint8_t)(value >> 8));
}

static void push32(std::vector<uint8_t>& v, unsigned value) {
	push16(v, value & 0xFFFF);
	push16(v, value >> 16);
}

/**
Build a bottom-up 8-bit BMP file with the RLE8 compressed pixel data.
The palette has 256 entries, entry i is (i, i, i).
*/
static std::vector<uint8_t> makeRLE8Bmp(int width, int height, const std::vector<uint8_t>& rle) {
	const unsigned headers = 14 + 40 + 256 * 4;
	std::vector<uint8_t> file;
	file.push_back('B');
	file.push_back('M');
	push32(file, (unsigned)(headers + rle.size()));
	push32(file, 0);
	push32(file, headers);
	push32(file, 40);			// BITMAPINFOHEADER
	push32(file, (unsigned)width);
	push32(file, (unsigned)height);
	push16(file, 1);			// planes
	push16(file, 8);			// bit count
	push32(file, 1);			// BI_RLE8
	push32(file, (unsigned)rle.size());
	push32(file, 2835);
	push32(file, 2835);
	push32(file, 256);
	push32(file, 0);
	for (unsigned i = 0; i < 256; i++) {
		file.push_back((uint8_t)i);
		file.push_back((uint8_t)i);
		file.push_back((uint8_t)i);
		file.push_back(0);
	}
	file.insert(file.end(), rle.begin(), rle.end());
	return file;
}

static FIBITMAP* loadBmpFromBuffer(std::vector<uint8_t>& file) {
	FIMEMORY *hmem = FreeImage_OpenMemory(file.data(), (uint32_t)file.size());
	FIBITMAP *dib = FreeImage_LoadFromMemory(FIF_BMP, hmem, 0);
	FreeImage_CloseMemory(hmem);
	return dib;
}

static bool checkRow(FIBITMAP *dib, int scanline, const uint8_t *expected, int width) {
	for (int x = 0; x < width; x++) {
		uint8_t index = 0;
		if (!FreeImage_GetPixelIndex(dib, (unsigned)x, (unsigned)scanline, &index) || index != expected[x]) {
			return false;
		}
	}
	return true;
}

static bool expect(bool condition, const char *what) {
	if (!condition) {
		printf("testBMPRLE8: FAILED: %s\n", what);
	}
	return condition;
}

// ----------------------------------------------------------

bool testBMPRLE8() {
	printf("testBMPRLE8 ...\n");
	bool ok = true;

	// A valid image 6x2: scanline 0 is an encoded run, scanline 1 is an absolute run followed by an encoded run
	{
		const uint8_t rle[] = {
			6, 7,					// 6 x index 7
			0, 0,					// end of line
			0, 3, 1, 2, 3, 0,		// absolute run of 3 pixels (padded to an even number of bytes)
			3, 9,					// 3 x index 9
			0, 0,					// end of line
			0, 1					// end of bitmap
		};
		const uint8_t row0[6] = { 7, 7, 7, 7, 7, 7 };
		const uint8_t row1[6] = { 1, 2, 3, 9, 9, 9 };
		std::vector<uint8_t> file = makeRLE8Bmp(6, 2, std::vector<uint8_t>(rle, rle + sizeof(rle)));
		FIBITMAP *dib = loadBmpFromBuffer(file);
		ok &= expect(dib != nullptr, "valid RLE8 is loaded");
		if (dib) {
			ok &= expect(FreeImage_GetWidth(dib) == 6 && FreeImage_GetHeight(dib) == 2 && FreeImage_GetBPP(dib) == 8, "valid RLE8 dimensions");
			ok &= expect(checkRow(dib, 0, row0, 6), "valid RLE8 row 0");
			ok &= expect(checkRow(dib, 1, row1, 6), "valid RLE8 row 1");
			FreeImage_Unload(dib);
		}
	}

	// Runs which are longer than the scanline: the part which does not fit is skipped, the following data is decoded normally
	{
		const uint8_t rle[] = {
			4, 5,								// 4 x index 5, the scanline is full
			0, 6, 11, 12, 13, 14, 15, 16,		// absolute run of 6 pixels, nothing fits
			9, 6,								// encoded run, nothing fits
			0, 0,								// end of line
			0, 6, 21, 22, 23, 24, 25, 26,		// absolute run of 6 pixels, only 4 fit
			0, 0,								// end of line
			0, 1
		};
		const uint8_t row0[4] = { 5, 5, 5, 5 };
		const uint8_t row1[4] = { 21, 22, 23, 24 };
		std::vector<uint8_t> file = makeRLE8Bmp(4, 2, std::vector<uint8_t>(rle, rle + sizeof(rle)));
		FIBITMAP *dib = loadBmpFromBuffer(file);
		ok &= expect(dib != nullptr, "over-long runs are loaded");
		if (dib) {
			ok &= expect(checkRow(dib, 0, row0, 4), "over-long runs row 0");
			ok &= expect(checkRow(dib, 1, row1, 4), "over-long runs row 1");
			FreeImage_Unload(dib);
		}
	}

	// Delta commands leaving the image: the position is kept inside of the image
	{
		const uint8_t rle[] = {
			2, 3,					// 2 x index 3
			0, 2, 200, 1,			// delta: far to the right, one line up
			0, 6, 1, 2, 3, 4, 5, 6,	// absolute run, outside of the line
			0, 2, 0, 200,			// delta: far above the image
			3, 4,					// outside of the image, ends the decoding
			0, 1
		};
		std::vector<uint8_t> file = makeRLE8Bmp(4, 2, std::vector<uint8_t>(rle, rle + sizeof(rle)));
		FIBITMAP *dib = loadBmpFromBuffer(file);
		ok &= expect(dib != nullptr, "delta outside of the image is handled");
		if (dib) {
			const uint8_t row0[2] = { 3, 3 };
			ok &= expect(checkRow(dib, 0, row0, 2), "delta row 0");
			FreeImage_Unload(dib);
		}
	}

	// Truncated data: must be rejected
	{
		const uint8_t rle[] = {
			4, 5,
			0, 0,
			0, 6, 1, 2, 3			// the absolute run is cut off
		};
		std::vector<uint8_t> file = makeRLE8Bmp(4, 2, std::vector<uint8_t>(rle, rle + sizeof(rle)));
		FIBITMAP *dib = loadBmpFromBuffer(file);
		ok &= expect(dib == nullptr, "truncated RLE8 is rejected");
		if (dib) {
			FreeImage_Unload(dib);
		}
	}

	printf(ok ? "testBMPRLE8: OK\n" : "testBMPRLE8: FAILED\n");
	return ok;
}

/**
Build a BMP file with a BITMAPINFOHEADER (40 bytes) and a 256 entry palette.
The pixel data is given as is, bit_count and compression are arbitrary.
*/
static std::vector<uint8_t> makeBmp(int width, int height, unsigned bit_count, unsigned compression, const std::vector<uint8_t>& data) {
	const unsigned headers = 14 + 40 + 256 * 4;
	std::vector<uint8_t> file;
	file.push_back('B');
	file.push_back('M');
	push32(file, (unsigned)(headers + data.size()));
	push32(file, 0);
	push32(file, headers);
	push32(file, 40);			// BITMAPINFOHEADER
	push32(file, (unsigned)width);
	push32(file, (unsigned)height);
	push16(file, 1);			// planes
	push16(file, bit_count);
	push32(file, compression);
	push32(file, (unsigned)data.size());
	push32(file, 2835);
	push32(file, 2835);
	push32(file, 0);
	push32(file, 0);
	for (unsigned i = 0; i < 256; i++) {
		file.push_back((uint8_t)i);
		file.push_back((uint8_t)i);
		file.push_back((uint8_t)i);
		file.push_back(0);
	}
	file.insert(file.end(), data.begin(), data.end());
	return file;
}

/**
Runs of a width of 64 pixels, 4 lines: much more than a scanline of a 1- or 4-bit image can hold
*/
static std::vector<uint8_t> makeLongRuns() {
	std::vector<uint8_t> rle;
	for (int y = 0; y < 4; y++) {
		rle.push_back(64);
		rle.push_back(0x11);
		rle.push_back(0);
		rle.push_back(0);
	}
	rle.push_back(0);
	rle.push_back(1);
	return rle;
}

// ----------------------------------------------------------

bool testBMPRLEBitDepth() {
	printf("testBMPRLEBitDepth ...\n");
	bool ok = true;

	// RLE8 and RLE4 data in a file whose bit depth is not 8 or 4: the decoders write more
	// pixels per scanline than the allocated image can hold, so the file must be rejected
	const struct { unsigned bit_count; unsigned compression; const char *what; } bad[] = {
		{ 1, 1, "RLE8 with 1-bit depth is rejected" },
		{ 4, 1, "RLE8 with 4-bit depth is rejected" },
		{ 1, 2, "RLE4 with 1-bit depth is rejected" },
		{ 8, 2, "RLE4 with 8-bit depth is rejected" },
	};
	for (const auto& t : bad) {
		std::vector<uint8_t> file = makeBmp(64, 4, t.bit_count, t.compression, makeLongRuns());
		FIBITMAP *dib = loadBmpFromBuffer(file);
		ok &= expect(dib == nullptr, t.what);
		if (dib) {
			FreeImage_Unload(dib);
		}
	}

	// the matching combinations are still decoded
	{
		std::vector<uint8_t> file = makeBmp(64, 4, 8, 1, makeLongRuns());
		FIBITMAP *dib = loadBmpFromBuffer(file);
		ok &= expect(dib != nullptr && FreeImage_GetBPP(dib) == 8, "RLE8 with 8-bit depth is loaded");
		if (dib) {
			FreeImage_Unload(dib);
		}
	}
	{
		std::vector<uint8_t> file = makeBmp(64, 4, 4, 2, makeLongRuns());
		FIBITMAP *dib = loadBmpFromBuffer(file);
		ok &= expect(dib != nullptr && FreeImage_GetBPP(dib) == 4, "RLE4 with 4-bit depth is loaded");
		if (dib) {
			FreeImage_Unload(dib);
		}
	}

	printf(ok ? "testBMPRLEBitDepth: OK\n" : "testBMPRLEBitDepth: FAILED\n");
	return ok;
}

bool testBMPTruncatedHeaders() {
	printf("testBMPTruncatedHeaders ...\n");
	bool ok = true;

	std::vector<uint8_t> pixels(4 * 4, 3);
	const std::vector<uint8_t> valid = makeBmp(4, 4, 8, 0, pixels);
	{
		std::vector<uint8_t> file = valid;
		FIBITMAP *dib = loadBmpFromBuffer(file);
		ok &= expect(dib != nullptr, "unmodified BMP is loaded");
		if (dib) {
			FreeImage_Unload(dib);
		}
	}

	// the file header is 14 bytes, the info header 40 bytes: every file which ends in one
	// of the headers must be rejected
	const size_t cuts[] = { 0, 1, 2, 13, 14, 15, 18, 30, 53 };
	for (size_t cut : cuts) {
		std::vector<uint8_t> file(valid.begin(), valid.begin() + cut);
		FIBITMAP *dib = nullptr;
		if (!file.empty()) {
			dib = loadBmpFromBuffer(file);
		}
		char what[64];
		snprintf(what, sizeof(what), "BMP cut after %u bytes is rejected", (unsigned)cut);
		ok &= expect(dib == nullptr, what);
		if (dib) {
			FreeImage_Unload(dib);
		}
	}

	// OS/2 1.x header (12 bytes) cut inside of the header
	{
		std::vector<uint8_t> file;
		file.push_back('B');
		file.push_back('M');
		push32(file, 14 + 12);
		push32(file, 0);
		push32(file, 14 + 12);
		push32(file, 12);			// BITMAPCOREHEADER
		push16(file, 4);			// width
		file.push_back(4);			// only the first byte of the height
		FIBITMAP *dib = loadBmpFromBuffer(file);
		ok &= expect(dib == nullptr, "OS/2 1.x BMP cut inside of the header is rejected");
		if (dib) {
			FreeImage_Unload(dib);
		}
	}

	printf(ok ? "testBMPTruncatedHeaders: OK\n" : "testBMPTruncatedHeaders: FAILED\n");
	return ok;
}

/**
Build a 4x2 bottom-up BMP with the info header of the given version (40, 108 or 124 bytes).
24 bit: BI_RGB, 32 bit: BI_BITFIELDS with the usual BGRA masks.
Pixel (x, y) in the top row has the color (10 + x, 20 + x, 30 + x), the bottom row (110 + x, 120 + x, 130 + x).
*/
static std::vector<uint8_t> makeVersionedBmp(unsigned header_size, unsigned bit_count) {
	const int width = 4, height = 2;
	const unsigned bytes = bit_count / 8;
	const unsigned pitch = (width * bytes + 3) & ~3u;
	std::vector<uint8_t> pixels(pitch * height, 0);
	for (int y = 0; y < height; y++) {
		// the file is bottom-up: the first row of the file is the bottom row
		const unsigned base = (y == 0) ? 110 : 10;
		for (int x = 0; x < width; x++) {
			uint8_t *p = &pixels[y * pitch + x * bytes];
			p[0] = (uint8_t)(base + 20 + x);	// blue
			p[1] = (uint8_t)(base + 10 + x);	// green
			p[2] = (uint8_t)(base + x);			// red
			if (bytes == 4) {
				p[3] = 255;
			}
		}
	}
	const unsigned offset = 14 + header_size;
	std::vector<uint8_t> file;
	file.push_back('B');
	file.push_back('M');
	push32(file, (unsigned)(offset + pixels.size()));
	push32(file, 0);
	push32(file, offset);
	push32(file, header_size);
	push32(file, (unsigned)width);
	push32(file, (unsigned)height);
	push16(file, 1);
	push16(file, bit_count);
	push32(file, bit_count == 32 ? 3u : 0u);	// BI_BITFIELDS or BI_RGB
	push32(file, (unsigned)pixels.size());
	push32(file, 2835);
	push32(file, 2835);
	push32(file, 0);
	push32(file, 0);
	if (header_size >= 108) {
		push32(file, 0x00FF0000);	// red mask
		push32(file, 0x0000FF00);	// green mask
		push32(file, 0x000000FF);	// blue mask
		push32(file, bit_count == 32 ? 0xFF000000u : 0u);	// alpha mask
		push32(file, 0x73524742);	// 'sRGB'
		while (file.size() < 14 + header_size) {
			file.push_back(0);		// endpoints, gamma, and the version 5 fields
		}
	}
	file.insert(file.end(), pixels.begin(), pixels.end());
	return file;
}

bool testBMPHeaderVersions() {
	printf("testBMPHeaderVersions ...\n");
	bool ok = true;

	const unsigned sizes[] = { 40, 108, 124 };
	const unsigned depths[] = { 24, 32 };
	for (unsigned header_size : sizes) {
		for (unsigned bit_count : depths) {
			char what[96];
			snprintf(what, sizeof(what), "BMP with a %u byte header, %u bit, is loaded", header_size, bit_count);
			std::vector<uint8_t> file = makeVersionedBmp(header_size, bit_count);
			FIBITMAP *dib = loadBmpFromBuffer(file);
			bool good = (dib != nullptr) && FreeImage_GetWidth(dib) == 4 && FreeImage_GetHeight(dib) == 2 && FreeImage_GetBPP(dib) == bit_count;
			if (good) {
				// scanline 0 of the dib is the bottom row of the image
				for (unsigned y = 0; y < 2; y++) {
					const unsigned base = (y == 0) ? 110 : 10;
					for (unsigned x = 0; x < 4; x++) {
						FIRGBA8 color;
						good &= FreeImage_GetPixelColor(dib, x, y, &color) != 0;
						good &= color.red == base + x && color.green == base + 10 + x && color.blue == base + 20 + x;
					}
				}
			}
			ok &= expect(good, what);
			if (dib) {
				FreeImage_Unload(dib);
			}
		}
	}

	// header fields which cannot be decoded are rejected
	{
		std::vector<uint8_t> file = makeVersionedBmp(40, 24);
		file[28] = 2;		// biBitCount = 2
		file[29] = 0;
		FIBITMAP *dib = loadBmpFromBuffer(file);
		ok &= expect(dib == nullptr, "BMP with a bit depth of 2 is rejected");
		if (dib) {
			FreeImage_Unload(dib);
		}
	}
	{
		std::vector<uint8_t> file = makeVersionedBmp(40, 24);
		file[14] = 44;		// biSize = 44, no such header
		FIBITMAP *dib = loadBmpFromBuffer(file);
		ok &= expect(dib == nullptr, "BMP with an unknown header size is rejected");
		if (dib) {
			FreeImage_Unload(dib);
		}
	}
	{
		// top-down and RLE8 compressed
		std::vector<uint8_t> rle = { 4, 1, 0, 0, 4, 2, 0, 0, 0, 1 };
		std::vector<uint8_t> file = makeBmp(4, -2, 8, 1, rle);
		FIBITMAP *dib = loadBmpFromBuffer(file);
		ok &= expect(dib == nullptr, "top-down RLE8 BMP is rejected");
		if (dib) {
			FreeImage_Unload(dib);
		}
	}

	printf(ok ? "testBMPHeaderVersions: OK\n" : "testBMPHeaderVersions: FAILED\n");
	return ok;
}

#endif // TEST_WITH_BMP
