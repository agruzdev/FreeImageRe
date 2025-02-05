// ==========================================================
// HIEF and AVIF
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

#include "FreeImage.hpp"
#include "Utilities.h"
#include "Metadata/FreeImageTag.h"
#include "FreeImage/SimpleTools.h"

#include <array>
#include <cstring>
#include "yato/types.h"
#include "yato/finally.h"
#include <libheif/heif.h>

#ifdef _WIN32
# include <Windows.h>
# define LIBRARY_HANDLE_TYPE HMODULE
# define LOAD_LIBRARY(Path_) LoadLibrary(Path_)
# define FREE_LIBRARY(Handle_) FreeLibrary(Handle_)
# define LOAD_SYMBOL(Handle_, Symbol_) GetProcAddress(Handle_, Symbol_)
#else
# include <dlfcn.h>
# define LIBRARY_HANDLE_TYPE void*
# define LOAD_LIBRARY(Path_) dlopen(Path_, RTLD_NOW | RTLD_GLOBAL)
# define FREE_LIBRARY(Handle_) dlclose(Handle_)
# define LOAD_SYMBOL(Handle_, Symbol_) dlsym(Handle_, Symbol_)
#endif


class LibraryLoader
{
public:
    LibraryLoader(const std::vector<std::string>& names) {
        for (const auto& n : names) {
            mHandle = LOAD_LIBRARY(n.c_str());
            if (mHandle) {
                break;
            }
        }
        if (!mHandle) {
            throw std::runtime_error("Failed to open library.");
        }
    }

    LibraryLoader(const std::string& name) 
        : LibraryLoader(std::vector<std::string>{ name })
    { }

    LibraryLoader(const LibraryLoader&) = delete;
    LibraryLoader(LibraryLoader&&) = delete;

    virtual ~LibraryLoader() {
        FREE_LIBRARY(mHandle);
    }

    LibraryLoader& operator=(const LibraryLoader&) = delete;
    LibraryLoader& operator=(LibraryLoader&&) = delete;


    template <typename SymbolType_>
    SymbolType_ LoadSymbol(const char* name, bool required = true) {
        SymbolType_ sym = reinterpret_cast<SymbolType_>(LOAD_SYMBOL(mHandle, name));
        if (required && !sym) {
            throw std::runtime_error(std::string("Failed to load symbol '") + name + "'");
        }
        return sym;
    }

private:
    LIBRARY_HANDLE_TYPE mHandle{};
};


class LibHeif
    : public LibraryLoader
{
public:
    static
    LibHeif& GetInstance()
    {
        static LibHeif instance;
        return instance;
    }

    LibHeif(const LibHeif&) = delete;
    LibHeif(LibHeif&&) = delete;

    ~LibHeif() override {
        // FreeImage is usually released from global destructor, it migth lead to crash in linheif.
        //if (heif_deinit_f) {
        //    heif_deinit_f();
        //}
    }

    LibHeif& operator=(const LibHeif&) = delete;
    LibHeif& operator=(LibHeif&&) = delete;


    // lib symbols
    decltype(&::heif_init) heif_init_f{ nullptr };
    decltype(&::heif_deinit) heif_deinit_f{ nullptr };
    decltype(&::heif_get_version) heif_get_version_f{ nullptr };
    decltype(&::heif_get_version_number_major) heif_get_version_number_major_f{ nullptr };
    decltype(&::heif_get_version_number_minor) heif_get_version_number_minor_f{ nullptr };
    decltype(&::heif_get_version_number_maintenance) heif_get_version_number_maintenance_f{ nullptr };
    decltype(&::heif_read_main_brand) heif_read_main_brand_f{ nullptr };
    decltype(&::heif_context_alloc) heif_context_alloc_f{ nullptr };
    decltype(&::heif_context_free) heif_context_free_f{ nullptr };
    decltype(&::heif_context_read_from_reader) heif_context_read_from_reader_f{ nullptr };
    decltype(&::heif_context_write) heif_context_write_f{ nullptr };
    decltype(&::heif_context_get_primary_image_handle) heif_context_get_primary_image_handle_f{ nullptr };
    decltype(&::heif_context_get_encoder_for_format) heif_context_get_encoder_for_format_f{ nullptr };
    decltype(&::heif_context_encode_image) heif_context_encode_image_f{ nullptr };
    decltype(&::heif_image_handle_release) heif_image_handle_release_f{ nullptr };
    decltype(&::heif_image_handle_get_width) heif_image_handle_get_width_f{ nullptr };
    decltype(&::heif_image_handle_get_height) heif_image_handle_get_height_f{ nullptr };
    decltype(&::heif_image_handle_get_preferred_decoding_colorspace) heif_image_handle_get_preferred_decoding_colorspace_f{ nullptr };
    decltype(&::heif_image_handle_has_alpha_channel) heif_image_handle_has_alpha_channel_f{ nullptr };
    decltype(&::heif_image_handle_get_number_of_metadata_blocks) heif_image_handle_get_number_of_metadata_blocks_f{ nullptr };
    decltype(&::heif_image_handle_get_metadata_type) heif_image_handle_get_metadata_type_f{ nullptr };
    decltype(&::heif_image_handle_get_list_of_metadata_block_IDs) heif_image_handle_get_list_of_metadata_block_IDs_f{ nullptr };
    decltype(&::heif_image_handle_get_metadata_size) heif_image_handle_get_metadata_size_f{ nullptr };
    decltype(&::heif_image_handle_get_metadata) heif_image_handle_get_metadata_f{ nullptr };
    decltype(&::heif_decode_image) heif_decode_image_f{ nullptr };
    decltype(&::heif_image_create) heif_image_create_f{ nullptr };
    decltype(&::heif_image_release) heif_image_release_f{ nullptr };
    decltype(&::heif_image_add_plane) heif_image_add_plane_f{ nullptr };
    decltype(&::heif_image_get_bits_per_pixel) heif_image_get_bits_per_pixel_f{ nullptr };
    decltype(&::heif_image_get_plane_readonly) heif_image_get_plane_readonly_f{ nullptr };
    decltype(&::heif_image_get_plane) heif_image_get_plane_f{ nullptr };
    decltype(&::heif_encoder_release) heif_encoder_release_f{ nullptr };
    decltype(&::heif_encoder_set_lossy_quality) heif_encoder_set_lossy_quality_f{ nullptr };
    decltype(&::heif_image_handle_get_number_of_thumbnails) heif_image_handle_get_number_of_thumbnails_f{ nullptr };
    decltype(&::heif_image_handle_get_list_of_thumbnail_IDs) heif_image_handle_get_list_of_thumbnail_IDs_f{ nullptr };
    decltype(&::heif_image_handle_get_thumbnail) heif_image_handle_get_thumbnail_f{ nullptr };

private:
    LibHeif()
#ifdef _WIN32
        : LibraryLoader(std::vector<std::string>{ "heif.dll", "libheif.dll" })
#else
        : LibraryLoader("libheif.so")
#endif
    {
        heif_init_f = LoadSymbol<decltype(&::heif_init)>("heif_init", /*required=*/false);
        heif_deinit_f = LoadSymbol<decltype(&::heif_deinit)>("heif_deinit", /*required=*/false);
        heif_get_version_f = LoadSymbol<decltype(&::heif_get_version)>("heif_get_version");
        heif_get_version_number_major_f = LoadSymbol<decltype(&::heif_get_version_number_major)>("heif_get_version_number_major");
        heif_get_version_number_minor_f = LoadSymbol<decltype(&::heif_get_version_number_minor)>("heif_get_version_number_minor");
        heif_get_version_number_maintenance_f = LoadSymbol<decltype(&::heif_get_version_number_maintenance)>("heif_get_version_number_maintenance");
        heif_read_main_brand_f = LoadSymbol<decltype(&::heif_read_main_brand)>("heif_read_main_brand", /*required=*/false);
        heif_context_alloc_f = LoadSymbol<decltype(&::heif_context_alloc)>("heif_context_alloc");
        heif_context_free_f = LoadSymbol<decltype(&::heif_context_free)>("heif_context_free");
        heif_context_read_from_reader_f = LoadSymbol<decltype(&::heif_context_read_from_reader)>("heif_context_read_from_reader");
        heif_context_write_f = LoadSymbol<decltype(&::heif_context_write)>("heif_context_write");
        heif_context_get_primary_image_handle_f = LoadSymbol<decltype(&::heif_context_get_primary_image_handle)>("heif_context_get_primary_image_handle");
        heif_context_get_encoder_for_format_f = LoadSymbol<decltype(&::heif_context_get_encoder_for_format)>("heif_context_get_encoder_for_format");
        heif_image_handle_release_f = LoadSymbol<decltype(&::heif_image_handle_release)>("heif_image_handle_release");
        heif_image_handle_get_width_f = LoadSymbol<decltype(&::heif_image_handle_get_width)>("heif_image_handle_get_width");
        heif_image_handle_get_height_f = LoadSymbol<decltype(&::heif_image_handle_get_height)>("heif_image_handle_get_height");
        heif_image_handle_get_number_of_metadata_blocks_f = LoadSymbol<decltype(&::heif_image_handle_get_number_of_metadata_blocks)>("heif_image_handle_get_number_of_metadata_blocks");
        heif_image_handle_get_list_of_metadata_block_IDs_f = LoadSymbol<decltype(&::heif_image_handle_get_list_of_metadata_block_IDs)>("heif_image_handle_get_list_of_metadata_block_IDs");
        heif_image_handle_get_metadata_type_f = LoadSymbol<decltype(&::heif_image_handle_get_metadata_type)>("heif_image_handle_get_metadata_type");
        heif_image_handle_get_metadata_size_f = LoadSymbol<decltype(&::heif_image_handle_get_metadata_size)>("heif_image_handle_get_metadata_size");
        heif_image_handle_get_metadata_f = LoadSymbol<decltype(&::heif_image_handle_get_metadata)>("heif_image_handle_get_metadata");
        heif_image_handle_get_preferred_decoding_colorspace_f = LoadSymbol<decltype(&::heif_image_handle_get_preferred_decoding_colorspace)>("heif_image_handle_get_preferred_decoding_colorspace", /*required=*/false);
        heif_decode_image_f = LoadSymbol<decltype(&::heif_decode_image)>("heif_decode_image");
        heif_image_create_f = LoadSymbol<decltype(&::heif_image_create)>("heif_image_create");
        heif_image_release_f = LoadSymbol<decltype(&::heif_image_release)>("heif_image_release");
        heif_image_add_plane_f = LoadSymbol<decltype(&::heif_image_add_plane)>("heif_image_add_plane");
        heif_image_handle_has_alpha_channel_f = LoadSymbol<decltype(&::heif_image_handle_has_alpha_channel)>("heif_image_handle_has_alpha_channel");
        heif_image_get_bits_per_pixel_f = LoadSymbol<decltype(&::heif_image_get_bits_per_pixel)>("heif_image_get_bits_per_pixel");
        heif_image_get_plane_readonly_f = LoadSymbol<decltype(&::heif_image_get_plane_readonly)>("heif_image_get_plane_readonly");
        heif_image_get_plane_f = LoadSymbol<decltype(&::heif_image_get_plane)>("heif_image_get_plane");
        heif_encoder_release_f = LoadSymbol<decltype(&::heif_encoder_release)>("heif_encoder_release");
        heif_encoder_set_lossy_quality_f = LoadSymbol<decltype(&::heif_encoder_set_lossy_quality)>("heif_encoder_set_lossy_quality");
        heif_context_encode_image_f = LoadSymbol<decltype(&::heif_context_encode_image)>("heif_context_encode_image");
        heif_image_handle_get_number_of_thumbnails_f = LoadSymbol<decltype(&::heif_image_handle_get_number_of_thumbnails)>("heif_image_handle_get_number_of_thumbnails", /*required=*/false);
        heif_image_handle_get_list_of_thumbnail_IDs_f = LoadSymbol<decltype(&::heif_image_handle_get_list_of_thumbnail_IDs)>("heif_image_handle_get_list_of_thumbnail_IDs", /*required=*/false);
        heif_image_handle_get_thumbnail_f = LoadSymbol<decltype(&::heif_image_handle_get_thumbnail)>("heif_image_handle_get_thumbnail", /*required=*/false);

        if (heif_init_f) {
            heif_init_f(nullptr);
        }
    }
};


class PluginHeif
    : public fi::Plugin2
{
public:
    enum class Mode {
        eHeif = 0,
        eAvif = 1
    };

    PluginHeif(Mode mode)
        : mMode(mode)
    { }

    PluginHeif(const PluginHeif&) = delete;
    PluginHeif(PluginHeif&&) = delete;

    ~PluginHeif() override = default;

    PluginHeif& operator=(const PluginHeif&) = delete;
    PluginHeif& operator=(PluginHeif&&) = delete;

    const char* FormatProc() override { 
        switch (mMode) {
        default:
        case Mode::eHeif:
            return "HEIF";
        case Mode::eAvif:
            return "AVIF";
        }
    };

    const char* DescriptionProc() override {
        switch (mMode) {
        default:
        case Mode::eHeif:
            return "High Efficiency Image File Format";
        case Mode::eAvif:
            return "AV1 Image File Format";
        }
    }

    const char* ExtensionListProc() override {
        switch (mMode) {
        default:
        case Mode::eHeif:
            return "heif,heifs,heic,heics,hif";
        case Mode::eAvif:
            return "avif";
        }
    }

    const char* MimeProc() override {
        switch (mMode) {
        default:
        case Mode::eHeif:
            return "image/heif";
        case Mode::eAvif:
            return "image/avif";
        }
    }


    //virtual uint32_t PageCountProc(FreeImageIO* /*io*/, fi_handle /*handle*/, void* /*data*/) { return 1U; };
    //virtual uint32_t PageCapabilityProc(FreeImageIO* /*io*/, fi_handle /*handle*/, void* /*data*/) { return 1U; };


    FIBITMAP* LoadProc(FreeImageIO* io, fi_handle handle, uint32_t /*page*/, uint32_t /*flags*/, void* /*data*/) override {

        if (!io || !handle) {
            return nullptr;
        }
        auto& libHeif = LibHeif::GetInstance();

        heif_context* heifContext = libHeif.heif_context_alloc_f();
        if (!heifContext) {
            throw std::runtime_error("PluginHeif[Load]: Failed to allocate heif_context.");
        }
        yato_finally(([&, this]() { libHeif.heif_context_free_f(heifContext); }));

        io->seek_proc(handle, 0, SEEK_END);
        const size_t fileSize = io->tell_proc(handle);
        io->seek_proc(handle, 0, SEEK_SET);

        auto ioContext = std::make_tuple(io, handle, fileSize);

        heif_reader heifReader{};
        heifReader.reader_api_version = 1;
        heifReader.get_position = [](void* userdata) -> int64_t {
            const auto& [io_, handle_, _] = *yato::pointer_cast<decltype(&ioContext)>(userdata);
            return io_->tell_proc(handle_);
        };
        heifReader.read = [](void* data, size_t size, void* userdata) -> int {
            const auto& [io_, handle_, _] = *yato::pointer_cast<decltype(&ioContext)>(userdata);
            if (size == io_->read_proc(data, 1U, yato::narrow_cast<unsigned>(size), handle_)) {
                return 0; // success
            }
            return 1; // error
        };
        heifReader.seek = [](int64_t position, void* userdata) -> int {
            const auto& [io_, handle_, _] = *yato::pointer_cast<decltype(&ioContext)>(userdata);
            return io_->seek_proc(handle_, yato::narrow_cast<long>(position), SEEK_SET);
        };
        heifReader.wait_for_file_size = [](int64_t target_size, void* userdata) -> heif_reader_grow_status {
            const size_t fileSize = std::get<size_t>(*yato::pointer_cast<decltype(&ioContext)>(userdata));
            if (target_size >= 0 && yato::narrow_cast<size_t>(target_size) <= fileSize) {
                return heif_reader_grow_status::heif_reader_grow_status_size_reached;
            }
            return heif_reader_grow_status::heif_reader_grow_status_size_beyond_eof;
        };

        auto heifError = libHeif.heif_context_read_from_reader_f(heifContext, &heifReader, &ioContext, nullptr);
        if (heifError.code != heif_error_Ok) {
            throw std::runtime_error(std::string("PluginHeif[Load]: Error in heif_context_read_from_reader(). ") + heifError.message);
        }

        heif_image_handle* heifImageHandle{};
        heifError = libHeif.heif_context_get_primary_image_handle_f(heifContext, &heifImageHandle);
        if (heifError.code != heif_error_Ok) {
            throw std::runtime_error(std::string("PluginHeif[Load]: Error in heif_context_get_primary_image_handle(). ") + heifError.message);
        }
        yato_finally(([&, this]() { libHeif.heif_image_handle_release_f(heifImageHandle); }));

        UniqueBitmap bmp = DecodeImage(libHeif, heifImageHandle);
        if (!bmp) {
            return nullptr;
        }

        // EXIF
        const int heifMetaCount = libHeif.heif_image_handle_get_number_of_metadata_blocks_f(heifImageHandle, nullptr);
        if (heifMetaCount > 0) {
            std::vector<heif_item_id> heifMetaIds(yato::narrow_cast<size_t>(heifMetaCount));
            std::vector<uint8_t> heifMetaData;
            if (heifMetaCount == libHeif.heif_image_handle_get_list_of_metadata_block_IDs_f(heifImageHandle, nullptr, heifMetaIds.data(), heifMetaCount)) {
                for (const auto& itemId : heifMetaIds) {
                    const char*  heifMetaType = libHeif.heif_image_handle_get_metadata_type_f(heifImageHandle, itemId);
                    const size_t heifMetaSize = libHeif.heif_image_handle_get_metadata_size_f(heifImageHandle, itemId);
                    if (heifMetaSize == 0) {
                        continue;
                    }

                    heifMetaData.resize(heifMetaSize);
                    heifError = libHeif.heif_image_handle_get_metadata_f(heifImageHandle, itemId, heifMetaData.data());
                    if (heifError.code != heif_error_Ok) {
                        continue;
                    }

                    if (std::strcmp(heifMetaType, "Exif") == 0 && heifMetaSize > 4) {
                        // @see heif_image_handle_get_metadata
                        //
                        // For Exif data, you probably have to skip the first four bytes of the data, since they
                        // indicate the offset to the start of the TIFF header of the Exif data.
                        heifMetaData.erase(heifMetaData.cbegin(), heifMetaData.cbegin() + 4);

                        // Exif or Adobe XMP profile
                        // Some heic images store Exif without signature...
                        jpeg_read_exif_profile(bmp.get(), heifMetaData.data(), heifMetaData.size(), /*optional_signature=*/true);
                        jpeg_read_exif_profile_raw(bmp.get(), heifMetaData.data(), heifMetaData.size(), /*optional_signature=*/true);
                    }
                }
            }
        }

        // Thumbnail
        if (libHeif.heif_image_handle_get_number_of_thumbnails_f && libHeif.heif_image_handle_get_list_of_thumbnail_IDs_f && libHeif.heif_image_handle_get_thumbnail_f) {
            const int thumbnailsCount = libHeif.heif_image_handle_get_number_of_thumbnails_f(heifImageHandle);
            if (thumbnailsCount > 0) {
                std::vector<heif_item_id> heifThumbnailIds(yato::narrow_cast<size_t>(thumbnailsCount));
                if (thumbnailsCount == libHeif.heif_image_handle_get_list_of_thumbnail_IDs_f(heifImageHandle, heifThumbnailIds.data(), thumbnailsCount)) {
                    heif_image_handle* heifThumbnailHandle{};
                    heifError = libHeif.heif_image_handle_get_thumbnail_f(heifImageHandle, heifThumbnailIds.at(0), &heifThumbnailHandle);
                    if (heifError.code != heif_error_Ok) {
                        throw std::runtime_error(std::string("PluginHeif[Load]: Error in heif_image_handle_get_thumbnail(). ") + heifError.message);
                    }
                    yato_finally(([&, this]() { libHeif.heif_image_handle_release_f(heifThumbnailHandle); }));

                    UniqueBitmap thumbnail = DecodeImage(libHeif, heifThumbnailHandle);
                    if (thumbnail) {
                        FreeImage_SetThumbnail(bmp.get(), thumbnail.get());
                    }
                }
            }
        }

        return bmp.release();
    };


    bool SaveProc(FreeImageIO* io, FIBITMAP* dib, fi_handle handle, uint32_t /*page*/, uint32_t /*flags*/, void* /*data*/) override { 

        if (!io || !handle || !dib || !FreeImage_HasPixels(dib)) {
            return false;
        }
        auto& libHeif = LibHeif::GetInstance();

        const int imgWidth  = FreeImage_GetWidth(dib);
        const int imgHeight = FreeImage_GetHeight(dib);
        if (imgWidth <= 0 || imgHeight <= 0) {
            return false;
        }

        heif_context* heifContext = libHeif.heif_context_alloc_f();
        if (!heifContext) {
            throw std::runtime_error("PluginHeif[Save]: Failed to allocate heif_context.");
        }
        yato_finally(([&, this]() { libHeif.heif_context_free_f(heifContext); }));

        heif_error heifError{};
        heif_encoder* heifEncoder{};
        switch (mMode) {
        default:
        case Mode::eHeif:
            heifError = libHeif.heif_context_get_encoder_for_format_f(heifContext, heif_compression_HEVC, &heifEncoder);
            break;
        case Mode::eAvif:
            heifError = libHeif.heif_context_get_encoder_for_format_f(heifContext, heif_compression_AV1, &heifEncoder);
            break;
        }
        if (heifError.code != heif_error_Ok) {
            throw std::runtime_error(std::string("PluginHeif[Save]: Error in heif_context_get_encoder_for_format(). ") + heifError.message);
        }
        yato_finally(([&, this]() { libHeif.heif_encoder_release_f(heifEncoder); }));

        libHeif.heif_encoder_set_lossy_quality_f(heifEncoder, 75);

        const FREE_IMAGE_TYPE imgType{ FreeImage_GetImageType(dib) };
        if (imgType != FIT_BITMAP) {
            // ToDo: add support for other types...
            return false;
        }

        heif_chroma heifCromaType{ heif_chroma_undefined };
        const unsigned bpp = FreeImage_GetBPP(dib);
        switch (bpp) {
        case 32:
            heifCromaType = heif_chroma_interleaved_RGBA;
            break;
        case 24:
            heifCromaType = heif_chroma_interleaved_RGB;
            break;
        default:
            // ToDo: add support for other bpp...
            return false;
        }

        heif_image* heifImage{};
        heifError = libHeif.heif_image_create_f(imgWidth, imgHeight, heif_colorspace_RGB, heifCromaType, &heifImage);
        if (heifError.code != heif_error_Ok) {
            throw std::runtime_error(std::string("PluginHeif[Save]: Error in heif_image_create(). ") + heifError.message);
        }
        yato_finally(([&, this]() { libHeif.heif_image_release_f(heifImage); }));

        heifError = libHeif.heif_image_add_plane_f(heifImage, heif_channel_interleaved, imgWidth, imgHeight, 8);
        if (heifError.code != heif_error_Ok) {
            throw std::runtime_error(std::string("PluginHeif[Save]: Error in heif_image_add_plane(). ") + heifError.message);
        }

        // Fill pixels
        {
            int heifStride{};
            uint8_t* heifData = libHeif.heif_image_get_plane_f(heifImage, heif_channel_interleaved, &heifStride);
            if (!heifData) {
                throw std::runtime_error("PluginHeif[Save]: Error in heif_image_get_plane().");
            }

            const uint8_t* imgData = yato::pointer_cast<const uint8_t*>(FreeImage_GetBits(dib));
            const auto imgStride = FreeImage_GetPitch(dib);
            imgData += (imgHeight - 1) * imgStride;
            for (int y = 0; y < imgHeight; ++y) {
                std::memcpy(heifData, imgData, imgWidth * bpp / 8);
                imgData  -= imgStride;
                heifData += heifStride;
            }
        }

        heifError = libHeif.heif_context_encode_image_f(heifContext, heifImage, heifEncoder, nullptr, nullptr);
        if (heifError.code != heif_error_Ok) {
            throw std::runtime_error(std::string("PluginHeif[Save]: Error in heif_context_encode_image(). ") + heifError.message);
        }

        // Write to file
        auto ioContext = std::make_tuple(io, handle);

        heif_writer heifWriter{};
        heifWriter.writer_api_version = 1;
        heifWriter.write = [](struct heif_context* ctx, const void* data, size_t size, void* userdata) -> heif_error {
            const auto& [io_, handle_] = *yato::pointer_cast<decltype(&ioContext)>(userdata);
            heif_error status{};
            status.message = "Success";
            if (size != io_->write_proc(const_cast<void*>(data), 1, yato::narrow_cast<unsigned>(size), handle_)) {
                status.code = heif_error_Encoding_error;
                status.subcode = heif_suberror_Cannot_write_output_data;
                status.message = "write_proc() failed to write all bytes.";
            }
            return status;
        };

        heifError = libHeif.heif_context_write_f(heifContext, &heifWriter, &ioContext);
        if (heifError.code != heif_error_Ok) {
            throw std::runtime_error(std::string("PluginHeif[Save]: Error in heif_context_write(). ") + heifError.message);
        }

        // EXIF
        // ToDo: to be supported...

        // Thumbnail
        // ToDo: to be supported...

        return true;
    }

    bool ValidateProc(FreeImageIO* io, fi_handle handle) override {

        auto& libHeif = LibHeif::GetInstance();
        if (libHeif.heif_read_main_brand_f) {

            heif_brand2 targetBrand{};
            switch (mMode) {
            default:
            case Mode::eHeif:
                targetBrand = heif_brand2_heic;
                break;
            case Mode::eAvif:
                targetBrand = heif_brand2_avif;
                break;
            }

            std::array<uint8_t, 12> header = {};
            const unsigned readCount = io->read_proc(header.data(), 1U, header.size(), handle);
            if (readCount >= 12) {
                if (targetBrand == libHeif.heif_read_main_brand_f(header.data(), yato::narrow_cast<int>(readCount))) {
                    return true;
                }
            }
        }
        return false; 
    }

    //virtual bool SupportsExportBPPProc(uint32_t /*bpp*/) { return false; };
    //virtual bool SupportsExportTypeProc(FREE_IMAGE_TYPE /*type*/) { return false; };
    //virtual bool SupportsICCProfilesProc() { return false; };
    //virtual bool SupportsNoPixelsProc() { return false; };

private:
    UniqueBitmap DecodeImage(LibHeif& libHeif, const heif_image_handle* heifImageHandle)
    {
        if (!heifImageHandle) {
            return UniqueBitmap{ nullptr, &::FreeImage_Unload };
        }

        const int heifWidth  = libHeif.heif_image_handle_get_width_f(heifImageHandle);
        const int heifHeight = libHeif.heif_image_handle_get_height_f(heifImageHandle);
        if (heifWidth <= 0 || heifHeight <= 0) {
            throw std::runtime_error("PluginHeif[Load]: Invalid image size.");
        }

        // ToDo: no way to differ 420 from greyscale?
        //heif_colorspace heifPreferredColorspace{ heif_colorspace_undefined };
        //heif_chroma heifPreferredChroma{ heif_chroma_undefined };
        //if (mLibHeif->heif_image_handle_get_preferred_decoding_colorspace_f) {
        //    mLibHeif->heif_image_handle_get_preferred_decoding_colorspace_f(heifImageHandle, &heifPreferredColorspace, &heifPreferredChroma);
        //}

        const heif_chroma targetHefChroma = libHeif.heif_image_handle_has_alpha_channel_f(heifImageHandle) ? heif_chroma_interleaved_RGBA : heif_chroma_interleaved_RGB;

        heif_image* heifImage{};
        heif_error heifError = libHeif.heif_decode_image_f(heifImageHandle, &heifImage, heif_colorspace_RGB, targetHefChroma, nullptr);
        if (heifError.code != heif_error_Ok) {
            throw std::runtime_error(std::string("PluginHeif[Load]: Error in heif_decode_image(). ") + heifError.message);
        }
        yato_finally(([&, this]() { libHeif.heif_image_release_f(heifImage); }));

        const int heifBpp = libHeif.heif_image_get_bits_per_pixel_f(heifImage, heif_channel_interleaved);
        if (heifBpp != 8 && heifBpp != 24 && heifBpp != 32) {
            // ToDo: Add support for other bpp
            throw std::runtime_error("PluginHeif[Load]: Unsupported BPPs.");
        }

        UniqueBitmap bmp(FreeImage_Allocate(heifWidth, heifHeight, heifBpp), &::FreeImage_Unload);

        // Copy pixels
        {
            int heifStride{};
            const uint8_t* heifData = libHeif.heif_image_get_plane_readonly_f(heifImage, heif_channel_interleaved, &heifStride);
            if (!heifData) {
                throw std::runtime_error("PluginHeif[Load]: Error in heif_image_get_plane_readonly()");
            }

            uint8_t* bmpData = yato::pointer_cast<uint8_t*>(FreeImage_GetBits(bmp.get()));
            const auto bmpStride = FreeImage_GetPitch(bmp.get());
            bmpData += (heifHeight - 1) * bmpStride;
            for (int y = 0; y < heifHeight; ++y) {
                std::memcpy(bmpData, heifData, heifWidth * heifBpp / 8);
                bmpData -= bmpStride;
                heifData += heifStride;
            }
        }

        return bmp;
    }

    Mode mMode{ Mode::eHeif };
};



// ==========================================================
//	 Init
// ==========================================================


std::unique_ptr<fi::Plugin2> CreatePluginHEIF()
try
{
    return std::make_unique<PluginHeif>(PluginHeif::Mode::eHeif);
}
catch (...) {
    return nullptr;
}


std::unique_ptr<fi::Plugin2> CreatepluginAVIF() 
try
{
    return std::make_unique<PluginHeif>(PluginHeif::Mode::eAvif);
}
catch (...) {
    return nullptr;
}



std::unique_ptr<FIDEPENDENCY> MakeHeifDependencyInfo() 
try {
    auto& libHeif = LibHeif::GetInstance();
    auto info = std::make_unique<FIDEPENDENCY>();
    info->type = FIDEP_DYNAMIC;
    info->name = "LibHeif";
    info->fullVersion  = libHeif.heif_get_version_f();
    info->majorVersion = libHeif.heif_get_version_number_major_f();
    info->minorVersion = libHeif.heif_get_version_number_minor_f();
    return info;
}
catch (...) {
    return nullptr;
}
