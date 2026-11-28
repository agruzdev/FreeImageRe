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
#include "FreeImage.hpp"

void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message) {
	printf("\n*** ");
	if(fif != FIF_UNKNOWN) {
		printf("%s Format\n", FreeImage_GetFormatFromFIF(fif));
	}
	printf("%s", message);
	printf(" ***\n");
}

static bool checkPluginRegistered(FREE_IMAGE_FORMAT fif, const char *name, bool compiledIn) {
	const bool registered = FreeImage_FIFSupportsReading(fif) || FreeImage_FIFSupportsWriting(fif);
	if (registered != compiledIn) {
		printf("FAILED: plugin %s is %s but %s\n", name,
			compiledIn ? "compiled in" : "compiled out",
			registered ? "registered" : "not registered");
		return false;
	}
	return true;
}

static bool checkPluginRegistry() {
	bool ok = true;
#define CHECK_PLUGIN(fif, macro) ok = checkPluginRegistered(fif, #fif, (macro) != 0) && ok
	CHECK_PLUGIN(FIF_TIFF, FREEIMAGE_WITH_LIBTIFF);
	CHECK_PLUGIN(FIF_HEIF, FREEIMAGE_WITH_LIBHEIF);
	CHECK_PLUGIN(FIF_AVIF, FREEIMAGE_WITH_LIBHEIF);
	CHECK_PLUGIN(FIF_JPEGXL, FREEIMAGE_WITH_LIBJPEGXL);
	CHECK_PLUGIN(FIF_JXR, FREEIMAGE_WITH_LIBJXR);
	CHECK_PLUGIN(FIF_JPEG, FREEIMAGE_WITH_LIBJPEG);
	CHECK_PLUGIN(FIF_PNG, FREEIMAGE_WITH_LIBPNG);
	CHECK_PLUGIN(FIF_BMP, FREEIMAGE_WITH_PLUGIN_BMP);
	CHECK_PLUGIN(FIF_GIF, FREEIMAGE_WITH_PLUGIN_GIF);
	CHECK_PLUGIN(FIF_ICO, FREEIMAGE_WITH_PLUGIN_ICO);
	CHECK_PLUGIN(FIF_PCX, FREEIMAGE_WITH_PLUGIN_PCX);
	CHECK_PLUGIN(FIF_PSD, FREEIMAGE_WITH_PLUGIN_PSD);
	CHECK_PLUGIN(FIF_TARGA, FREEIMAGE_WITH_PLUGIN_TARGA);
	CHECK_PLUGIN(FIF_DDS, FREEIMAGE_WITH_PLUGIN_DDS);
	CHECK_PLUGIN(FIF_HDR, FREEIMAGE_WITH_PLUGIN_HDR);
#undef CHECK_PLUGIN
	return ok;
}

class FreeImageTestEnvironment final : public ::testing::Environment {
public:
	void SetUp() override {
#if defined(FREEIMAGE_LIB) || !defined(WIN32)
		FreeImage_Initialise();
#endif
		FreeImage_SetOutputMessage(FreeImageErrorHandler);
		showPlugins();
	}

	void TearDown() override {
#if defined(FREEIMAGE_LIB) || !defined(WIN32)
		FreeImage_DeInitialise();
#endif
	}
};

namespace {
[[maybe_unused]] const auto *freeImageTestEnvironment =
	::testing::AddGlobalTestEnvironment(new FreeImageTestEnvironment);
}

TEST(Plugins, MatchBuildConfiguration)
{
	EXPECT_TRUE(checkPluginRegistry());
}

TEST(Dependencies, ReportVersions)
{
	const uint32_t depsCount = FreeImage_GetDependenciesCount();
	for (uint32_t i = 0; i < depsCount; ++i) {
		auto dep = FreeImage_GetDependencyInfo(i);
		ASSERT_NE(dep, nullptr);
		if (dep->fullVersion) {
			printf("%u %s\n", i, dep->fullVersion);
		}
		while (dep->next) {
			if (dep->fullVersion) {
				printf("  - %s\n", dep->next->fullVersion);
			}
			dep = dep->next;
		}
	}
}

