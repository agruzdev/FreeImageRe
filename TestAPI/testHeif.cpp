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

void testHeif(const char *src_file) 
{
	std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> img_heic{ FreeImage_Load(FIF_HEIF, src_file, 0), &::FreeImage_Unload };
	assert(img_heic != nullptr);
	std::cout << "heic size = " << FreeImage_GetWidth(img_heic.get()) << "x" << FreeImage_GetHeight(img_heic.get()) << std::endl;

	const bool success = FreeImage_Save(FIF_HEIF, img_heic.get(), "heic_out.heic");
	assert(success);
}

void testAvif(const char* src_file)
{
	std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> img_heic{ FreeImage_Load(FIF_AVIF, src_file, 0), &::FreeImage_Unload };
	assert(img_heic != nullptr);
	std::cout << "avis size = " << FreeImage_GetWidth(img_heic.get()) << "x" << FreeImage_GetHeight(img_heic.get()) << std::endl;

	const bool success = FreeImage_Save(FIF_AVIF, img_heic.get(), "avif_out.avif");
	assert(success);
}
