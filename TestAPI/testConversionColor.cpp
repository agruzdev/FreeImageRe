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
#include <memory>
#include <limits>

/**
Test FreeImage_ConvertToColor
*/
void testConvertToColor()
{
	{
		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp1(FreeImage_AllocateT(FIT_RGBF, 4, 4, 3 * sizeof(float)), &::FreeImage_Unload);
		assert(bmp1 != nullptr);
		assert(FreeImage_GetColorType(bmp1.get()) == FIC_RGB);
		for (unsigned y = 0; y < FreeImage_GetHeight(bmp1.get()); ++y) {
			const auto line = reinterpret_cast<FIRGBF*>(FreeImage_GetScanLine(bmp1.get(), y));
			for (unsigned x = 0; x < FreeImage_GetWidth(bmp1.get()); ++x) {
				line[x].red   = 0.25;
				line[x].green = 0.50;
				line[x].blue  = 0.75;
			}
		}

		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp2(FreeImage_ConvertToColor(bmp1.get(), FIC_YUV, FICPARAM_YUV_STANDARD_JPEG), &::FreeImage_Unload);
		assert(bmp2 != nullptr);
		assert(FreeImage_GetColorType(bmp2.get()) == FIC_YUV);
		for (unsigned y = 0; y < FreeImage_GetHeight(bmp2.get()); ++y) {
			const auto line = reinterpret_cast<FIRGBF*>(FreeImage_GetScanLine(bmp2.get(), y));
			for (unsigned x = 0; x < FreeImage_GetWidth(bmp2.get()); ++x) {
				assert(std::abs(line[x].red   - 0.453750) < 1e-6);
				assert(std::abs(line[x].green - 0.667184) < 1e-6);
				assert(std::abs(line[x].blue  - 0.354672) < 1e-6);
			}
		}

		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp3(FreeImage_ConvertToColor(bmp2.get(), FIC_RGB, FICPARAM_YUV_STANDARD_JPEG), &::FreeImage_Unload);
		assert(bmp3 != nullptr);
		assert(FreeImage_GetColorType(bmp3.get()) == FIC_RGB);
		for (unsigned y = 0; y < FreeImage_GetHeight(bmp3.get()); ++y) {
			const auto line = reinterpret_cast<FIRGBF*>(FreeImage_GetScanLine(bmp3.get(), y));
			for (unsigned x = 0; x < FreeImage_GetWidth(bmp3.get()); ++x) {
				assert(std::abs(line[x].red   - 0.25) < 1e-6);
				assert(std::abs(line[x].green - 0.50) < 1e-6);
				assert(std::abs(line[x].blue  - 0.75) < 1e-6);
			}
		}
	}


	{
		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp1(FreeImage_Allocate(3, 5, 32), &::FreeImage_Unload);
		assert(bmp1 != nullptr);
		assert(FreeImage_GetColorType(bmp1.get()) == FIC_RGBALPHA);
		for (unsigned y = 0; y < FreeImage_GetHeight(bmp1.get()); ++y) {
			const auto line = reinterpret_cast<FIRGBA8*>(FreeImage_GetScanLine(bmp1.get(), y));
			for (unsigned x = 0; x < FreeImage_GetWidth(bmp1.get()); ++x) {
				line[x].red   = 50;
				line[x].green = 127;
				line[x].blue  = 200;
				line[x].alpha = 10;
			}
		}

		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp2(FreeImage_ConvertToColor(bmp1.get(), FIC_YUV, FICPARAM_YUV_STANDARD_JPEG), &::FreeImage_Unload);
		assert(bmp2 != nullptr);
		assert(FreeImage_GetColorType(bmp2.get()) == FIC_YUV);
		assert(FreeImage_GetBPP(bmp2.get()) == 32);
		for (unsigned y = 0; y < FreeImage_GetHeight(bmp2.get()); ++y) {
			const auto line = reinterpret_cast<FIRGBA8*>(FreeImage_GetScanLine(bmp2.get(), y));
			for (unsigned x = 0; x < FreeImage_GetWidth(bmp2.get()); ++x) {
				assert(std::abs(line[x].red   - 112) <= 1);
				assert(std::abs(line[x].green - 178) <= 1);
				assert(std::abs(line[x].blue  -  84) <= 1);
				assert(line[x].alpha == 10);
			}
		}

		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp3(FreeImage_ConvertToColor(bmp2.get(), FIC_RGBALPHA, FICPARAM_YUV_STANDARD_JPEG), &::FreeImage_Unload);
		assert(bmp3 != nullptr);
		assert(FreeImage_GetColorType(bmp3.get()) == FIC_RGBALPHA);
		assert(FreeImage_GetBPP(bmp3.get()) == 32);
		for (unsigned y = 0; y < FreeImage_GetHeight(bmp3.get()); ++y) {
			const auto line = reinterpret_cast<FIRGBA8*>(FreeImage_GetScanLine(bmp3.get(), y));
			for (unsigned x = 0; x < FreeImage_GetWidth(bmp3.get()); ++x) {
				assert(std::abs(line[x].red   -  50) <= 1);
				assert(std::abs(line[x].green - 127) <= 1);
				assert(std::abs(line[x].blue  - 200) <= 1);
				assert(line[x].alpha == 10);
			}
		}
	}
}

