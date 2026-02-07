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
# define NOMINMAX
# include <windows.h>
#endif
#include <array>

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
			if (lpReserved == nullptr) {
				FreeImage_DeInitialise();
			}
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
	static const char* version = FI_QUOTE(FREEIMAGERE_MAJOR_VERSION) "." FI_QUOTE(FREEIMAGERE_MINOR_VERSION) "." FI_QUOTE(FREEIMAGERE_PATCH_VERSION);
	return version;
}

void DLL_CALLCONV
FreeImageRe_GetVersionNumbers(int* major, int* minor, int* patch) {
	if (major) {
		*major = FREEIMAGERE_MAJOR_VERSION;
	}
	if (minor) {
		*minor = FREEIMAGERE_MINOR_VERSION;
	}
	if (patch) {
		*patch = FREEIMAGERE_PATCH_VERSION;
	}
}

// Forward declaration for version functions
const FIDEPENDENCY* GetPngDependencyInfo();
const FIDEPENDENCY* GetJpegDependencyInfo();
const FIDEPENDENCY* GetJpeg2kDependencyInfo();
const FIDEPENDENCY* GetExrDependencyInfo();
const FIDEPENDENCY* GetTiffDependencyInfo();
const FIDEPENDENCY* GetRawDependencyInfo();
const FIDEPENDENCY* GetWebpDependencyInfo();
const FIDEPENDENCY* GetJxrDependencyInfo();
const FIDEPENDENCY* GetHeifDependencyInfo();
const FIDEPENDENCY* GetJpegXlDependencyInfo();

namespace {
	
	const FIDEPENDENCY* GetZLibDependencyInfo() {
		static const FIDEPENDENCY info = {
			.name = "zlib",
			.fullVersion  = "zlib v" ZLIB_VERSION,
			.majorVersion = ZLIB_VER_MAJOR,
			.minorVersion = ZLIB_VER_MINOR
		};
		return &info;
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
			return mEntries.at(index);
		}

	private:
		DependenciesTable() {
			Append(GetZLibDependencyInfo());
#if FREEIMAGE_WITH_LIBPNG
			Append(GetPngDependencyInfo());
#endif
#if FREEIMAGE_WITH_LIBJPEG
			Append(GetJpegDependencyInfo());
#endif
#if FREEIMAGE_WITH_LIBOPENJPEG
			Append(GetJpeg2kDependencyInfo());
#endif
#if FREEIMAGE_WITH_LIBOPENEXR
			Append(GetExrDependencyInfo());
#endif
#if FREEIMAGE_WITH_LIBTIFF
			Append(GetTiffDependencyInfo());
#endif
#if FREEIMAGE_WITH_LIBRAW
			Append(GetRawDependencyInfo());
#endif
#if FREEIMAGE_WITH_LIBWEBP
			Append(GetWebpDependencyInfo());
#endif
#if FREEIMAGE_WITH_LIBJXR
			Append(GetJxrDependencyInfo());
#endif
#if FREEIMAGE_WITH_LIBHEIF
			Append(GetHeifDependencyInfo());
#endif
#if FREEIMAGE_WITH_LIBJPEGXL
			Append(GetJpegXlDependencyInfo());
#endif
		}

		void Append(const FIDEPENDENCY* dep) {
			if (dep) {
				mEntries.emplace_back(std::move(dep));
			}
		}

		std::vector<const FIDEPENDENCY*> mEntries;	// not null
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

constexpr size_t kMaxMessageProcessorsNumber = 32;

struct MessageProcessorRecord
{
	uint32_t id{ };
	void* ctx{ nullptr };
	FreeImage_ProcessMessageFunction func{ nullptr };
};

// ToDo: implement an object pool?
thread_local std::array<MessageProcessorRecord, kMaxMessageProcessorsNumber> g_message_processors{ };


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
	constexpr size_t MSG_SIZE = 512;

	if (fmt && (!g_message_processors.empty()  || freeimage_outputmessage_proc || freeimage_outputmessagestdcall_proc)) {
		FIMESSAGE message{ static_cast<FREE_IMAGE_FORMAT>(fif), FISEV_WARNING };
		message.text.resize(MSG_SIZE);

		va_list arg;
		va_start(arg, fmt);

		std::vsnprintf(message.text.data(), message.text.size(), fmt, arg);

		va_end(arg);

		// output the message to the user program

		if (!g_message_processors.empty()) {
			FreeImage_ProcessMessage(& message);
		}
		else {
			// legacy behaviour

			if (freeimage_outputmessage_proc)
				freeimage_outputmessage_proc(message.scope, message.text.c_str());

			if (freeimage_outputmessagestdcall_proc)
				freeimage_outputmessagestdcall_proc(message.scope, message.text.c_str());
		}
	}
}


uint32_t DLL_CALLCONV
FreeImage_AddProcessMessageFunction(void* ctx, FreeImage_ProcessMessageFunction func)
{
	static_assert(kMaxMessageProcessorsNumber < std::numeric_limits<uint32_t>::max() - 1);
	if (func == nullptr) {
		return 0;
	}

	size_t free_idx = 0;
	for (; free_idx < g_message_processors.size(); ++free_idx) {
		if (!g_message_processors[free_idx].id) {
			break;
		}
	}
	if (free_idx >= g_message_processors.size()) {
		return 0;
	}

	auto& rec = g_message_processors[free_idx];
	rec.id   = static_cast<uint32_t>(free_idx) + 1;
	rec.ctx  = ctx;
	rec.func = func;

	return rec.id;
}


FIBOOL DLL_CALLCONV
FreeImage_RemoveProcessMessageFunction(uint32_t id)
{
	if (id == 0) {
		return FALSE;
	}

	const uint32_t idx = id - 1;
	if (idx >= g_message_processors.size()) {
		return FALSE;
	}

	auto& rec = g_message_processors[idx];
	if (rec.id != id) {
		return FALSE;
	}

	rec.id   = 0;
	rec.func = nullptr;
	rec.ctx  = nullptr;

	return TRUE;
}


void DLL_CALLCONV
FreeImage_ProcessMessage(const FIMESSAGE* msg)
{
	if (!msg) {
		return;
	}

	for (const auto& rec : g_message_processors) {
		if (rec.id && rec.func) {
			rec.func(rec.ctx, msg);
		}
	}
}


FIMESSAGE* DLL_CALLCONV
FreeImage_CreateMessage(FREE_IMAGE_FORMAT scope, FREE_IMAGE_SEVERITY severity, const char* what)
{
	return new FIMESSAGE(scope, severity, what ? what : "Unknown");
}


void DLL_CALLCONV
FreeImage_DeleteMessage(FIMESSAGE* msg)
{
	if (msg) {
		delete msg;
	}
}


FREE_IMAGE_FORMAT DLL_CALLCONV
FreeImage_GetMessageScope(const FIMESSAGE* msg)
{
	if (msg) {
		return msg->scope;
	}
	return FIF_UNKNOWN;
}


FREE_IMAGE_SEVERITY DLL_CALLCONV
FreeImage_GetMessageSeverity(const FIMESSAGE* msg)
{
	if (msg) {
		return msg->severity;
	}
	return FISEV_NONE;
}


const char* DLL_CALLCONV
FreeImage_GetMessageString(const FIMESSAGE* msg)
{
	if (msg) {
		return msg->text.c_str();
	}
	return nullptr;
}
