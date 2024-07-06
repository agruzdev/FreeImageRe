// =====================================================================
// FreeImage Plugin Interface
//
// Design and implementation by
// - Floris van den Berg (floris@geekhq.nl)
// - Rui Lopes (ruiglopes@yahoo.com)
// - Detlev Vendt (detlev.vendt@brillit.de)
// - Petr Pytelka (pyta@lightcomp.com)
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
// =====================================================================

#ifdef _MSC_VER 
#pragma warning (disable : 4786) // identifier was truncated to 'number' characters
#endif

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <ctype.h>
#endif // _WIN32

#include "FreeImage.h"
#include "Utilities.h"
#include "FreeImageIO.h"
#include "Plugin.h"

#include "../Metadata/FreeImageTag.h"

// =====================================================================

using namespace std;

// =====================================================================
// Plugin search list
// =====================================================================

const char *
s_search_list[] = {
	"",
	"plugins\\",
};

#if defined(_WIN32) && !defined(__MINGW32__)
# define GCC_INIT_PRIORITY(Val_)
#else 
# define GCC_INIT_PRIORITY(Val_) __attribute__((init_priority(Val_)))
#endif

static std::unique_ptr<PluginList> s_plugins GCC_INIT_PRIORITY(101) { };    // init before FreeImage_SO_Initialize
static int s_plugin_reference_count = 0;


// =====================================================================
// Reimplementation of stricmp (it is not supported on some systems)
// =====================================================================

int
FreeImage_stricmp(const char *s1, const char *s2) {
	int c1, c2;

	do {
		c1 = tolower(*s1++);
		c2 = tolower(*s2++);
	} while (c1 && c1 == c2);

	return c1 - c2;
}

// =====================================================================
//  Implementation of PluginList
// =====================================================================

PluginList::PluginList() = default;


FREE_IMAGE_FORMAT
PluginList::AddNode(FI_InitProc init_proc, void *instance, const char *format, const char *description, const char *extension, const char *regexpr) {
	if (init_proc) {
		try {
			const int curr_id = static_cast<int>(m_plugins.size());

			auto node = std::make_unique<PluginNode>();
			node->m_plugin = std::make_unique<Plugin>();
			init_proc(node->m_plugin.get(), curr_id);

			// get the format string (two possible ways)

			const char* the_format{};
			if (format) {
				the_format = format;
			}
			else if (node->m_plugin->format_proc) {
				the_format = node->m_plugin->format_proc();
			}

			// add the node if it wasn't there already

			if (the_format) {
				// fill-in the plugin structure
				node->m_id = curr_id;
				node->m_instance = instance;
				node->m_format = format;
				node->m_description = description;
				node->m_extension = extension;
				node->m_regexpr = regexpr;
				node->m_enabled = true;

				m_plugins.emplace_back(std::move(node));
				return static_cast<FREE_IMAGE_FORMAT>(curr_id);
			}
		}
		catch (...) { };
	}
	return FIF_UNKNOWN;
}

void
PluginList::SkipNode(size_t count) {
	for (size_t i = 0; i < count; ++i) {
		m_plugins.emplace_back(nullptr);
	}
}

PluginNode *
PluginList::FindNodeFromFormat(const char *format) {
	if (!format) {
		return nullptr;
	}

	for (auto& node : m_plugins) {
		if (!node) {
			continue;
		}

		const char* the_format = node->m_format;
		if (!the_format) {
			if (!node->m_plugin->format_proc) {
				continue;
			}
			the_format = node->m_plugin->format_proc();
		}

		if (node->m_enabled) {
			if (FreeImage_stricmp(the_format, format) == 0) {
				return node.get();
			}
		}
	}

	return nullptr;
}

PluginNode *
PluginList::FindNodeFromMime(const char *mime) {
	if (!mime) {
		return nullptr;
	}

	for (auto& node : m_plugins) {
		if (!node) {
			continue;
		}
		const char *the_mime = (node->m_plugin->mime_proc) ? node->m_plugin->mime_proc() : "";

		if (node->m_enabled) {
			if (strcmp(the_mime, mime) == 0) {
				return node.get();
			}
		}
	}

	return nullptr;
}

PluginNode *
PluginList::FindNodeFromFIF(int node_id) {
	if (node_id < 0 || node_id >= m_plugins.size()) {
		return nullptr;
	}

	auto& node = m_plugins[node_id];
	if (node && node->m_enabled) {
		return node.get();
	}

	return nullptr;
}

int
PluginList::Size() const {
	return static_cast<int>(m_plugins.size());
}

bool
PluginList::IsEmpty() const {
	return m_plugins.empty();
}

PluginList::~PluginList() {
#ifdef _WIN32
	for (auto& node : m_plugins) {
		if (node && node->m_instance) {
			FreeLibrary((HINSTANCE)node->m_instance);
		}
	}
#endif
}

// =====================================================================
// Retrieve a pointer to the plugin list container
// =====================================================================

PluginList * DLL_CALLCONV
FreeImage_GetPluginList() {
	return s_plugins.get();
}

// =====================================================================
// Plugin System Initialization
// =====================================================================

void DLL_CALLCONV
FreeImage_Initialise(FIBOOL load_local_plugins_only) {
	if (s_plugin_reference_count++ == 0) {
		
		/*
		Note: initialize all singletons here 
		in order to avoid race conditions with multi-threading
		*/

		// initialise the TagLib singleton
		const TagLib& s = TagLib::instance();

		// internal plugin initialization

		s_plugins.reset(new(std::nothrow) PluginList);

		if (s_plugins) {
			/* NOTE : 
			The order used to initialize internal plugins below MUST BE the same order 
			as the one used to define the FREE_IMAGE_FORMAT enum. 
			*/
			s_plugins->AddNode(InitBMP);
			s_plugins->AddNode(InitICO);
#if FREEIMAGE_WITH_LIBJPEG
			s_plugins->AddNode(InitJPEG);
#else
			s_plugins->SkipNode();
#endif
			s_plugins->AddNode(InitJNG);
			s_plugins->AddNode(InitKOALA);
			s_plugins->AddNode(InitIFF);
			s_plugins->AddNode(InitMNG);
			s_plugins->AddNode(InitPNM, nullptr, "PBM", "Portable Bitmap (ASCII)", "pbm", "^P1");
			s_plugins->AddNode(InitPNM, nullptr, "PBMRAW", "Portable Bitmap (RAW)", "pbm", "^P4");
			s_plugins->AddNode(InitPCD);
			s_plugins->AddNode(InitPCX);
			s_plugins->AddNode(InitPNM, nullptr, "PGM", "Portable Greymap (ASCII)", "pgm", "^P2");
			s_plugins->AddNode(InitPNM, nullptr, "PGMRAW", "Portable Greymap (RAW)", "pgm", "^P5");
#if FREEIMAGE_WITH_LIBPNG
			s_plugins->AddNode(InitPNG);
#else
			s_plugins->SkipNode();
#endif
			s_plugins->AddNode(InitPNM, nullptr, "PPM", "Portable Pixelmap (ASCII)", "ppm", "^P3");
			s_plugins->AddNode(InitPNM, nullptr, "PPMRAW", "Portable Pixelmap (RAW)", "ppm", "^P6");
			s_plugins->AddNode(InitRAS);
			s_plugins->AddNode(InitTARGA);
#if FREEIMAGE_WITH_LIBTIFF
			s_plugins->AddNode(InitTIFF);
#else
			s_plugins->SkipNode();
#endif
			s_plugins->AddNode(InitWBMP);
			s_plugins->AddNode(InitPSD);
			s_plugins->AddNode(InitCUT);
			s_plugins->AddNode(InitXBM);
			s_plugins->AddNode(InitXPM);
			s_plugins->AddNode(InitDDS);
	        s_plugins->AddNode(InitGIF);
	        s_plugins->AddNode(InitHDR);
#if FREEIMAGE_WITH_LIBTIFF
			s_plugins->AddNode(InitG3);
#else
			s_plugins->SkipNode();
#endif
			s_plugins->AddNode(InitSGI);
#if FREEIMAGE_WITH_LIBEXR
			s_plugins->AddNode(InitEXR);
#else
			s_plugins->SkipNode();
#endif
#if FREEIMAGE_WITH_LIBOPENJPEG
			s_plugins->AddNode(InitJ2K);
			s_plugins->AddNode(InitJP2);
#else
			s_plugins->SkipNode(2);
#endif
			s_plugins->AddNode(InitPFM);
			s_plugins->AddNode(InitPICT);
#if FREEIMAGE_WITH_LIBRAW
			s_plugins->AddNode(InitRAW);
#else
			s_plugins->SkipNode();
#endif
#if FREEIMAGE_WITH_LIBWEBP
			s_plugins->AddNode(InitWEBP);
#else
			s_plugins->SkipNode();
#endif
#if FREEIMAGE_WITH_LIBJXR
			s_plugins->AddNode(InitJXR);
#else
			s_plugins->SkipNode();
#endif
			// external plugin initialization
#ifdef _WIN32
			if (!load_local_plugins_only) {
				int count = 0;
				char buffer[MAX_PATH + 200];
				wchar_t current_dir[2 * _MAX_PATH], module[2 * _MAX_PATH];
				FIBOOL bOk = FALSE;

				// store the current directory. then set the directory to the application location

				if (GetCurrentDirectoryW(2 * _MAX_PATH, current_dir) != 0) {
					if (GetModuleFileNameW(nullptr, module, 2 * _MAX_PATH) != 0) {
						wchar_t *last_point = wcsrchr(module, L'\\');

						if (last_point) {
							*last_point = L'\0';

							bOk = SetCurrentDirectoryW(module);
						}
					}
				}

				// search for plugins

				while (count < std::size(s_search_list)) {
					_finddata_t find_data;
					long find_handle;

					strcpy(buffer, s_search_list[count]);
					strcat(buffer, "*.fip");

					if ((find_handle = (long)_findfirst(buffer, &find_data)) != -1L) {
						do {
							strcpy(buffer, s_search_list[count]);
							strncat(buffer, find_data.name, MAX_PATH + 200);

							if (auto instance = LoadLibrary(buffer)) {
								FARPROC proc_address = GetProcAddress(instance, "_Init@8");

								if (!proc_address || FIF_UNKNOWN == s_plugins->AddNode((FI_InitProc)proc_address, (void *)instance)) {
									FreeLibrary(instance);
								}
							}
						} while (_findnext(find_handle, &find_data) != -1L);

						_findclose(find_handle);
					}

					count++;
				}

				// restore the current directory

				if (bOk) {
					SetCurrentDirectoryW(current_dir);
				}
			}
#endif // _WIN32
		}
	}
}

void DLL_CALLCONV
FreeImage_DeInitialise() {
	--s_plugin_reference_count;

	if (s_plugin_reference_count == 0) {
		s_plugins.reset();
	}
}

// =====================================================================
// Open and close a bitmap
// =====================================================================

void * DLL_CALLCONV
FreeImage_Open(PluginNode *node, FreeImageIO *io, fi_handle handle, bool open_for_reading) {
	if (node->m_plugin->open_proc) {
       return node->m_plugin->open_proc(io, handle, static_cast<FIBOOL>(open_for_reading));
	}

	return nullptr;
}

void DLL_CALLCONV
FreeImage_Close(PluginNode *node, FreeImageIO *io, fi_handle handle, void *data) {
	if (node->m_plugin->close_proc) {
		node->m_plugin->close_proc(io, handle, data);
	}
}

// =====================================================================
// Plugin System Load/Save Functions
// =====================================================================

FIBITMAP * DLL_CALLCONV
FreeImage_LoadFromHandle(FREE_IMAGE_FORMAT fif, FreeImageIO *io, fi_handle handle, int flags) {
	FIBITMAP *bitmap{};

	if ((fif >= 0) && (fif < FreeImage_GetFIFCount())) {

		if (auto *node = s_plugins->FindNodeFromFIF(fif)) {
			if (node->m_plugin->load_proc) {
				void *data = FreeImage_Open(node, io, handle, true);
					
				bitmap = node->m_plugin->load_proc(io, handle, -1, flags, data);
					
				FreeImage_Close(node, io, handle, data);
			}
		}
	}
	return bitmap;
}

FIBITMAP * DLL_CALLCONV
FreeImage_Load(FREE_IMAGE_FORMAT fif, const char *filename, int flags) {
	FreeImageIO io;
	SetDefaultIO(&io);
	FIBITMAP *bitmap{};

	if (auto *handle = fopen(filename, "rb")) {
		bitmap = FreeImage_LoadFromHandle(fif, &io, (fi_handle)handle, flags);

		fclose(handle);
	} else {
		FreeImage_OutputMessageProc((int)fif, "FreeImage_Load: failed to open file %s", filename);
	}

	return bitmap;
}

FIBITMAP * DLL_CALLCONV
FreeImage_LoadU(FREE_IMAGE_FORMAT fif, const wchar_t *filename, int flags) {
	FreeImageIO io;
	SetDefaultIO(&io);
	FIBITMAP *bitmap{};
#ifdef _WIN32	
	if (auto *handle = _wfopen(filename, L"rb")) {
		bitmap = FreeImage_LoadFromHandle(fif, &io, (fi_handle)handle, flags);

		fclose(handle);
	} else {
		FreeImage_OutputMessageProc((int)fif, "FreeImage_LoadU: failed to open input file");
	}
#endif
	return bitmap;
}

FIBOOL DLL_CALLCONV
FreeImage_SaveToHandle(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, FreeImageIO *io, fi_handle handle, int flags) {
	// cannot save "header only" formats
	FIBOOL result{FALSE};
	if (!FreeImage_HasPixels(dib)) {
		FreeImage_OutputMessageProc((int)fif, "FreeImage_SaveToHandle: cannot save \"header only\" formats");
		return result;
	}

	if ((fif >= 0) && (fif < FreeImage_GetFIFCount())) {

		if (auto *node = s_plugins->FindNodeFromFIF(fif)) {
			if (node->m_plugin->save_proc) {
				void *data = FreeImage_Open(node, io, handle, false);
					
				result = node->m_plugin->save_proc(io, dib, handle, -1, flags, data);
					
				FreeImage_Close(node, io, handle, data);
			}
		}
	}

	return result;
}


FIBOOL DLL_CALLCONV
FreeImage_Save(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, const char *filename, int flags) {
	FreeImageIO io;
	SetDefaultIO(&io);
	FIBOOL success{FALSE};

	if (auto *handle = fopen(filename, "w+b")) {
		success = FreeImage_SaveToHandle(fif, dib, &io, (fi_handle)handle, flags);

		fclose(handle);
	} else {
		FreeImage_OutputMessageProc((int)fif, "FreeImage_Save: failed to open file %s", filename);
	}

	return success;
}

FIBOOL DLL_CALLCONV
FreeImage_SaveU(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, const wchar_t *filename, int flags) {
	FreeImageIO io;
	SetDefaultIO(&io);
	FIBOOL success{FALSE};
#ifdef _WIN32	
	if (auto *handle = _wfopen(filename, L"w+b")) {
		success = FreeImage_SaveToHandle(fif, dib, &io, (fi_handle)handle, flags);

		fclose(handle);
	} else {
		FreeImage_OutputMessageProc((int)fif, "FreeImage_SaveU: failed to open output file");
	}
#endif
	return success;
}

// =====================================================================
// Plugin construction + enable/disable functions
// =====================================================================

FREE_IMAGE_FORMAT DLL_CALLCONV
FreeImage_RegisterLocalPlugin(FI_InitProc proc_address, const char *format, const char *description, const char *extension, const char *regexpr) {
	return s_plugins->AddNode(proc_address, nullptr, format, description, extension, regexpr);
}

#ifdef _WIN32
FREE_IMAGE_FORMAT DLL_CALLCONV
FreeImage_RegisterExternalPlugin(const char *path, const char *format, const char *description, const char *extension, const char *regexpr) {
	if (path) {

		if (auto instance = LoadLibrary(path)) {
			FARPROC proc_address = GetProcAddress(instance, "_Init@8");

			if (auto result = s_plugins->AddNode((FI_InitProc)proc_address, (void *)instance, format, description, extension, regexpr); FIF_UNKNOWN != result) {
				return result;
			}

			FreeLibrary(instance);
		}
	}

	return FIF_UNKNOWN;
}
#endif // _WIN32

int DLL_CALLCONV
FreeImage_SetPluginEnabled(FREE_IMAGE_FORMAT fif, FIBOOL enable) {
	if (s_plugins) {

		if (auto *node = s_plugins->FindNodeFromFIF(fif)) {
			FIBOOL previous_state = node->m_enabled;
			node->m_enabled = enable;
			return previous_state;
		}
	}

	return -1;
}

int DLL_CALLCONV
FreeImage_IsPluginEnabled(FREE_IMAGE_FORMAT fif) {
	if (s_plugins) {
		auto *node = s_plugins->FindNodeFromFIF(fif);
		return node ? node->m_enabled : FALSE;
	}
	
	return -1;
}

// =====================================================================
// Plugin Access Functions
// =====================================================================

int DLL_CALLCONV
FreeImage_GetFIFCount() {
	return s_plugins ? s_plugins->Size() : 0;
}

FREE_IMAGE_FORMAT DLL_CALLCONV
FreeImage_GetFIFFromFormat(const char *format) {
	if (s_plugins) {
		PluginNode *node = s_plugins->FindNodeFromFormat(format);

		return node ? (FREE_IMAGE_FORMAT)node->m_id : FIF_UNKNOWN;
	}

	return FIF_UNKNOWN;
}

FREE_IMAGE_FORMAT DLL_CALLCONV
FreeImage_GetFIFFromMime(const char *mime) {
	if (s_plugins) {
		PluginNode *node = s_plugins->FindNodeFromMime(mime);

		return node ? (FREE_IMAGE_FORMAT)node->m_id : FIF_UNKNOWN;
	}

	return FIF_UNKNOWN;
}

const char * DLL_CALLCONV
FreeImage_GetFormatFromFIF(FREE_IMAGE_FORMAT fif) {
	if (s_plugins) {
		auto *node = s_plugins->FindNodeFromFIF(fif);

		return node ? (node->m_format ? node->m_format : node->m_plugin->format_proc()) : nullptr;
	}

	return nullptr;
}

const char * DLL_CALLCONV 
FreeImage_GetFIFMimeType(FREE_IMAGE_FORMAT fif) {
	if (s_plugins) {
		auto *node = s_plugins->FindNodeFromFIF(fif);

		return node ? (node->m_plugin ? ( node->m_plugin->mime_proc ? node->m_plugin->mime_proc() : nullptr) : nullptr) : nullptr;
	}

	return nullptr;
}

const char * DLL_CALLCONV
FreeImage_GetFIFExtensionList(FREE_IMAGE_FORMAT fif) {
	if (s_plugins) {
		auto *node = s_plugins->FindNodeFromFIF(fif);

		return node ? (node->m_extension ? node->m_extension : (node->m_plugin->extension_proc ? node->m_plugin->extension_proc() : nullptr)) : nullptr;
	}

	return nullptr;
}

const char * DLL_CALLCONV
FreeImage_GetFIFDescription(FREE_IMAGE_FORMAT fif) {
	if (s_plugins) {
		auto *node = s_plugins->FindNodeFromFIF(fif);

		return node ? (node->m_description ? node->m_description : (node->m_plugin->description_proc ? node->m_plugin->description_proc() : nullptr)) : nullptr;
	}

	return nullptr;
}

const char * DLL_CALLCONV
FreeImage_GetFIFRegExpr(FREE_IMAGE_FORMAT fif) {
	if (s_plugins) {
		auto *node = s_plugins->FindNodeFromFIF(fif);

		return node ? (node->m_regexpr ? node->m_regexpr : (node->m_plugin->regexpr_proc ? node->m_plugin->regexpr_proc() : nullptr)) : nullptr;
	}

	return nullptr;
}

FIBOOL DLL_CALLCONV
FreeImage_FIFSupportsReading(FREE_IMAGE_FORMAT fif) {
	if (s_plugins) {
		auto *node = s_plugins->FindNodeFromFIF(fif);

		return node ? node->m_plugin->load_proc != nullptr : FALSE;
	}

	return FALSE;
}

FIBOOL DLL_CALLCONV
FreeImage_FIFSupportsWriting(FREE_IMAGE_FORMAT fif) {
	if (s_plugins) {
		auto *node = s_plugins->FindNodeFromFIF(fif);

		return node ? node->m_plugin->save_proc != nullptr : FALSE ;
	}

	return FALSE;
}

FIBOOL DLL_CALLCONV
FreeImage_FIFSupportsExportBPP(FREE_IMAGE_FORMAT fif, int depth) {
	if (s_plugins) {
		auto *node = s_plugins->FindNodeFromFIF(fif);

		return node ? 
			(node->m_plugin->supports_export_bpp_proc ? 
				node->m_plugin->supports_export_bpp_proc(depth) : FALSE) : FALSE;
	}

	return FALSE;
}

FIBOOL DLL_CALLCONV
FreeImage_FIFSupportsExportType(FREE_IMAGE_FORMAT fif, FREE_IMAGE_TYPE type) {
	if (s_plugins) {
		auto *node = s_plugins->FindNodeFromFIF(fif);

		return node ? 
			(node->m_plugin->supports_export_type_proc ? 
				node->m_plugin->supports_export_type_proc(type) : FALSE) : FALSE;
	}

	return FALSE;
}

FIBOOL DLL_CALLCONV
FreeImage_FIFSupportsICCProfiles(FREE_IMAGE_FORMAT fif) {
	if (s_plugins) {
		auto *node = s_plugins->FindNodeFromFIF(fif);

		return node ? 
			(node->m_plugin->supports_icc_profiles_proc ? 
				node->m_plugin->supports_icc_profiles_proc() : FALSE) : FALSE;
	}

	return FALSE;
}

FIBOOL DLL_CALLCONV
FreeImage_FIFSupportsNoPixels(FREE_IMAGE_FORMAT fif) {
	if (s_plugins) {
		auto *node = s_plugins->FindNodeFromFIF(fif);

		return node ? 
			(node->m_plugin->supports_no_pixels_proc ? 
				node->m_plugin->supports_no_pixels_proc() : FALSE) : FALSE;
	}

	return FALSE;
}

FREE_IMAGE_FORMAT DLL_CALLCONV
FreeImage_GetFIFFromFilename(const char *filename) {
	if (filename) {
		const char *extension;

		// get the proper extension if we received a filename

		char *place = strrchr((char *)filename, '.');	
		extension = (place ? ++place : filename);

		// look for the extension in the plugin table

		for (int i = 0; i < FreeImage_GetFIFCount(); ++i) {

			auto* node = s_plugins->FindNodeFromFIF(i);
			if (node && node->m_enabled) {

				// compare the format id with the extension

				if (FreeImage_stricmp(FreeImage_GetFormatFromFIF((FREE_IMAGE_FORMAT)i), extension) == 0) {
					return (FREE_IMAGE_FORMAT)i;
				} else {
					// make a copy of the extension list and split it

					auto *copy = (char *)malloc(strlen(FreeImage_GetFIFExtensionList((FREE_IMAGE_FORMAT)i)) + 1);
					memset(copy, 0, strlen(FreeImage_GetFIFExtensionList((FREE_IMAGE_FORMAT)i)) + 1);
					memcpy(copy, FreeImage_GetFIFExtensionList((FREE_IMAGE_FORMAT)i), strlen(FreeImage_GetFIFExtensionList((FREE_IMAGE_FORMAT)i)));

					// get the first token

					char *token = strtok(copy, ",");

					while (token) {
						if (FreeImage_stricmp(token, extension) == 0) {
							free(copy);

							return (FREE_IMAGE_FORMAT)i;
						}

						token = strtok(nullptr, ",");
					}

					// free the copy of the extension list

					free(copy);
				}
			}
		}
	}

	return FIF_UNKNOWN;
}

FREE_IMAGE_FORMAT DLL_CALLCONV 
FreeImage_GetFIFFromFilenameU(const wchar_t *filename) {
#ifdef _WIN32	
	if (!filename) {
		return FIF_UNKNOWN;
	}
	// get the proper extension if we received a filename
	wchar_t *place = wcsrchr((wchar_t *)filename, '.');	
	if (!place) {
		return FIF_UNKNOWN;
	}
	// convert to single character - no national chars in extensions
	auto *extension = (char *)malloc(wcslen(place)+1);
	if (!extension) {
		return FIF_UNKNOWN;
	}
	unsigned int i=0;
	for (; i < wcslen(place); i++) // convert 16-bit to 8-bit
		extension[i] = (char)(place[i] & 0x00FF);
	// set terminating 0
	extension[i]=0;
	FREE_IMAGE_FORMAT fRet = FreeImage_GetFIFFromFilename(extension);
	free(extension);

	return fRet;
#else
	return FIF_UNKNOWN;
#endif // _WIN32
}

bool DLL_CALLCONV
FreeImage_ValidateFIF(FREE_IMAGE_FORMAT fif, FreeImageIO *io, fi_handle handle) {
	if (s_plugins) {
		FIBOOL validated = FALSE;

		if (auto *node = s_plugins->FindNodeFromFIF(fif)) {
			long tell = io->tell_proc(handle);

			validated = node ? (node->m_enabled ? (node->m_plugin->validate_proc ? node->m_plugin->validate_proc(io, handle) : FALSE) : FALSE) : FALSE;

			io->seek_proc(handle, tell, SEEK_SET);
		}

		return validated;
	}

	return FALSE;
}
