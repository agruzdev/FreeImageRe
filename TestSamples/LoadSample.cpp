// LoadSample: loads one image file through the public C API and unloads it again.
// Used by run-samples.sh, best together with an AddressSanitizer build: the process exits
// with 0 if the file was loaded, rejected or not recognised, and is aborted by the
// sanitizer (or crashes) on a memory error.

#include "FreeImage.h"

#include <cstdio>

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::fprintf(stderr, "usage: LoadSample <file>\n");
		return 2;
	}

	const FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(argv[1], 0);
	if (fif == FIF_UNKNOWN) {
		std::printf("unknown\n");
		return 0;
	}

	FIBITMAP* dib = FreeImage_Load(fif, argv[1], 0);
	std::printf(dib ? "loaded\n" : "rejected\n");
	FreeImage_Unload(dib);
	return 0;
}
