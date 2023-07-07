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
Test FreeImage_ConvertToFloat
*/
void testConvertToFloat()
{
	{
		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp_uint16(FreeImage_AllocateT(FIT_UINT16, 5, 1, 16), &::FreeImage_Unload);
		assert(bmp_uint16 != nullptr);

		uint16_t* bmp_uint16_ptr = reinterpret_cast<uint16_t*>(FreeImage_GetBits(bmp_uint16.get()));
		assert(bmp_uint16_ptr != nullptr);

		bmp_uint16_ptr[0] = 0;
		bmp_uint16_ptr[1] = 255;
		bmp_uint16_ptr[2] = 1000;
		bmp_uint16_ptr[3] = 10000;
		bmp_uint16_ptr[4] = 65535;

		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> res_scaled(FreeImage_ConvertToFloat(bmp_uint16.get(), TRUE), &::FreeImage_Unload);
		assert(res_scaled != nullptr);

		float* res_scaled_ptr = reinterpret_cast<float*>(FreeImage_GetBits(res_scaled.get()));
		assert(res_scaled_ptr != nullptr);
		assert(res_scaled_ptr[0] == 0.0f);
		assert(res_scaled_ptr[1] == 255.0f / 65535.0f);
		assert(res_scaled_ptr[2] == 1000.0f / 65535.0f);
		assert(res_scaled_ptr[3] == 10000.0f / 65535.0f);
		assert(res_scaled_ptr[4] == 1.0f);


		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> res(FreeImage_ConvertToFloat(bmp_uint16.get(), FALSE), &::FreeImage_Unload);
		assert(res != nullptr);

		float* res_ptr = reinterpret_cast<float*>(FreeImage_GetBits(res.get()));
		assert(res_ptr != nullptr);
		assert(res_ptr[0] == 0.0f);
		assert(res_ptr[1] == 255.0f);
		assert(res_ptr[2] == 1000.0f);
		assert(res_ptr[3] == 10000.0f);
		assert(res_ptr[4] == 65535.0f);
	}

	{
		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp_int32(FreeImage_AllocateT(FIT_INT32, 5, 1, 16), &::FreeImage_Unload);
		assert(bmp_int32 != nullptr);

		int32_t* bmp_uint32_ptr = reinterpret_cast<int32_t*>(FreeImage_GetBits(bmp_int32.get()));
		assert(bmp_uint32_ptr != nullptr);

		bmp_uint32_ptr[0] = 0;
		bmp_uint32_ptr[1] = 255;
		bmp_uint32_ptr[2] = 10000;
		bmp_uint32_ptr[3] = 65535;
		bmp_uint32_ptr[4] = std::numeric_limits<int32_t>::max();

		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> res_scaled(FreeImage_ConvertToFloat(bmp_int32.get(), TRUE), &::FreeImage_Unload);
		assert(res_scaled != nullptr);

		float* res_scaled_ptr = reinterpret_cast<float*>(FreeImage_GetBits(res_scaled.get()));
		assert(res_scaled_ptr != nullptr);
		assert(res_scaled_ptr[0] == 0.0f);
		assert(res_scaled_ptr[4] == 1.0f);


		std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> res(FreeImage_ConvertToFloat(bmp_int32.get(), FALSE), &::FreeImage_Unload);
		assert(res != nullptr);

		float* res_ptr = reinterpret_cast<float*>(FreeImage_GetBits(res.get()));
		assert(res_ptr != nullptr);
		assert(res_ptr[0] == 0.0f);
		assert(res_ptr[1] == 255.0f);
		assert(res_ptr[2] == 10000.0f);
		assert(res_ptr[3] == 65535.0f);
	}
}

