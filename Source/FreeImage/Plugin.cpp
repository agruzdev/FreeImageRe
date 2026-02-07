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
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
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
// Plugin search list
// =====================================================================

const char *
s_search_list[] = {
	"",
	"plugins\\",
};


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
//  Implementation of Plugin
// =====================================================================


PluginNodeBase::~PluginNodeBase() {
#ifdef _WIN32
	if (mInstance) {
		FreeLibrary((HINSTANCE)mInstance);
	}
#endif
}


class PluginNodeV1
	: public PluginNodeBase
{
public:
	PluginNodeV1(FI_InitProc func, void* ctx, int id, void* instance, const char* format, const char* description, const char* extension, const char* regexpr) 
		: PluginNodeBase(instance, format, description, extension, regexpr)
	{
		func(mPlugin.get(), id);
	}

	~PluginNodeV1() override = default;

private:
	const char* DoGetFormat() const override {
		if (mPlugin->format_proc) {
			return mPlugin->format_proc();
		}
		return nullptr;
	}

	const char* DoGetDescription() const override {
		if (mPlugin->description_proc) {
			return mPlugin->description_proc();
		}
		return nullptr;
	}

	const char* DoGetExtension() const override {
		if (mPlugin->extension_proc) {
			return mPlugin->extension_proc();
		}
		return nullptr;
	}

	const char* DoGetRegexpr() const override {
		if (mPlugin->regexpr_proc) {
			return mPlugin->regexpr_proc();
		}
		return nullptr;
	}

	const char* DoGetMime() const override {
		if (mPlugin->mime_proc) {
			return mPlugin->mime_proc();
		}
		return nullptr;
	}

	void* DoOpen(FreeImageIO* io, fi_handle handle, bool open_for_reading) override {
		if (mPlugin->open_proc) {
			return mPlugin->open_proc(io, handle, static_cast<FIBOOL>(open_for_reading));
		}
		return nullptr;
	}

	void DoClose(FreeImageIO* io, fi_handle handle, void* data) override {
		if (mPlugin->close_proc) {
			mPlugin->close_proc(io, handle, data);
		}
	}

	bool DoValidate(FreeImageIO* io, fi_handle handle) const override {
		if (mPlugin->validate_proc) {
			return mPlugin->validate_proc(io, handle);
		}
		return false;
	}

	FIBITMAP* DoLoad(FreeImageIO* io, fi_handle handle, int page, int flags, void* data) override {
		if (mPlugin->load_proc) {
			return mPlugin->load_proc(io, handle, page, flags, data);
		}
		return nullptr;
	}

	bool DoSave(FIBITMAP* dib, FreeImageIO* io, fi_handle handle, int page, int flags, void* data) override {
		if (mPlugin->save_proc) {
			return mPlugin->save_proc(io, dib, handle, page, flags, data);
		}
		return false;
	}

	int DoGetPageCount(FreeImageIO* io, fi_handle handle, void* data) override {
		if (mPlugin->pagecount_proc) {
			return mPlugin->pagecount_proc(io, handle, data);
		}
		return 1;
	}

	bool DoSupportsLoad() const override {
		return (mPlugin->load_proc != nullptr);
	}

	bool DoSupportsSave() const override {
		return (mPlugin->save_proc != nullptr);
	}

	bool DoSupportsExportBpp(int depth) const override {
		if (mPlugin->supports_export_bpp_proc) {
			return mPlugin->supports_export_bpp_proc(depth);
		}
		return false;
	}

	bool DoSupportsExportType(FREE_IMAGE_TYPE type) const override {
		if (mPlugin->supports_export_type_proc) {
			return mPlugin->supports_export_type_proc(type);
		}
		return false;
	}

	bool DoSupportsIccProfiles() const override {
		if (mPlugin->supports_icc_profiles_proc) {
			return mPlugin->supports_icc_profiles_proc();
		}
		return false;
	}

	bool DoSupportsNoPixels() const override {
		if (mPlugin->supports_no_pixels_proc) {
			return mPlugin->supports_no_pixels_proc();
		}
		return false;
	}


	/** The actual plugin, holding the function pointers */
	std::unique_ptr<Plugin> mPlugin = std::make_unique<Plugin>();
};

class PluginNodeV2
	: public PluginNodeBase
{
public:
	
	PluginNodeV2(FI_InitProc2 func, void* ctx, int /*id*/, void* instance, const char* format, const char* description, const char* extension, const char* regexpr)
		: PluginNodeBase(instance, format, description, extension, regexpr), mContext(ctx)
	{
		if (!func(mPlugin.get(), ctx)) {
			throw std::runtime_error("Failed to init plugin");
		}
	}

	~PluginNodeV2() override {
		if (mPlugin->release_proc) {
			mPlugin->release_proc(mContext);
		}
	}

private:
	const char* DoGetFormat() const override {
		if (mPlugin->format_proc) {
			return mPlugin->format_proc(mContext);
		}
		return nullptr;
	}

	const char* DoGetDescription() const override {
		if (mPlugin->description_proc) {
			return mPlugin->description_proc(mContext);
		}
		return nullptr;
	}

	const char* DoGetExtension() const override {
		if (mPlugin->extension_proc) {
			return mPlugin->extension_proc(mContext);
		}
		return nullptr;
	}

	const char* DoGetRegexpr() const override {
		if (mPlugin->regexpr_proc) {
			return mPlugin->regexpr_proc(mContext);
		}
		return nullptr;
	}

	const char* DoGetMime() const override {
		if (mPlugin->mime_proc) {
			return mPlugin->mime_proc(mContext);
		}
		return nullptr;
	}

	void* DoOpen(FreeImageIO* io, fi_handle handle, bool open_for_reading) override {
		if (mPlugin->open_proc) {
			return mPlugin->open_proc(mContext, io, handle, static_cast<FIBOOL>(open_for_reading));
		}
		return nullptr;
	}

	void DoClose(FreeImageIO* io, fi_handle handle, void* data) override {
		if (mPlugin->close_proc) {
			mPlugin->close_proc(mContext, io, handle, data);
		}
	}

	bool DoValidate(FreeImageIO* io, fi_handle handle) const override {
		if (mPlugin->validate_proc) {
			return mPlugin->validate_proc(mContext, io, handle);
		}
		return false;
	}

	FIBITMAP* DoLoad(FreeImageIO* io, fi_handle handle, int page, int flags, void* data) override {
		if (mPlugin->load_proc) {
			return mPlugin->load_proc(mContext, io, handle, page, flags, data);
		}
		return nullptr;
	}

	bool DoSave(FIBITMAP* dib, FreeImageIO* io, fi_handle handle, int page, int flags, void* data) override {
		if (mPlugin->save_proc) {
			return mPlugin->save_proc(mContext, io, dib, handle, page, flags, data);
		}
		return false;
	}

	int DoGetPageCount(FreeImageIO* io, fi_handle handle, void* data) override {
		if (mPlugin->pagecount_proc) {
			return mPlugin->pagecount_proc(mContext, io, handle, data);
		}
		return 1;
	}

	bool DoSupportsLoad() const override {
		return (mPlugin->load_proc != nullptr);
	}

	bool DoSupportsSave() const override {
		return (mPlugin->save_proc != nullptr);
	}

	bool DoSupportsExportBpp(int depth) const override {
		if (mPlugin->supports_export_bpp_proc) {
			return mPlugin->supports_export_bpp_proc(mContext, depth);
		}
		return false;
	}

	bool DoSupportsExportType(FREE_IMAGE_TYPE type) const override {
		if (mPlugin->supports_export_type_proc) {
			return mPlugin->supports_export_type_proc(mContext, type);
		}
		return false;
	}

	bool DoSupportsIccProfiles() const override {
		if (mPlugin->supports_icc_profiles_proc) {
			return mPlugin->supports_icc_profiles_proc(mContext);
		}
		return false;
	}

	bool DoSupportsNoPixels() const override {
		if (mPlugin->supports_no_pixels_proc) {
			return mPlugin->supports_no_pixels_proc(mContext);
		}
		return false;
	}

private:
	/** The actual plugin, holding the function pointers */
	void* mContext = nullptr;
	std::unique_ptr<Plugin2> mPlugin = std::make_unique<Plugin2>();
};


PluginsRegistry::PluginsRegistry()
{
	// internal plugins initialization
	Put(FIF_BMP, InitBMP);
	Put(FIF_ICO, InitICO);
#if FREEIMAGE_WITH_LIBJPEG
	Put(FIF_JPEG, InitJPEG);
#endif
	Put(FIF_JNG, InitJNG);
	Put(FIF_KOALA, InitKOALA);
	Put(FIF_IFF, InitIFF);
	Put(FIF_MNG, InitMNG);
	Put(FIF_PBM, InitPNM, false, nullptr, "PBM", "Portable Bitmap (ASCII)", "pbm", "^P1");
	Put(FIF_PBMRAW, InitPNM, false, nullptr, "PBMRAW", "Portable Bitmap (RAW)", "pbm", "^P4");
	Put(FIF_PCD, InitPCD);
	Put(FIF_PCX, InitPCX);
	Put(FIF_PGM, InitPNM, false, nullptr, "PGM", "Portable Greymap (ASCII)", "pgm", "^P2");
	Put(FIF_PGMRAW, InitPNM, false, nullptr, "PGMRAW", "Portable Greymap (RAW)", "pgm", "^P5");
#if FREEIMAGE_WITH_LIBPNG
	Put(FIF_PNG, InitPNG);
#endif
	Put(FIF_PPM, InitPNM, false, nullptr, "PPM", "Portable Pixelmap (ASCII)", "ppm", "^P3");
	Put(FIF_PPMRAW, InitPNM, false, nullptr, "PPMRAW", "Portable Pixelmap (RAW)", "ppm", "^P6");
	Put(FIF_RAS, InitRAS);
	Put(FIF_TARGA, InitTARGA);
#if FREEIMAGE_WITH_LIBTIFF
	Put(FIF_TIFF, InitTIFF);
#endif
	Put(FIF_WBMP, InitWBMP);
	Put(FIF_PSD, InitPSD);
	Put(FIF_CUT, InitCUT);
	Put(FIF_XBM, InitXBM);
	Put(FIF_XPM, InitXPM);
	Put(FIF_DDS, InitDDS);
	Put(FIF_GIF, InitGIF);
	Put(FIF_HDR, InitHDR);
#if FREEIMAGE_WITH_LIBTIFF
	Put(FIF_FAXG3, InitG3);
#endif
	Put(FIF_SGI, InitSGI);
#if FREEIMAGE_WITH_LIBOPENEXR
	Put(FIF_EXR, InitEXR);
#endif
#if FREEIMAGE_WITH_LIBOPENJPEG
	Put(FIF_J2K, InitJ2K);
	Put(FIF_JP2, InitJP2);
#endif
	Put(FIF_PFM, InitPFM);
	Put(FIF_PICT, InitPICT);
#if FREEIMAGE_WITH_LIBRAW
	Put(FIF_RAW, InitRAW);
#endif
#if FREEIMAGE_WITH_LIBWEBP
	Put(FIF_WEBP, InitWEBP);
#endif
#if FREEIMAGE_WITH_LIBJXR
	Put(FIF_JXR, InitJXR);
#endif
#if FREEIMAGE_WITH_LIBHEIF
	Put(FIF_HEIF, CreatePluginHEIF());
	Put(FIF_AVIF, CreatePluginAVIF());
#endif
#if FREEIMAGE_WITH_LIBJPEGXL
	Put(FIF_JPEGXL, CreatePluginJPEGXL());
#endif

	mNextId = FIF_JXR + 1;
}

PluginsRegistry::~PluginsRegistry() = default;


template <typename PluginType_, typename InitFunc_>
bool PluginsRegistry::ResetImpl(FREE_IMAGE_FORMAT fif, InitFunc_ init_proc, void* ctx, bool force, void* instance, const char* format, const char* description, const char* extension, const char* regexpr)
{
	if (init_proc == nullptr) {
		// clear plugin node
		return (mPlugins.erase(fif) > 0);
	}

	// instert or assign new
	std::unique_ptr<PluginNodeBase>& dst_node = mPlugins[fif];
	if (dst_node && !force) {
		// already in use
		return false;
	}

	std::unique_ptr<PluginNodeBase> new_node{ nullptr };
	try {
		new_node = std::make_unique<PluginType_>(init_proc, ctx, fif, instance, format, description, extension, regexpr);
	}
	catch (...) {
		// ToDo: report error here
		return false;
	}

	dst_node.swap(new_node);
	return true;
}

bool PluginsRegistry::Put(FREE_IMAGE_FORMAT fif, FI_InitProc init_proc, bool force, void* instance, const char* format, const char* description, const char* extension, const char* regexpr)
{
	return ResetImpl<PluginNodeV1>(fif, init_proc, /* ctx = */ nullptr, force, instance, format, description, extension, regexpr);
}

bool PluginsRegistry::Put(FREE_IMAGE_FORMAT fif, FI_InitProc2 init_proc, void* ctx, bool force, void* instance)
{
	return ResetImpl<PluginNodeV2>(fif, init_proc, ctx, force, instance);
}

bool PluginsRegistry::Put(FREE_IMAGE_FORMAT fif, std::unique_ptr<fi::Plugin2> plugin)
{
	if (!plugin) {
		return ResetImpl<PluginNodeV2>(fif, nullptr, nullptr, false, nullptr);
	}
	auto wrapper = std::make_unique<fi::details::Plugin2Wrapper>(std::move(plugin));
	const auto success = ResetImpl<PluginNodeV2>(fif, &fi::details::Plugin2Wrapper::InitProc, wrapper.get(), false, nullptr);
	if (success) {
		wrapper.release();
	}
	return success;
}

FREE_IMAGE_FORMAT PluginsRegistry::Append(FI_InitProc init_proc, void* instance, const char* format, const char* description, const char* extension, const char* regexpr)
{
	if (mNextId >= FIF_MAX_USER_ID) {
		return FIF_UNKNOWN;
	}
	FREE_IMAGE_FORMAT curr_id = static_cast<FREE_IMAGE_FORMAT>(mNextId);
	if (ResetImpl<PluginNodeV1>(curr_id, init_proc, /* ctx = */nullptr, /* force = */ false, instance, format, description, extension, regexpr)) {
		++mNextId;
		return curr_id;
	}
	return FIF_UNKNOWN;
}

FREE_IMAGE_FORMAT PluginsRegistry::Append(FI_InitProc2 init_proc, void* ctx, void* instance)
{
	if (mNextId >= FIF_MAX_USER_ID) {
		return FIF_UNKNOWN;
	}
	FREE_IMAGE_FORMAT curr_id = static_cast<FREE_IMAGE_FORMAT>(mNextId);
	if (ResetImpl<PluginNodeV2>(curr_id, init_proc, ctx, /* force = */ false, instance)) {
		++mNextId;
		return curr_id;
	}
	return FIF_UNKNOWN;
}


std::tuple<FREE_IMAGE_FORMAT, PluginNodeBase*> PluginsRegistry::FindFromFormat(const char* format) const
{
	if (!format) {
		return std::make_tuple(FIF_UNKNOWN, nullptr);
	}

	for (auto& [fif, node] : mPlugins) {
		if (!node || !node->IsEnabled()) {
			continue;
		}

		const char* plugin_format = node->GetDescription();
		if (!plugin_format) {
			continue;
		}

		if (FreeImage_stricmp(plugin_format, format) == 0) {
			return std::make_tuple(fif, node.get());
		}
	}

	return std::make_tuple(FIF_UNKNOWN, nullptr);
}

std::tuple<FREE_IMAGE_FORMAT, PluginNodeBase*> PluginsRegistry::FindFromMime(const char* mime) const
{
	if (!mime) {
		return std::make_tuple(FIF_UNKNOWN, nullptr);
	}

	for (auto& [fif, node] : mPlugins) {
		if (!node || !node->IsEnabled()) {
			continue;
		}

		const char* plugin_mime = node->GetMime();
		if (!plugin_mime) {
			continue;
		}

		if (strcmp(plugin_mime, mime) == 0) {
			return std::make_tuple(fif, node.get());
		}
	}

	return std::make_tuple(FIF_UNKNOWN, nullptr);
}

PluginNodeBase* PluginsRegistry::FindFromFIF(FREE_IMAGE_FORMAT fif) const
{
	auto it = mPlugins.find(fif);
	if (it != mPlugins.cend()) {
		auto& node = (*it).second;
		if (node && node->IsEnabled()) {
			return node.get();
		}
	}
	return nullptr;
}


bool PluginsRegistry::ValidateFIF(FREE_IMAGE_FORMAT fif, FreeImageIO* io, fi_handle handle) const
{
	bool status = false;
	if (PluginNodeBase* node = FindFromFIF(fif)) {
		if (node->IsEnabled()) {
			status = node->Validate(io, handle);
		}
	}
	return status;
}



PluginsRegistrySingleton::PluginsRegistrySingleton() = default;

PluginsRegistrySingleton::~PluginsRegistrySingleton() = default;

PluginsRegistrySingleton& PluginsRegistrySingleton::Instance()
{
	static PluginsRegistrySingleton instance;
	return instance;
}

bool PluginsRegistrySingleton::AddRef()
{
	bool firstRef = false;
	if ((mRefCounter < std::numeric_limits<uint32_t>::max()) && (mRefCounter++ == 0)) {
		mInstance.reset(new PluginsRegistry());
		firstRef = true;
	}
	return firstRef;
}

bool PluginsRegistrySingleton::DecRef()
{
	bool lastRef = false;
	if ((mRefCounter > 0) && (--mRefCounter == 0)) {
		mInstance.reset();
		lastRef = true;
	}
	return lastRef;
}

// =====================================================================
// Plugin System Initialization
// =====================================================================

void DLL_CALLCONV
FreeImage_Initialise(FIBOOL load_local_plugins_only) {
	// initialise the TagLib singleton
	const TagLib& s = TagLib::instance();

	auto& plugins = PluginsRegistrySingleton::Instance();
	if (plugins.AddRef()) {
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

							if (!proc_address || FIF_UNKNOWN == plugins->Append((FI_InitProc)proc_address, (void *)instance)) {
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

void DLL_CALLCONV
FreeImage_DeInitialise() {
	PluginsRegistrySingleton::Instance().DecRef();
}


// =====================================================================
// Plugin System Load/Save Functions
// =====================================================================

FIBITMAP * DLL_CALLCONV
FreeImage_LoadFromHandle(FREE_IMAGE_FORMAT fif, FreeImageIO *io, fi_handle handle, int flags) {
	FIBITMAP *bitmap{};
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		if (auto* node = plugins->FindFromFIF(fif)) {
			bitmap = node->Load(io, handle, -1, flags);
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

	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		if (auto* node = plugins->FindFromFIF(fif)) {
			result = node->Save(dib, io, handle, -1, flags);
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
FreeImage_RegisterLocalPlugin2(FI_InitProc2 proc_address, void* ctx)
{
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		return plugins->Append(proc_address, ctx);
	}
	return FIF_UNKNOWN;
}

FIBOOL DLL_CALLCONV
FreeImage_ResetLocalPlugin2(FREE_IMAGE_FORMAT fif, FI_InitProc2 proc_address, void* ctx, FIBOOL force)
{
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		return plugins->Put(fif, proc_address, ctx, force);
	}
	return FALSE;
}

FREE_IMAGE_FORMAT DLL_CALLCONV
FreeImage_RegisterLocalPlugin(FI_InitProc proc_address, const char *format, const char *description, const char *extension, const char *regexpr) {
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		return plugins->Append(proc_address, nullptr, format, description, extension, regexpr);
	}
	return FIF_UNKNOWN;
}

#ifdef _WIN32
FREE_IMAGE_FORMAT DLL_CALLCONV
FreeImage_RegisterExternalPlugin(const char *path, const char *format, const char *description, const char *extension, const char *regexpr) {
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		if (path) {
			if (auto instance = LoadLibrary(path)) {
				FARPROC proc_address = GetProcAddress(instance, "_Init@8");

				if (auto result = plugins->Append((FI_InitProc)proc_address, (void*)instance, format, description, extension, regexpr); FIF_UNKNOWN != result) {
					return result;
				}

				FreeLibrary(instance);
			}
		}
	}
	return FIF_UNKNOWN;
}
#endif // _WIN32

int DLL_CALLCONV
FreeImage_SetPluginEnabled(FREE_IMAGE_FORMAT fif, FIBOOL enable) {
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		if (auto *node = plugins->FindFromFIF(fif)) {
			return node->SetEnabled(enable);
		}
	}
	return -1;
}

int DLL_CALLCONV
FreeImage_IsPluginEnabled(FREE_IMAGE_FORMAT fif) {
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		auto *node = plugins->FindFromFIF(fif);
		return node ? node->IsEnabled() : FALSE;
	}	
	return -1;
}

// =====================================================================
// Plugin Access Functions
// =====================================================================


int DLL_CALLCONV
FreeImage_GetFIFCount2() {
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		return static_cast<int>(plugins->GetFifCount2());
	}
	return 0;
}

FREE_IMAGE_FORMAT DLL_CALLCONV
FreeImage_GetFIFFromIndex(int idx) {
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		return plugins->GetFifFromIndex(idx);
	}
	return FIF_UNKNOWN;
}

int DLL_CALLCONV
FreeImage_GetFIFCount() {
	// Legacy behavior: FIF_JXR + registered plugins count
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		return static_cast<int>(plugins->GetNextFif());
	}
	return 0;
}

FREE_IMAGE_FORMAT DLL_CALLCONV
FreeImage_GetFIFFromFormat(const char *format) {
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		return std::get< FREE_IMAGE_FORMAT>(plugins->FindFromFormat(format));
	}
	return FIF_UNKNOWN;
}

FREE_IMAGE_FORMAT DLL_CALLCONV
FreeImage_GetFIFFromMime(const char *mime) {
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		return std::get< FREE_IMAGE_FORMAT>(plugins->FindFromMime(mime));
	}
	return FIF_UNKNOWN;
}

const char * DLL_CALLCONV
FreeImage_GetFormatFromFIF(FREE_IMAGE_FORMAT fif) {
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		auto node = plugins->FindFromFIF(fif);
		return node ? node->GetFormat() : nullptr;
	}
	return nullptr;
}

const char * DLL_CALLCONV 
FreeImage_GetFIFMimeType(FREE_IMAGE_FORMAT fif) {
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		auto node = plugins->FindFromFIF(fif);
		return node ? node->GetMime() : nullptr;
	}
	return nullptr;
}

const char * DLL_CALLCONV
FreeImage_GetFIFExtensionList(FREE_IMAGE_FORMAT fif) {
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		auto node = plugins->FindFromFIF(fif);
		return node ? node->GetExtension() : nullptr;
	}
	return nullptr;
}

const char * DLL_CALLCONV
FreeImage_GetFIFDescription(FREE_IMAGE_FORMAT fif) {
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		auto node = plugins->FindFromFIF(fif);
		return node ? node->GetDescription() : nullptr;
	}
	return nullptr;
}

const char * DLL_CALLCONV
FreeImage_GetFIFRegExpr(FREE_IMAGE_FORMAT fif) {
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		auto node = plugins->FindFromFIF(fif);
		return node ? node->GetRegexpr() : nullptr;
	}
	return nullptr;
}

FIBOOL DLL_CALLCONV
FreeImage_FIFSupportsReading(FREE_IMAGE_FORMAT fif) {
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		auto node = plugins->FindFromFIF(fif);
		return node ? node->SupportsLoad() : FALSE;
	}
	return FALSE;
}

FIBOOL DLL_CALLCONV
FreeImage_FIFSupportsWriting(FREE_IMAGE_FORMAT fif) {
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		auto node = plugins->FindFromFIF(fif);
		return node ? node->SupportsSave() : FALSE;
	}
	return FALSE;
}

FIBOOL DLL_CALLCONV
FreeImage_FIFSupportsExportBPP(FREE_IMAGE_FORMAT fif, int depth) {
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		auto node = plugins->FindFromFIF(fif);
		return node ? node->SupportsExportBpp(depth) : FALSE;
	}
	return FALSE;
}

FIBOOL DLL_CALLCONV
FreeImage_FIFSupportsExportType(FREE_IMAGE_FORMAT fif, FREE_IMAGE_TYPE type) {
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		auto node = plugins->FindFromFIF(fif);
		return node ? node->SupportsExportType(type) : FALSE;
	}
	return FALSE;
}

FIBOOL DLL_CALLCONV
FreeImage_FIFSupportsICCProfiles(FREE_IMAGE_FORMAT fif) {
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		auto node = plugins->FindFromFIF(fif);
		return node ? node->SupportsIccProfiles() : FALSE;
	}
	return FALSE;
}

FIBOOL DLL_CALLCONV
FreeImage_FIFSupportsNoPixels(FREE_IMAGE_FORMAT fif) {
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		auto node = plugins->FindFromFIF(fif);
		return node ? node->SupportsNoPixels() : FALSE;
	}
	return FALSE;
}

FREE_IMAGE_FORMAT DLL_CALLCONV
FreeImage_GetFIFFromFilename(const char *filename) {
	if (!filename) {
		return FIF_UNKNOWN;
	}

	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		// get the proper extension if we received a filename
		char* place = strrchr((char*)filename, '.');
		const char* extension = (place ? ++place : filename);

		// look for the extension in the plugin table
		for (const auto& [fif, node] : plugins->NodesCRange()) {
			if (node && node->IsEnabled()) {
				// compare the format id with the extension
				if (FreeImage_stricmp(node->GetFormat(), extension) == 0) {
					return fif;
				}
				// make a copy of the extension list and split it
				if (const char* extension_list = node->GetExtension()) {
					auto copy = std::string(extension_list);
					// get the first token
					char* token = strtok(copy.data(), ",");
					while (token) {
						if (FreeImage_stricmp(token, extension) == 0) {
							return fif;
						}
						token = strtok(nullptr, ",");
					}
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
	for (; i < wcslen(place); i++) { // convert 16-bit to 8-bit
		extension[i] = (char)(place[i] & 0x00FF);
	}
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
	if (auto& plugins = PluginsRegistrySingleton::Instance()) {
		auto node = plugins->FindFromFIF(fif);
		return node ? node->Validate(io, handle) : FALSE;
	}
	return FALSE;
}
