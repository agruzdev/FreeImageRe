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
#include <vector>

/**
Test FreeImage_GetHistogram
*/
TEST(Tools, Histogram)
{
	std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp(FreeImage_Allocate(4, 4, 32), & ::FreeImage_Unload);

	FIRGBA8* ptr = reinterpret_cast<FIRGBA8*>(FreeImage_GetScanLine(bmp.get(), 0));
	ptr[0].red = 1;
	ptr[0].green = 50;
	ptr[0].blue = 200;
	ptr[0].alpha = 30;

	ptr[1].red = 30;
	ptr[1].green = 10;
	ptr[1].blue = 140;
	ptr[1].alpha = 250;

	ptr[2].red = 180;
	ptr[2].green = 20;
	ptr[2].blue = 70;
	ptr[2].alpha = 0;

	ptr[3].red = 30;
	ptr[3].green = 40;
	ptr[3].blue = 0;
	ptr[3].alpha = 100;

	ptr = reinterpret_cast<FIRGBA8*>(FreeImage_GetScanLine(bmp.get(), 1));

	ptr[0].red = 233;
	ptr[0].green = 150;
	ptr[0].blue = 99;
	ptr[0].alpha = 100;

	ptr[1].red = 180;
	ptr[1].green = 220;
	ptr[1].blue = 230;
	ptr[1].alpha = 70;

	ptr[2].red = 20;
	ptr[2].green = 40;
	ptr[2].blue = 200;
	ptr[2].alpha = 230;

	ptr[3].red = 10;
	ptr[3].green = 20;
	ptr[3].blue = 230;
	ptr[3].alpha = 120;

	ptr = reinterpret_cast<FIRGBA8*>(FreeImage_GetScanLine(bmp.get(), 2));

	ptr[0].red = 200;
	ptr[0].green = 190;
	ptr[0].blue = 210;
	ptr[0].alpha = 100;

	ptr[1].red = 20;
	ptr[1].green = 30;
	ptr[1].blue = 40;
	ptr[1].alpha = 190;

	ptr[2].red = 255;
	ptr[2].green = 0;
	ptr[2].blue = 30;
	ptr[2].alpha = 50;

	ptr[3].red = 60;
	ptr[3].green = 30;
	ptr[3].blue = 90;
	ptr[3].alpha = 20;

	ptr = reinterpret_cast<FIRGBA8*>(FreeImage_GetScanLine(bmp.get(), 3));

	ptr[0].red = 30;
	ptr[0].green = 20;
	ptr[0].blue = 10;
	ptr[0].alpha = 199;

	ptr[1].red = 60;
	ptr[1].green = 80;
	ptr[1].blue = 180;
	ptr[1].alpha = 200;

	ptr[2].red = 200;
	ptr[2].green = 20;
	ptr[2].blue = 100;
	ptr[2].alpha = 100;

	ptr[3].red = 30;
	ptr[3].green = 40;
	ptr[3].blue = 10;
	ptr[3].alpha = 210;

	uint8_t histMin{}, histMax{};
	std::vector<uint32_t> hist(8 * 4);
	bool success = FreeImage_MakeHistogram(bmp.get(), 8, &histMin, &histMax, hist.data(), 4, hist.data() + 1, 4, hist.data() + 2, 4, hist.data() + 3, 4);
	EXPECT_TRUE(success);

	EXPECT_TRUE(hist[0]  == 8);
	EXPECT_TRUE(hist[4]  == 2);
	EXPECT_TRUE(hist[8]  == 0);
	EXPECT_TRUE(hist[12] == 0);
	EXPECT_TRUE(hist[16] == 0);
	EXPECT_TRUE(hist[20] == 2);
	EXPECT_TRUE(hist[24] == 2);
	EXPECT_TRUE(hist[28] == 2);

	EXPECT_TRUE(hist[1]  == 8);
	EXPECT_TRUE(hist[5]  == 4);
	EXPECT_TRUE(hist[9]  == 1);
	EXPECT_TRUE(hist[13] == 0);
	EXPECT_TRUE(hist[17] == 1);
	EXPECT_TRUE(hist[21] == 1);
	EXPECT_TRUE(hist[25] == 1);
	EXPECT_TRUE(hist[29] == 0);

	EXPECT_TRUE(hist[2]  == 4);
	EXPECT_TRUE(hist[6]  == 1);
	EXPECT_TRUE(hist[10] == 2);
	EXPECT_TRUE(hist[14] == 2);
	EXPECT_TRUE(hist[18] == 1);
	EXPECT_TRUE(hist[22] == 1);
	EXPECT_TRUE(hist[26] == 3);
	EXPECT_TRUE(hist[30] == 2);

	EXPECT_TRUE(hist[3]  == 3);
	EXPECT_TRUE(hist[7]  == 6);
	EXPECT_TRUE(hist[11] == 4);
	EXPECT_TRUE(hist[15] == 0);
	EXPECT_TRUE(hist[19] == 0);
	EXPECT_TRUE(hist[23] == 1);
	EXPECT_TRUE(hist[27] == 2);
	EXPECT_TRUE(hist[31] == 0);
}

