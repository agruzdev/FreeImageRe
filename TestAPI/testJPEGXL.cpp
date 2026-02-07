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
#include <iostream>

// Local test functions
// ----------------------------------------------------------

void testJpegXl(FREE_IMAGE_FORMAT fif, const char* src_path, const char* dst_path)
{
	auto detected_fif = FreeImage_GetFIFFromFilename(src_path);
	assert(detected_fif == fif);

	detected_fif = FreeImage_GetFileType(src_path);
	assert(detected_fif == fif);

	std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> img_heic{ FreeImage_Load(fif, src_path, 0), &::FreeImage_Unload };
	assert(img_heic != nullptr);
	std::cout << "Test JPEGXL " << fif << ", size = " << FreeImage_GetWidth(img_heic.get()) << "x" << FreeImage_GetHeight(img_heic.get()) << std::endl;

	const bool success = FreeImage_Save(fif, img_heic.get(), dst_path);
	assert(success);
}
