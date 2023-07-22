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


void testTmoClamp()
{
	{
		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp1(FreeImage_AllocateT(FIT_RGBF, 4, 1, 3 * sizeof(float)), &::FreeImage_Unload);
		assert(bmp1 != nullptr);
		assert(FreeImage_GetColorType(bmp1.get()) == FIC_RGB);

		const auto srcLine = reinterpret_cast<FIRGBF*>(FreeImage_GetBits(bmp1.get()));

		srcLine[0].red   = 0.0f;
		srcLine[0].green = 0.1f;
		srcLine[0].blue  = 1.0f;

		srcLine[1].red   = 0.5f;
		srcLine[1].green = 0.2f;
		srcLine[1].blue  = 0.9f;

		srcLine[2].red   = 0.3f;
		srcLine[2].green = 0.4f;
		srcLine[2].blue  = 0.0f;

		srcLine[3].red   = 1.0f;
		srcLine[3].green = 0.1f;
		srcLine[3].blue  = 0.7f;

		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> res(FreeImage_TmoClamp(bmp1.get()), &::FreeImage_Unload);
		assert(res != nullptr);

		const auto resLine = reinterpret_cast<FIRGB8*>(FreeImage_GetBits(res.get()));

		assert(resLine[0].red == 0);
		assert(std::abs(resLine[0].green - 25) <= 1);
		assert(resLine[0].blue == 255);

		assert(std::abs(resLine[1].red   - 128) <= 1);
		assert(std::abs(resLine[1].green -  51) <= 1);
		assert(std::abs(resLine[1].blue  - 230) <= 1);

		assert(std::abs(resLine[2].red   -  76) <= 1);
		assert(std::abs(resLine[2].green - 102) <= 1);
		assert(resLine[2].blue == 0);

		assert(resLine[3].red == 255);
		assert(std::abs(resLine[3].green -  25) <= 1);
		assert(std::abs(resLine[3].blue  - 179) <= 1);
	}

	{
		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp1(FreeImage_AllocateT(FIT_RGBAF, 4, 1, 3 * sizeof(float)), &::FreeImage_Unload);
		assert(bmp1 != nullptr);
		assert(FreeImage_GetColorType(bmp1.get()) == FIC_RGBALPHA);

		const auto srcLine = reinterpret_cast<FIRGBF*>(FreeImage_GetBits(bmp1.get()));

		srcLine[0].red = 0.0f;
		srcLine[0].green = 0.1f;
		srcLine[0].blue = 1.0f;

		srcLine[1].red = 0.5f;
		srcLine[1].green = 0.2f;
		srcLine[1].blue = 0.9f;

		srcLine[2].red = 0.3f;
		srcLine[2].green = 0.4f;
		srcLine[2].blue = 0.0f;

		srcLine[3].red = 1.0f;
		srcLine[3].green = 0.1f;
		srcLine[3].blue = 0.7f;

		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> res(FreeImage_ToneMapping(bmp1.get(), FITMO_CLAMP, 0.0, 100.0), &::FreeImage_Unload);
		assert(res != nullptr);

		const auto resLine = reinterpret_cast<FIRGB8*>(FreeImage_GetBits(res.get()));

		assert(resLine[0].red == 0);
		assert(std::abs(resLine[0].green - 25) <= 1);
		assert(resLine[0].blue == 100);

		assert(std::abs(resLine[1].red  - 100) <= 1);
		assert(std::abs(resLine[1].green - 51) <= 1);
		assert(std::abs(resLine[1].blue - 100) <= 1);

		assert(std::abs(resLine[2].red - 76) <= 1);
		assert(std::abs(resLine[2].green - 100) <= 1);
		assert(resLine[2].blue == 0);

		assert(resLine[3].red == 100);
		assert(std::abs(resLine[3].green - 25) <= 1);
		assert(std::abs(resLine[3].blue - 100) <= 1);
	}

	{
		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp1(FreeImage_AllocateT(FIT_RGB16, 4, 1, 3 * sizeof(float)), &::FreeImage_Unload);
		assert(bmp1 != nullptr);
		assert(FreeImage_GetColorType(bmp1.get()) == FIC_RGB);

		const auto srcLine = reinterpret_cast<FIRGB16*>(FreeImage_GetBits(bmp1.get()));

		srcLine[0].red   = 0;
		srcLine[0].green = 100;
		srcLine[0].blue  = 150;

		srcLine[1].red   = 1000;
		srcLine[1].green = 65535;
		srcLine[1].blue  = 90;

		srcLine[2].red   = 255;
		srcLine[2].green = 256;
		srcLine[2].blue  = 1;

		srcLine[3].red   = 140;
		srcLine[3].green = 30;
		srcLine[3].blue  = 210;

		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> res(FreeImage_ToneMapping(bmp1.get(), FITMO_CLAMP), &::FreeImage_Unload);
		assert(res != nullptr);

		const auto resLine = reinterpret_cast<FIRGB8*>(FreeImage_GetBits(res.get()));

		assert(resLine[0].red == 0);
		assert(resLine[0].green == 100);
		assert(resLine[0].blue == 150);

		assert(resLine[1].red == 255);
		assert(resLine[1].green == 255);
		assert(resLine[1].blue == 90);

		assert(resLine[2].red == 255);
		assert(resLine[2].green == 255);
		assert(resLine[2].blue == 1);

		assert(resLine[3].red == 140);
		assert(resLine[3].green == 30);
		assert(resLine[3].blue == 210);
	}
}

