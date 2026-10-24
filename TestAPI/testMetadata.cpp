// ==========================================================
// FreeImage 3 Test Script
//
// Metadata: malformed Exif data
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

#if FREEIMAGE_WITH_LIBJPEG

#include <vector>
#include <cstdint>

// ----------------------------------------------------------

static void push16(std::vector<uint8_t>& v, unsigned value) {
	v.push_back((uint8_t)(value & 0xFF));
	v.push_back((uint8_t)(value >> 8));
}

static void push32(std::vector<uint8_t>& v, unsigned value) {
	push16(v, value & 0xFFFF);
	push16(v, value >> 16);
}

static void pushIfdEntry(std::vector<uint8_t>& v, unsigned tag, unsigned type, unsigned count, unsigned value) {
	push16(v, tag);
	push16(v, type);
	push32(v, count);
	push32(v, value);
}

/**
Build the TIFF part of an Exif block (little endian):
IFD0 with the tag Make (of the given type) and the pointer to the Exif IFD, whose maker note holds one Canon tag 0x0001 (camera settings).
@param make_type FIDT type of the Make tag
@param canon_type FIDT type of the Canon tag
@param canon_count count of the Canon tag; the value bytes (2 * count) follow the IFD of the maker note
*/
static std::vector<uint8_t> makeExifTiff(unsigned make_type, unsigned canon_type, unsigned canon_count) {
	const unsigned make_offset = 38;
	const unsigned exif_offset = 44;
	const unsigned maker_offset = 62;
	const unsigned value_offset = maker_offset + 18;
	const unsigned value_size = 2 * canon_count;

	std::vector<uint8_t> t;
	t.push_back('I'); t.push_back('I'); push16(t, 42); push32(t, 8);
	// IFD0
	push16(t, 2);
	pushIfdEntry(t, 0x010F, make_type, 6, make_offset);
	pushIfdEntry(t, 0x8769, FIDT_LONG, 1, exif_offset);
	push32(t, 0);
	// Make
	const char make[6] = { 'C', 'a', 'n', 'o', 'n', 0 };
	t.insert(t.end(), make, make + 6);
	// Exif IFD
	push16(t, 1);
	pushIfdEntry(t, 0x927C, FIDT_UNDEFINED, 18 + value_size, maker_offset);
	push32(t, 0);
	// maker note: IFD with the Canon tag 0x0001, the value follows the IFD
	push16(t, 1);
	pushIfdEntry(t, 0x0001, canon_type, canon_count, (value_size > 4) ? value_offset : 0x44332211);
	push32(t, 0);
	for (unsigned i = 0; i < value_size; i++) {
		t.push_back((uint8_t)(i + 1));
	}
	return t;
}

/**
Load a JPEG with the given Exif block and return the number of the maker note tags (-1 if the file is rejected).
*/
static int countMakerNoteTags(const std::vector<uint8_t>& tiff) {
	FIBITMAP *src = FreeImage_Allocate(8, 8, 24);
	FIMEMORY *hsave = FreeImage_OpenMemory();
	FreeImage_SaveToMemory(FIF_JPEG, src, hsave, 0);
	FreeImage_Unload(src);

	uint8_t *data = nullptr;
	uint32_t size = 0;
	FreeImage_AcquireMemory(hsave, &data, &size);

	// SOI, APP1 (Exif), the rest of the file
	std::vector<uint8_t> file(data, data + 2);
	file.push_back(0xFF);
	file.push_back(0xE1);
	file.push_back((uint8_t)((2 + 6 + tiff.size()) >> 8));
	file.push_back((uint8_t)((2 + 6 + tiff.size()) & 0xFF));
	const char exif_id[6] = { 'E', 'x', 'i', 'f', 0, 0 };
	file.insert(file.end(), exif_id, exif_id + 6);
	file.insert(file.end(), tiff.begin(), tiff.end());
	file.insert(file.end(), data + 2, data + size);
	FreeImage_CloseMemory(hsave);

	FIMEMORY *hmem = FreeImage_OpenMemory(file.data(), (uint32_t)file.size());
	FIBITMAP *dib = FreeImage_LoadFromMemory(FIF_JPEG, hmem, 0);
	FreeImage_CloseMemory(hmem);
	if (!dib) {
		return -1;
	}
	const int count = (int)FreeImage_GetMetadataCount(FIMD_EXIF_MAKERNOTE, dib);
	FreeImage_Unload(dib);
	return count;
}

static bool expect(bool condition, const char *what) {
	if (!condition) {
		printf("testExifMalformed: FAILED: %s\n", what);
	}
	return condition;
}

// ----------------------------------------------------------

bool testExifMalformed() {
	printf("testExifMalformed ...\n");
	bool ok = true;

	// A Canon camera settings array of 4 shorts: the first member is skipped, the other 3 become tags
	ok &= expect(countMakerNoteTags(makeExifTiff(FIDT_ASCII, FIDT_SHORT, 4)) == 3, "valid Canon array of 4 shorts");

	// The same tag with the type BYTE and the count 4: only 4 bytes are stored, they are not an array of 16-bit values
	ok &= expect(countMakerNoteTags(makeExifTiff(FIDT_ASCII, FIDT_BYTE, 4)) == 0, "Canon tag which is not an array of shorts");

	printf(ok ? "testExifMalformed: OK\n" : "testExifMalformed: FAILED\n");
	return ok;
}

#endif // FREEIMAGE_WITH_LIBJPEG
