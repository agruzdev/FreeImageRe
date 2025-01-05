// ==========================================================
// FreeImage implementation
//
// Design and implementation by
// - Floris van den Berg (flvdberg@wxs.nl)
// - Herve Drolon (drolon@infonie.fr)
// - Karl-Heinz Bussian (khbussian@moss.de)
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


#ifdef _WIN32
#include <windows.h>
#endif

#include "zlib.h"

#include "FreeImage.h"
#include "Utilities.h"

//----------------------------------------------------------------------

static const char *s_copyright = "This program uses FreeImage, a free, open source image library supporting all common bitmap formats. See http://freeimage.sourceforge.net for details";

//----------------------------------------------------------------------

#if defined(_WIN32) && !defined(__MINGW32__)
#ifndef FREEIMAGE_LIB

FIBOOL APIENTRY
DllMain(HANDLE hModule, uint32_t ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH :
			FreeImage_Initialise(FALSE);
			break;

		case DLL_PROCESS_DETACH :
			FreeImage_DeInitialise();
			break;

		case DLL_THREAD_ATTACH :
		case DLL_THREAD_DETACH :
			break;
    }

    return TRUE;
}

#endif // FREEIMAGE_LIB

#else // !_WIN32 
#ifndef FREEIMAGE_LIB

void FreeImage_SO_Initialise() __attribute__((constructor));
void FreeImage_SO_DeInitialise() __attribute__((destructor));

void FreeImage_SO_Initialise() {
    FreeImage_Initialise(FALSE);
}

void FreeImage_SO_DeInitialise() {
    // FreeImage_SO_DeInitialise is called after all static objects are destoryed. Nothing to do here.
    //FreeImage_DeInitialise();
}
#endif // FREEIMAGE_LIB

#endif // _WIN32

//----------------------------------------------------------------------

const char * DLL_CALLCONV
FreeImage_GetVersion() {
	static char s_version[16];
	snprintf(s_version, std::size(s_version), "%d.%d.%d", FREEIMAGE_MAJOR_VERSION, FREEIMAGE_MINOR_VERSION, FREEIMAGE_RELEASE_SERIAL);
	return s_version;
}

const char * DLL_CALLCONV
FreeImage_GetCopyrightMessage() {
	return s_copyright;
}

const char* DLL_CALLCONV
FreeImageRe_GetVersion() {
	static const char* version = FI_QUOTE(FREEIMAGERE_MAJOR_VERSION) "." FI_QUOTE(FREEIMAGERE_MINOR_VERSION);
	return version;
}

void DLL_CALLCONV
FreeImageRe_GetVersionNumbers(int* major, int* minor) {
	if (major) {
		*major = FREEIMAGERE_MAJOR_VERSION;
	}
	if (minor) {
		*minor = FREEIMAGERE_MINOR_VERSION;
	}
}

// Forward declaration for version functions
FIDEPENDENCY MakePngDependencyInfo();
FIDEPENDENCY MakeJpegDependencyInfo();
FIDEPENDENCY MakeJpeg2kDependencyInfo();
FIDEPENDENCY MakeExrDependencyInfo();
FIDEPENDENCY MakeTiffDependencyInfo();
FIDEPENDENCY MakeRawDependencyInfo();
FIDEPENDENCY MakeWebpDependencyInfo();
FIDEPENDENCY MakeJxrDependencyInfo();

namespace {
	
	FIDEPENDENCY MakeZLibDependencyInfo() {
		FIDEPENDENCY info{};
		info.name = "zlib";
		info.fullVersion = ZLIB_VERSION;
		info.majorVersion = ZLIB_VER_MAJOR;
		info.minorVersion = ZLIB_VER_MINOR;
		return info;
	}

	class DependenciesTable {
	public:
		static const DependenciesTable& GetInstance() {
			static DependenciesTable instance;
			return instance;
		}

		uint32_t GetSize() const {
			assert(mEntries.size() <= UINT32_MAX);
			return static_cast<uint32_t>(mEntries.size());
		}

		const FIDEPENDENCY* GetByIndex(uint32_t index) const {
			return &mEntries.at(index);
		}

	private:
		DependenciesTable() {
			mEntries.emplace_back(MakeZLibDependencyInfo());
#if FREEIMAGE_WITH_LIBPNG
			mEntries.emplace_back(MakePngDependencyInfo());
#endif
#if FREEIMAGE_WITH_LIBJPEG
			mEntries.emplace_back(MakeJpegDependencyInfo());
#endif
#if FREEIMAGE_WITH_LIBOPENJPEG
			mEntries.emplace_back(MakeJpeg2kDependencyInfo());
#endif
#if FREEIMAGE_WITH_LIBOPENEXR
			mEntries.emplace_back(MakeExrDependencyInfo());
#endif
#if FREEIMAGE_WITH_LIBTIFF
			mEntries.emplace_back(MakeTiffDependencyInfo());
#endif
#if FREEIMAGE_WITH_LIBRAW
			mEntries.emplace_back(MakeRawDependencyInfo());
#endif
#if FREEIMAGE_WITH_LIBWEBP
			mEntries.emplace_back(MakeWebpDependencyInfo());
#endif
#if FREEIMAGE_WITH_LIBJXR
			mEntries.emplace_back(MakeJxrDependencyInfo());
#endif
		}

		std::vector<FIDEPENDENCY> mEntries;
	};

} // namespace

uint32_t DLL_CALLCONV 
FreeImage_GetDependenciesCount(void)
try {
	return DependenciesTable::GetInstance().GetSize();
}
catch (...) {
	return 0;
}

const FIDEPENDENCY* DLL_CALLCONV
FreeImage_GetDependencyInfo(uint32_t index) 
try {
	return DependenciesTable::GetInstance().GetByIndex(index);
}
catch (...) {
	return nullptr;
}

//----------------------------------------------------------------------

FIBOOL DLL_CALLCONV
FreeImage_IsLittleEndian() {
	union {
		uint32_t i;
		uint8_t c[4];
	} u;
	u.i = 1;
	return (u.c[0] != 0);
}

//----------------------------------------------------------------------

static FreeImage_OutputMessageFunction freeimage_outputmessage_proc{};
static FreeImage_OutputMessageFunctionStdCall freeimage_outputmessagestdcall_proc{};

void DLL_CALLCONV
FreeImage_SetOutputMessage(FreeImage_OutputMessageFunction omf) {
	freeimage_outputmessage_proc = omf;
}

void DLL_CALLCONV
FreeImage_SetOutputMessageStdCall(FreeImage_OutputMessageFunctionStdCall omf) {
	freeimage_outputmessagestdcall_proc = omf;
}

void DLL_CALLCONV
FreeImage_OutputMessageProc(int fif, const char *fmt, ...) {
	const int MSG_SIZE = 512; // 512 bytes should be more than enough for a short message

	if (fmt && (freeimage_outputmessage_proc || freeimage_outputmessagestdcall_proc)) {
		char message[MSG_SIZE];
		memset(message, 0, MSG_SIZE);

		// initialize the optional parameter list

		va_list arg;
		va_start(arg, fmt);

		// check the length of the format string

		int str_length = (int)( (strlen(fmt) > MSG_SIZE) ? MSG_SIZE : strlen(fmt) );

		// parse the format string and put the result in 'message'

		for (int i = 0, j = 0; i < str_length; ++i) {
			if (fmt[i] == '%') {
				if (i + 1 < str_length) {
					switch (tolower(fmt[i + 1])) {
						case '%' :
							message[j++] = '%';
							break;

						case 'o' : // octal numbers
						{
							char tmp[16];

							_itoa(va_arg(arg, int), tmp, 8);

							strcat(message, tmp);

							j += (int)strlen(tmp);

							++i;

							break;
						}

						case 'i' : // decimal numbers
						case 'd' :
						{
							char tmp[16];

							_itoa(va_arg(arg, int), tmp, 10);

							strcat(message, tmp);

							j += (int)strlen(tmp);

							++i;

							break;
						}

						case 'x' : // hexadecimal numbers
						{
							char tmp[16];

							_itoa(va_arg(arg, int), tmp, 16);

							strcat(message, tmp);

							j += (int)strlen(tmp);

							++i;

							break;
						}

						case 's' : // strings
						{
							char *tmp = va_arg(arg, char*);

							strcat(message, tmp);

							j += (int)strlen(tmp);

							++i;

							break;
						}
					};
				} else {
					message[j++] = fmt[i];
				}
			} else {
				message[j++] = fmt[i];
			};
		}

		// deinitialize the optional parameter list

		va_end(arg);

		// output the message to the user program

		if (freeimage_outputmessage_proc)
			freeimage_outputmessage_proc((FREE_IMAGE_FORMAT)fif, message);

		if (freeimage_outputmessagestdcall_proc)
			freeimage_outputmessagestdcall_proc((FREE_IMAGE_FORMAT)fif, message); 
	}
}
