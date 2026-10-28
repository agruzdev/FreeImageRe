// ==========================================================
// FreeImage 3 Test Script
//
// PFM: header validation
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

#include <string>
#include <vector>
#include <cstdint>
#include <cstring>

// ----------------------------------------------------------

static FIBITMAP* loadPfm(const std::string& header, const std::vector<float>& pixels, int flags) {
	std::vector<uint8_t> file(header.begin(), header.end());
	const uint8_t *p = reinterpret_cast<const uint8_t*>(pixels.data());
	file.insert(file.end(), p, p + pixels.size() * sizeof(float));
	FIMEMORY *hmem = FreeImage_OpenMemory(file.data(), (uint32_t)file.size());
	FIBITMAP *dib = FreeImage_LoadFromMemory(FIF_PFM, hmem, flags);
	FreeImage_CloseMemory(hmem);
	return dib;
}

static bool expect(bool condition, const char *what) {
	if (!condition) {
		printf("testPFM: FAILED: %s\n", what);
	}
	return condition;
}

// ----------------------------------------------------------

bool testPFM() {
	printf("testPFM ...\n");
	bool ok = true;

	// a valid 2x2 greyscale image, the scale factor is negative: little endian floats
	{
		const std::vector<float> pixels = { 1.0f, 2.0f, 3.0f, 4.0f };
		FIBITMAP *dib = loadPfm("Pf\n2 2\n-1.0\n", pixels, 0);
		ok &= expect(dib != nullptr && FreeImage_GetImageType(dib) == FIT_FLOAT && FreeImage_GetWidth(dib) == 2 && FreeImage_GetHeight(dib) == 2, "valid PFM is loaded");
		if (dib) {
			// the plugin stores the first row of the file in the last scanline of the dib
			const float *top = reinterpret_cast<const float*>(FreeImage_GetScanLine(dib, 1));
			const float *bottom = reinterpret_cast<const float*>(FreeImage_GetScanLine(dib, 0));
			ok &= expect(top[0] == 1.0f && top[1] == 2.0f && bottom[0] == 3.0f && bottom[1] == 4.0f, "valid PFM pixels");
			FreeImage_Unload(dib);
		}
	}

	// the numbers of the header may be separated by any white space, preceded by comments and by a plus sign
	{
		const char *valid[] = { "Pf\n# a comment\n2 2\n-1.0\n", "Pf\n2\t2\n-1.0\n", "Pf\n+2 +2\n-1.0\n" };
		for (const char *header : valid) {
			const std::vector<float> pixels = { 1.0f, 2.0f, 3.0f, 4.0f };
			FIBITMAP *dib = loadPfm(header, pixels, 0);
			ok &= expect(dib != nullptr && FreeImage_GetWidth(dib) == 2 && FreeImage_GetHeight(dib) == 2, "PFM with comment, tab or plus sign in the header is loaded");
			if (dib) {
				FreeImage_Unload(dib);
			}
		}
	}

	// sizes which are zero, negative, do not fit into an int (also not after wrapping around once or twice),
	// are too large for the line buffer or are not a plain number are rejected, also if only the header is requested
	const char *headers[] = {
		"PF\n0 2\n-1.0\n",
		"PF\n2 0\n-1.0\n",
		"PF\n4294967295 2\n-1.0\n",
		"PF\n2 4294967295\n-1.0\n",
		"PF\n1431655766 1\n-1.0\n",		// 3 * width does not fit into an unsigned
		"Pf\n2147483648 1\n-1.0\n",
		"PF\n4294967297 1\n-1.0\n",				// wraps around to 1
		"PF\n8589934593 1\n-1.0\n",				// wraps around twice to 1
		"PF\n2 18446744073709551617\n-1.0\n",
		"PF\n-2 2\n-1.0\n",
		"PF\n2 -2\n-1.0\n",
		"PF\n+ 2 2\n-1.0\n",
		"PF\n++2 2\n-1.0\n",
		"PF\n+-2 2\n-1.0\n",
		"PF\n2 +\n-1.0\n",
		"PF\nx2 2\n-1.0\n",
	};
	for (const char *header : headers) {
		for (int flags : { 0, (int)FIF_LOAD_NOPIXELS }) {
			FIBITMAP *dib = loadPfm(header, std::vector<float>(8, 0.5f), flags);
			ok &= expect(dib == nullptr, "PFM with an invalid size is rejected");
			if (dib) {
				FreeImage_Unload(dib);
			}
		}
	}

	printf(ok ? "testPFM: OK\n" : "testPFM: FAILED\n");
	return ok;
}
