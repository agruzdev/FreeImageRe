// ==========================================================
// FreeImage Plugin Interface
//
// Design and implementation by
// - Floris van den Berg (flvdberg@wxs.nl)
// - Rui Lopes (ruiglopes@yahoo.com)
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

#ifdef _MSC_VER 
#pragma warning (disable : 4786) // identifier was truncated to 'number' characters
#endif 

#ifndef PLUGIN_H
#define PLUGIN_H

#include <memory>
#include <unordered_map>
#include "FreeImage.h"
#include "Utilities.h"


// =====================================================================
//  Plugin Node
// =====================================================================

class PluginNodeBase
{
public:
	PluginNodeBase(void* instance = nullptr, const char* format = nullptr, const char* description = nullptr, const char* extension = nullptr, const char* regexpr = nullptr)
		: mInstance(instance), mFormat(format), mDescription(description), mExtension(extension), mRegexpr(regexpr)
	{ }

	PluginNodeBase(const PluginNodeBase&) = delete;
	PluginNodeBase(PluginNodeBase&&) = delete;

	virtual ~PluginNodeBase();

	PluginNodeBase& operator=(const PluginNodeBase&) = delete;
	PluginNodeBase& operator=(PluginNodeBase&&) = delete;


	bool IsEnabled() const
	{
		return mEnabled;
	}

	bool SetEnabled(bool value = true)
	{
		std::swap(mEnabled, value);
		return value;
	}

	void* Open(FreeImageIO* io, fi_handle handle, bool open_for_reading) {
		return DoOpen(io, handle, open_for_reading);
	}

	void Close(FreeImageIO* io, fi_handle handle, void* data) {
		return DoClose(io, handle, data);
	}

	bool Validate(FreeImageIO* io, fi_handle handle) const {
		const long tell = io->tell_proc(handle);
		const bool status = DoValidate(io, handle);
		io->seek_proc(handle, tell, SEEK_SET);
		return status;
	}

	FIBITMAP* Load(FreeImageIO* io, fi_handle handle, int page, int flags) {
		void* data = DoOpen(io, handle, true);
		auto bitmap = DoLoad(io, handle, page, flags, data);
		DoClose(io, handle, data);
		return bitmap;
	}

	// load from already opened io
	FIBITMAP* Load(FreeImageIO* io, fi_handle handle, int page, int flags, void* data) {
		return DoLoad(io, handle, page, flags, data);
	}

	bool Save(FIBITMAP* dib, FreeImageIO* io, fi_handle handle, int page, int flags) {
		void* data = DoOpen(io, handle, false);
		const bool result = DoSave(dib, io, handle, page, flags, data);
		DoClose(io, handle, data);
		return result;
	}

	// Save to already opened io
	bool Save(FIBITMAP* dib, FreeImageIO* io, fi_handle handle, int page, int flags, void* data) {
		return DoSave(dib, io, handle, page, flags, data);
	}

	int GetPageCount(FreeImageIO* io, fi_handle handle) {
		io->seek_proc(handle, 0, SEEK_SET);
		void* data = DoOpen(io, handle, true);
		const int page_count = DoGetPageCount(io, handle, data);
		DoClose(io, handle, data);
		return page_count;
	}

	const char* GetFormat() const {
		if (mFormat) {
			return mFormat;
		}
		return DoGetFormat();
	}

	const char* GetDescription() const {
		if (mDescription) {
			return mDescription;
		}
		return DoGetDescription();
	}

	const char* GetExtension() const {
		if (mExtension) {
			return mExtension;
		}
		return DoGetExtension();
	}

	const char* GetRegexpr() const {
		if (mRegexpr) {
			return mRegexpr;
		}
		return DoGetRegexpr();
	}

	const char* GetMime() const {
		return DoGetMime();
	}

	bool SupportsLoad() const {
		return DoSupportsLoad();
	}

	bool SupportsSave() const {
		return DoSupportsSave();
	}

	bool SupportsExportBpp(int depth) const {
		return DoSupportsExportBpp(depth);
	}

	bool SupportsExportType(FREE_IMAGE_TYPE type) const {
		return DoSupportsExportType(type);
	}

	bool SupportsIccProfiles() const {
		return DoSupportsIccProfiles();
	}

	bool SupportsNoPixels() const {
		return DoSupportsNoPixels();
	}

private:
	virtual void* DoOpen(FreeImageIO* io, fi_handle handle, bool open_for_reading) = 0;

	virtual void DoClose(FreeImageIO* io, fi_handle handle, void* data) = 0;

	virtual bool DoValidate(FreeImageIO* io, fi_handle handle) const = 0;

	virtual FIBITMAP* DoLoad(FreeImageIO* io, fi_handle handle, int page, int flags, void* data) = 0;

	virtual bool DoSave(FIBITMAP* dib, FreeImageIO* io, fi_handle handle, int page, int flags, void* data) = 0;

	virtual int DoGetPageCount(FreeImageIO* io, fi_handle handle, void* data) = 0;

	virtual const char* DoGetFormat() const = 0;

	virtual const char* DoGetDescription() const = 0;

	virtual const char* DoGetExtension() const = 0;

	virtual const char* DoGetRegexpr() const = 0;

	virtual const char* DoGetMime() const = 0;

	virtual bool DoSupportsLoad() const = 0;

	virtual bool DoSupportsSave() const = 0;

	virtual bool DoSupportsExportBpp(int depth) const = 0;

	virtual bool DoSupportsExportType(FREE_IMAGE_TYPE type) const = 0;

	virtual bool DoSupportsIccProfiles() const = 0;

	virtual bool DoSupportsNoPixels() const = 0;

private:
	/** Handle to a user plugin DLL (NULL for standard plugins) */
	void* mInstance{ nullptr };
	/** Enable/Disable switch */
	bool mEnabled{ true };

	/** Unique format string for the plugin */
	const char* mFormat{ nullptr };
	/** Description string for the plugin */
	const char* mDescription{ nullptr };
	/** Comma separated list of file extensions indicating what files this plugin can open */
	const char* mExtension{ nullptr };
	/** optional regular expression to help	software identifying a bitmap type */
	const char* mRegexpr{ nullptr };
};


// =====================================================================
//  Internal Plugin List
// =====================================================================


class PluginsRegistry
{
public:
	PluginsRegistry();
	~PluginsRegistry();

	PluginsRegistry(const PluginsRegistry&) = delete;
	PluginsRegistry(PluginsRegistry&&) = delete;

	PluginsRegistry& operator=(const PluginsRegistry&) = delete;
	PluginsRegistry& operator=(PluginsRegistry&&) = delete;
	
	bool Put(FREE_IMAGE_FORMAT fif, FI_InitProc init_proc, bool force = false, void* instance = nullptr, const char* format = 0, const char* description = 0, const char* extension = 0, const char* regexpr = 0);
	bool Put(FREE_IMAGE_FORMAT fif, FI_InitProc2 init_proc, void* ctx, bool force = false, void* instance = nullptr);

	FREE_IMAGE_FORMAT Append(FI_InitProc init_proc, void* instance = nullptr, const char* format = 0, const char* description = 0, const char* extension = 0, const char* regexpr = 0);
	FREE_IMAGE_FORMAT Append(FI_InitProc2 init_proc, void* ctx = nullptr, void* instance = nullptr);
	
	bool ValidateFIF(FREE_IMAGE_FORMAT fif, FreeImageIO* io, fi_handle handle) const;

	std::tuple<FREE_IMAGE_FORMAT, PluginNodeBase*> FindFromFormat(const char* format) const;

	std::tuple<FREE_IMAGE_FORMAT, PluginNodeBase*> FindFromMime(const char* mime) const;

	PluginNodeBase* FindFromFIF(FREE_IMAGE_FORMAT fif) const;

	size_t GetFifCount() const
	{
		return mNextId;
	}

private:
	template <typename PluginType_, typename InitFunc_>
	bool ResetImpl(FREE_IMAGE_FORMAT fif, InitFunc_ init_proc, void* ctx, bool force, void* instance,
		const char* format = nullptr, const char* description = nullptr, const char* extension = nullptr, const char* regexpr = nullptr);

	std::unordered_map<FREE_IMAGE_FORMAT, std::unique_ptr<PluginNodeBase>> mPlugins{ };
	int32_t mNextId = 0;
};



class PluginsRegistrySingleton
{
public:
	static PluginsRegistrySingleton& Instance();

	PluginsRegistrySingleton(const PluginsRegistrySingleton&) = delete;
	PluginsRegistrySingleton(PluginsRegistrySingleton&&) = delete;

	~PluginsRegistrySingleton();

	PluginsRegistrySingleton& operator=(const PluginsRegistrySingleton&) = delete;
	PluginsRegistrySingleton& operator=(PluginsRegistrySingleton&&) = delete;

	/**
	 * returns 'true' for the first reference
	 */
	bool AddRef();

	/**
	 * returns 'true' for the last reference
	 */
	bool DecRef();


	operator bool() const {
		return static_cast<bool>(mInstance);
	}

	PluginsRegistry* get() const {
		return mInstance.get();
	}

	PluginsRegistry* operator->() const {
		return mInstance.get();
	}

private:
	PluginsRegistrySingleton();

	std::unique_ptr<PluginsRegistry> mInstance{ nullptr };
	uint32_t mRefCounter{ 0 };
};



// ==========================================================
//   Plugin Initialisation Callback
// ==========================================================

void DLL_CALLCONV FreeImage_OutputMessage(int fif, const char *message, ...);

// =====================================================================
// Reimplementation of stricmp (it is not supported on some systems)
// =====================================================================

int FreeImage_stricmp(const char *s1, const char *s2);

// ==========================================================
//   Internal functions
// ==========================================================

bool DLL_CALLCONV FreeImage_ValidateFIF(FREE_IMAGE_FORMAT fif, FreeImageIO *io, fi_handle handle);

// ==========================================================
//   Internal plugins
// ==========================================================

void DLL_CALLCONV InitBMP(Plugin *plugin, int format_id);
void DLL_CALLCONV InitCUT(Plugin *plugin, int format_id);
void DLL_CALLCONV InitICO(Plugin *plugin, int format_id);
void DLL_CALLCONV InitIFF(Plugin *plugin, int format_id);
void DLL_CALLCONV InitJPEG(Plugin *plugin, int format_id);
void DLL_CALLCONV InitKOALA(Plugin *plugin, int format_id);
//void DLL_CALLCONV InitLBM(Plugin *plugin, int format_id);
void DLL_CALLCONV InitMNG(Plugin *plugin, int format_id);
void DLL_CALLCONV InitPCD(Plugin *plugin, int format_id);
void DLL_CALLCONV InitPCX(Plugin *plugin, int format_id);
void DLL_CALLCONV InitPNG(Plugin *plugin, int format_id);
void DLL_CALLCONV InitPNM(Plugin *plugin, int format_id);
void DLL_CALLCONV InitPSD(Plugin *plugin, int format_id);
void DLL_CALLCONV InitRAS(Plugin *plugin, int format_id);
void DLL_CALLCONV InitTARGA(Plugin *plugin, int format_id);
void DLL_CALLCONV InitTIFF(Plugin *plugin, int format_id);
void DLL_CALLCONV InitWBMP(Plugin *plugin, int format_id);
void DLL_CALLCONV InitXBM(Plugin *plugin, int format_id);
void DLL_CALLCONV InitXPM(Plugin *plugin, int format_id);
void DLL_CALLCONV InitDDS(Plugin *plugin, int format_id);
void DLL_CALLCONV InitGIF(Plugin *plugin, int format_id);
void DLL_CALLCONV InitHDR(Plugin *plugin, int format_id);
void DLL_CALLCONV InitG3(Plugin *plugin, int format_id);
void DLL_CALLCONV InitSGI(Plugin *plugin, int format_id);
void DLL_CALLCONV InitEXR(Plugin *plugin, int format_id);
void DLL_CALLCONV InitJ2K(Plugin *plugin, int format_id);
void DLL_CALLCONV InitJP2(Plugin *plugin, int format_id);
void DLL_CALLCONV InitPFM(Plugin *plugin, int format_id);
void DLL_CALLCONV InitPICT(Plugin *plugin, int format_id);
void DLL_CALLCONV InitRAW(Plugin *plugin, int format_id);
void DLL_CALLCONV InitJNG(Plugin *plugin, int format_id);
void DLL_CALLCONV InitWEBP(Plugin *plugin, int format_id);
void DLL_CALLCONV InitJXR(Plugin *plugin, int format_id);

#endif //!PLUGIN_H
