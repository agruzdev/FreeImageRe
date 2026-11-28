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

#ifndef TEST_FREEIMAGE_API_H
#define TEST_FREEIMAGE_API_H

#include "FreeImage.h"
#include "FreeImageOptions.h"

#include <gtest/gtest.h>

#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include <stdlib.h>

#if (defined(WIN32) || defined(__WIN32__))
#if (defined(_DEBUG))
#include <crtdbg.h>
#endif
#endif // WIN32

// Some useful tools
// ==========================================================
FIBITMAP* createZonePlateImage(unsigned width, unsigned height, int scale);

// Test plugins capabilities
// ==========================================================
void showPlugins();

#endif // TEST_FREEIMAGE_API_H


