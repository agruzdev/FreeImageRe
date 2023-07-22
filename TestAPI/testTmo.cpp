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
		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp1(FreeImage_AllocateT(FIT_RGBF, 4, 1, 8 * 3 * sizeof(float)), &::FreeImage_Unload);
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
		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp1(FreeImage_AllocateT(FIT_RGBA16, 4, 1, 8 * 4 * sizeof(uint16_t)), &::FreeImage_Unload);
		assert(bmp1 != nullptr);
		assert(FreeImage_GetColorType(bmp1.get()) == FIC_RGBALPHA);

		const auto srcLine = reinterpret_cast<FIRGBA16*>(FreeImage_GetBits(bmp1.get()));

		srcLine[0].red   = 0;
		srcLine[0].green = 100;
		srcLine[0].blue  = 150;

		srcLine[1].red   = 1000;
		srcLine[1].green = 65535;
		srcLine[1].blue  = 90;

		srcLine[2].red   = 255;
		srcLine[2].green = 256;
		srcLine[2].blue  = 1;

		srcLine[3].red   = 11400;
		srcLine[3].green = 60000;
		srcLine[3].blue  = 2100;

		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> res(FreeImage_ToneMapping(bmp1.get(), FITMO_CLAMP), &::FreeImage_Unload);
		assert(res != nullptr);

		const auto resLine = reinterpret_cast<FIRGBA8*>(FreeImage_GetBits(res.get()));

		assert(resLine[0].red == 0);
		assert(std::abs(resLine[0].green - 0) <= 1);
		assert(std::abs(resLine[0].blue  - 0) <= 1);

		assert(std::abs(resLine[1].red  - 3) <= 1);
		assert(resLine[1].green == 255);
		assert(std::abs(resLine[1].blue - 0) <= 1);

		assert(std::abs(resLine[2].red   - 0) <= 1);
		assert(std::abs(resLine[2].green - 1) <= 1);
		assert(std::abs(resLine[2].blue  - 0) <= 1);

		assert(std::abs(resLine[3].red    - 44) <= 1);
		assert(std::abs(resLine[3].green - 234) <= 1);
		assert(std::abs(resLine[3].blue  -   8) <= 1);
	}

	{
		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp1(FreeImage_AllocateT(FIT_RGB32, 4, 1, 8 * 3 * sizeof(uint32_t)), &::FreeImage_Unload);
		assert(bmp1 != nullptr);
		assert(FreeImage_GetColorType(bmp1.get()) == FIC_RGB);

		const auto srcLine = reinterpret_cast<FIRGB32*>(FreeImage_GetBits(bmp1.get()));

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
		srcLine[3].green = 1023;
		srcLine[3].blue  = 210;

		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> res(FreeImage_ToneMapping(bmp1.get(), FITMO_CLAMP, 1023), &::FreeImage_Unload);
		assert(res != nullptr);

		const auto resLine = reinterpret_cast<FIRGB8*>(FreeImage_GetBits(res.get()));

		assert(resLine[0].red == 0);
		assert(std::abs(resLine[0].green - 25) <= 1);
		assert(std::abs(resLine[0].blue  - 37) <= 1);

		assert(std::abs(resLine[1].red - 250) <= 1);
		assert(resLine[1].green == 255);
		assert(std::abs(resLine[1].blue - 22) <= 1);

		assert(std::abs(resLine[2].red   - 63) <= 1);
		assert(std::abs(resLine[2].green - 64) <= 1);
		assert(std::abs(resLine[2].blue  -  0) <= 1);

		assert(std::abs(resLine[3].red - 35) <= 1);
		assert(resLine[3].green == 255);
		assert(std::abs(resLine[3].blue - 52) <= 1);
	}
}


void testTmoLinear()
{
	{
		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp1(FreeImage_AllocateT(FIT_RGBF, 4, 1, 8 * 3 * sizeof(float)), &::FreeImage_Unload);
		assert(bmp1 != nullptr);
		assert(FreeImage_GetColorType(bmp1.get()) == FIC_RGB);

		const auto srcLine = reinterpret_cast<FIRGBF*>(FreeImage_GetBits(bmp1.get()));

		srcLine[0].red = 0.0f;
		srcLine[0].green = 0.0f;
		srcLine[0].blue = 0.0f;

		srcLine[1].red = 1.5f;
		srcLine[1].green = 1.2f;
		srcLine[1].blue = 1.9f;

		srcLine[2].red = 10.3f;
		srcLine[2].green = 10.4f;
		srcLine[2].blue = 10.0f;

		srcLine[3].red = 17.0f;
		srcLine[3].green = 17.0f;
		srcLine[3].blue = 17.0f;

		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> res(FreeImage_TmoLinear(bmp1.get()), &::FreeImage_Unload);
		assert(res != nullptr);

		const auto resLine = reinterpret_cast<FIRGB8*>(FreeImage_GetBits(res.get()));

		assert(resLine[0].red   == 0);
		assert(resLine[0].green == 0);
		assert(resLine[0].blue  == 0);

		assert(resLine[3].red   == 255);
		assert(resLine[3].green == 255);
		assert(resLine[3].blue  == 255);
	}


	{
		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp1(FreeImage_AllocateT(FIT_RGBA16, 4, 1, 8 * 4 * sizeof(uint16_t)), &::FreeImage_Unload);
		assert(bmp1 != nullptr);
		assert(FreeImage_GetColorType(bmp1.get()) == FIC_RGBALPHA);

		const auto srcLine = reinterpret_cast<FIRGBA16*>(FreeImage_GetBits(bmp1.get()));

		srcLine[0].red   = 120;
		srcLine[0].green = 120;
		srcLine[0].blue  = 120;
		srcLine[0].alpha = 0;

		srcLine[1].red   = 50;
		srcLine[1].green = 50;
		srcLine[1].blue  = 50;
		srcLine[1].alpha = 1000;

		srcLine[2].red   = 500;
		srcLine[2].green = 500;
		srcLine[2].blue  = 500;
		srcLine[2].alpha = 500;

		srcLine[3].red   = 1000;
		srcLine[3].green = 1000;
		srcLine[3].blue  = 1000;
		srcLine[3].alpha = 1000;

		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> res(FreeImage_ToneMapping(bmp1.get(), FITMO_LINEAR, 1000.0), &::FreeImage_Unload);
		assert(res != nullptr);

		const auto resLine = reinterpret_cast<FIRGBA8*>(FreeImage_GetBits(res.get()));

		assert(resLine[1].red   == 0);
		assert(resLine[1].green == 0);
		assert(resLine[1].blue  == 0);

		assert(resLine[3].red   == 255);
		assert(resLine[3].green == 255);
		assert(resLine[3].blue  == 255);

		assert(resLine[0].alpha == 0);
		assert(resLine[1].alpha == 255);
		assert(std::abs(resLine[2].alpha - 127) <= 1);
		assert(resLine[3].alpha == 255);
	}


	{
		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp1(FreeImage_AllocateT(FIT_DOUBLE, 4, 1, 8 * sizeof(double)), &::FreeImage_Unload);
		assert(bmp1 != nullptr);
		assert(FreeImage_GetColorType(bmp1.get()) == FIC_MINISBLACK);

		const auto srcLine = reinterpret_cast<double*> (FreeImage_GetBits(bmp1.get()));

		srcLine[0] = 10.0;
		srcLine[1] = 50.0;
		srcLine[2] = 500.0;
		srcLine[3] = 1010.0;

		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> res(FreeImage_ToneMapping(bmp1.get(), FITMO_LINEAR), &::FreeImage_Unload);
		assert(res != nullptr);

		const auto resLine = reinterpret_cast<uint8_t*>(FreeImage_GetBits(res.get()));

		assert(resLine[0] == 0);
		assert(std::abs(resLine[1] - 10) <= 1);
		assert(std::abs(resLine[2] - 125) <= 1);
		assert(resLine[3] == 255);
	}

	{
		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp1(FreeImage_AllocateT(FIT_UINT16, 4, 1, 8 * sizeof(uint16_t)), &::FreeImage_Unload);
		assert(bmp1 != nullptr);
		assert(FreeImage_GetColorType(bmp1.get()) == FIC_MINISBLACK);

		const auto srcLine = reinterpret_cast<uint16_t*> (FreeImage_GetBits(bmp1.get()));

		srcLine[0] = 500;
		srcLine[1] = 500;
		srcLine[2] = 500;
		srcLine[3] = 500;

		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> res(FreeImage_ToneMapping(bmp1.get(), FITMO_LINEAR, 1023.0), &::FreeImage_Unload);
		assert(res != nullptr);

		const auto resLine = reinterpret_cast<uint8_t*>(FreeImage_GetBits(res.get()));

		assert(std::abs(resLine[0] - 125) <= 1);
		assert(std::abs(resLine[1] - 125) <= 1);
		assert(std::abs(resLine[2] - 125) <= 1);
		assert(std::abs(resLine[3] - 125) <= 1);
	}
}



