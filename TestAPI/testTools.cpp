// ==========================================================
// FreeImage 3 Test Script
//
// Design and implementation by
// - Hervé Drolon (drolon@infonie.fr)
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
#include <cmath>
#include <memory>


// ----------------------------------------------------------

/** Create a Zone Plate test pattern.

  The circular zone plate has zero horizontal and vertical frequencies at the center. 
  Horizontal frequencies increase as you move horizontally, and vertical frequencies increase as you move vertically.
  These patterns are useful to:
  <ul>
  <li> evaluate image compression 
  <li> evaluate filter properties 
  <li> evaluate the quality of resampling algorithms 
  <li> adjust gamma correction - at the proper gamma setting, the møires should be minimized 
  </ul>
  Reference: 
  [1] Ken Turkowski, Open Source Repository. [Online] http://www.worldserver.com/turk/opensource/
*/
FIBITMAP* createZonePlateImage(unsigned width, unsigned height, int scale) {
	const double PI = 3.1415926535898;
	uint8_t sinTab[256];

	FIBITMAP *dst;
	int i, j, x, y;
	int cX, cY, d;

	// allocate a 8-bit dib
	dst = FreeImage_Allocate(width, height, 8);
	if(!dst)
		return NULL;

	// build a greyscale palette
	FIRGBA8 *pal = FreeImage_GetPalette(dst);
	for(i = 0; i < 256; i++) {
		pal[i].red	= (uint8_t)i;
		pal[i].green = (uint8_t)i;
		pal[i].blue	= (uint8_t)i;
	}

	// build the sinus table
	for(i = 0; i < 256; i++) {
		sinTab[i] = (uint8_t)(127.5 * sin(PI * (i - 127.5) / 127.5) + 127.5);
	}

	cX = width / 2;
	cY = height / 2;
	
	// create a zone plate
	for(i = height, y = -cY; i--; y++) {
		uint8_t *dst_bits = FreeImage_GetScanLine(dst, i);
		for (j = width, x = -cX; j--; x++) {
			d = ((x * x + y * y) * scale) >> 8;
			dst_bits[j] = sinTab[d & 0xFF];
		}
	}

	return dst;
}


void testFindMinMax()
{
	{
		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp1(FreeImage_AllocateT(FIT_RGBF, 4, 4, 3 * sizeof(float)), &::FreeImage_Unload);
		assert(bmp1 != nullptr);
		assert(FreeImage_GetColorType(bmp1.get()) == FIC_RGB);
		for (unsigned y = 0; y < FreeImage_GetHeight(bmp1.get()); ++y) {
			const auto line = reinterpret_cast<FIRGBF*>(FreeImage_GetScanLine(bmp1.get(), y));
			for (unsigned x = 0; x < FreeImage_GetWidth(bmp1.get()); ++x) {
				if (y == 1 && x == 2) {
					line[x].red = line[x].green = line[x].blue = 0.8f;
				}
				else if (y == 2 && x == 3) {
					line[x].red = line[x].green = line[x].blue = 0.1f;
				}
				else if (y == 0 && x == 0) {
					line[x].red = line[x].green = line[x].blue = static_cast<float>(std::nan("1"));
				}
				else {
					line[x].red   = 0.50;
					line[x].green = 0.50;
					line[x].blue  = 0.50;
				}
			}
		}

		FIRGBF* minPtr = nullptr;
		FIRGBF* maxPtr = nullptr;
		double minBrightness{};
		double maxBrightness{};

		auto success = FreeImage_FindMinMax(bmp1.get(), &minBrightness, &maxBrightness, reinterpret_cast<void**>(&minPtr), reinterpret_cast<void**>(&maxPtr));
		assert(success);
		assert(std::abs(maxBrightness - 0.8) < 1e-6);
		assert(std::abs(minBrightness - 0.1) < 1e-6);

		const auto stride = FreeImage_GetPitch(bmp1.get());
		assert(reinterpret_cast<uint8_t*>(minPtr) == FreeImage_GetBits(bmp1.get()) + 2 * stride + 3 * sizeof(FIRGBF));
		assert(reinterpret_cast<uint8_t*>(maxPtr) == FreeImage_GetBits(bmp1.get()) + 1 * stride + 2 * sizeof(FIRGBF));
	}

	{
		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp1(FreeImage_Allocate(4, 4, 32), &::FreeImage_Unload);
		auto icc = FreeImage_CreateICCProfile(bmp1.get(), nullptr, 0);
		icc->flags = FIICC_COLOR_IS_YUV;

		assert(bmp1 != nullptr);
		assert(FreeImage_GetColorType(bmp1.get()) == FIC_YUV);
		for (unsigned y = 0; y < FreeImage_GetHeight(bmp1.get()); ++y) {
			const auto line = reinterpret_cast<FIRGBA8*>(FreeImage_GetScanLine(bmp1.get(), y));
			for (unsigned x = 0; x < FreeImage_GetWidth(bmp1.get()); ++x) {
				if (y == 3 && x == 1) {
					line[x].red   = 125;
					line[x].green = 5;
					line[x].blue  = 200;
				}
				else if (y == 2 && x == 2) {
					line[x].red   = 10;
					line[x].green = 0;
					line[x].blue  = 180;
				}
				else {
					line[x].red   = 100;
					line[x].green = 255;
					line[x].blue  = 0;
				}
			}
		}

		FIRGBA8* minPtr = nullptr;
		FIRGBA8* maxPtr = nullptr;
		double minBrightness{};
		double maxBrightness{};

		auto success = FreeImage_FindMinMax(bmp1.get(), &minBrightness, &maxBrightness, reinterpret_cast<void**>(&minPtr), reinterpret_cast<void**>(&maxPtr));
		assert(success);
		assert(std::abs(maxBrightness - 125.0) < 1e-6);
		assert(std::abs(minBrightness - 10.0) < 1e-6);

		const auto stride = FreeImage_GetPitch(bmp1.get());
		assert(reinterpret_cast<uint8_t*>(minPtr) == FreeImage_GetBits(bmp1.get()) + 2 * stride + 2 * sizeof(FIRGBA8));
		assert(reinterpret_cast<uint8_t*>(maxPtr) == FreeImage_GetBits(bmp1.get()) + 3 * stride + 1 * sizeof(FIRGBA8));
	}


	{
		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp1(FreeImage_Allocate(4, 4, 8), &::FreeImage_Unload);
		assert(bmp1 != nullptr);
		assert(FreeImage_GetColorType(bmp1.get()) == FIC_MINISBLACK);
		for (unsigned y = 0; y < FreeImage_GetHeight(bmp1.get()); ++y) {
			const auto line = reinterpret_cast<uint8_t*>(FreeImage_GetScanLine(bmp1.get(), y));
			for (unsigned x = 0; x < FreeImage_GetWidth(bmp1.get()); ++x) {
				if (y == 3 && x == 3) {
					line[x] = 101;
				}
				else if (y == 1 && x == 0) {
					line[x] = 8;
				}
				else {
					line[x] = 100;
				}
			}
		}

		uint8_t* minPtr = nullptr;
		uint8_t* maxPtr = nullptr;
		double minBrightness{};
		double maxBrightness{};

		auto success = FreeImage_FindMinMax(bmp1.get(), &minBrightness, &maxBrightness, reinterpret_cast<void**>(&minPtr), reinterpret_cast<void**>(&maxPtr));
		assert(success);
		assert(std::abs(maxBrightness - 101.0) < 1e-6);
		assert(std::abs(minBrightness - 8.0) < 1e-6);

		const auto stride = FreeImage_GetPitch(bmp1.get());
		assert(reinterpret_cast<uint8_t*>(minPtr) == FreeImage_GetBits(bmp1.get()) + 1 * stride + 0 * sizeof(uint8_t));
		assert(reinterpret_cast<uint8_t*>(maxPtr) == FreeImage_GetBits(bmp1.get()) + 3 * stride + 3 * sizeof(uint8_t));
	}

	{
		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp(FreeImage_AllocateT(FIT_RGBAF, 4, 4), &::FreeImage_Unload);

		FIRGBAF* ptr = reinterpret_cast<FIRGBAF*>(FreeImage_GetScanLine(bmp.get(), 0));
		ptr[0].red = 1.0f;
		ptr[0].green = 0.4f;
		ptr[0].blue = 0.1f;
		ptr[0].alpha = 0.0f;

		ptr[1].red = 0.9f;
		ptr[1].green = 0.8f;
		ptr[1].blue = 0.5f;
		ptr[1].alpha = 0.1f;

		ptr[2].red = 0.9f;
		ptr[2].green = 0.0f;
		ptr[2].blue = 4.1f;
		ptr[2].alpha = 0.0f;

		ptr[3].red = 0.2f;
		ptr[3].green = 0.3f;
		ptr[3].blue = 0.6f;
		ptr[3].alpha = 0.7f;

		ptr = reinterpret_cast<FIRGBAF*>(FreeImage_GetScanLine(bmp.get(), 1));

		ptr[0].red = 1.0f;
		ptr[0].green = 0.5f;
		ptr[0].blue = 0.1f;
		ptr[0].alpha = 2.0f;

		ptr[1].red = 0.3f;
		ptr[1].green = 0.8f;
		ptr[1].blue = 0.4f;
		ptr[1].alpha = 0.5f;

		ptr[2].red = 0.9f;
		ptr[2].green = 0.3f;
		ptr[2].blue = 6.1f;
		ptr[2].alpha = 0.0f;

		ptr[3].red = 0.2f;
		ptr[3].green = 9.3f;
		ptr[3].blue = 0.6f;
		ptr[3].alpha = 0.7f;

		ptr = reinterpret_cast<FIRGBAF*>(FreeImage_GetScanLine(bmp.get(), 2));

		ptr[0].red = 11.0f;
		ptr[0].green = 5.4f;
		ptr[0].blue = 0.4f;
		ptr[0].alpha = 3.0f;

		ptr[1].red = 0.5f;
		ptr[1].green = 1.4f;
		ptr[1].blue = 5.4f;
		ptr[1].alpha = 4.5f;

		ptr[2].red = 0.1f;
		ptr[2].green = 0.4f;
		ptr[2].blue = 3.1f;
		ptr[2].alpha = 1.0f;

		ptr[3].red = 0.5f;
		ptr[3].green = 8.4f;
		ptr[3].blue = 1.6f;
		ptr[3].alpha = 2.7f;

		ptr = reinterpret_cast<FIRGBAF*>(FreeImage_GetScanLine(bmp.get(), 3));

		ptr[0].red = 1.4f;
		ptr[0].green = 0.4f;
		ptr[0].blue = 9.2f;
		ptr[0].alpha = 2.3f;

		ptr[1].red = 3.3f;
		ptr[1].green = 1.8f;
		ptr[1].blue = 3.4f;
		ptr[1].alpha = 1.5f;

		ptr[2].red = 0.4f;
		ptr[2].green = 5.3f;
		ptr[2].blue = 6.3f;
		ptr[2].alpha = 0.0f;

		ptr[3].red = 0.3f;
		ptr[3].green = 7.3f;
		ptr[3].blue = 0.3f;
		ptr[3].alpha = 45.7f;

		FIRGBAF minVal, maxVal;
		bool success = FreeImage_FindMinMaxValue(bmp.get(), &minVal, &maxVal);
		assert(success);

		assert(minVal.red == 0.1f);
		assert(minVal.green == 0.0f);
		assert(minVal.blue == 0.1f);
		assert(minVal.alpha == 0.0f);

		assert(maxVal.red == 11.0f);
		assert(maxVal.green == 9.3f);
		assert(maxVal.blue == 9.2f);
		assert(maxVal.alpha == 45.7f);
	}
}

