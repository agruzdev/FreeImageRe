//===========================================================
// FreeImage Re(surrected)
// Modified fork from the original FreeImage 3.18
// with updated dependencies and extended features.
//
// See changes in ReadMe.md
// 
//===========================================================

#define CMS_NO_REGISTER_KEYWORD 1
#define JXL_CMS_STATIC_DEFINE 1
#define JXL_STATIC_DEFINE 1

#include <array>

#include "hwy/base.h"
#include "brotli/decode.h"
#include "brotli/encode.h"
#include "lcms2.h"

#include "jxl/codestream_header.h"
#include "jxl/decode_cxx.h"
#include "jxl/encode_cxx.h"

#include "FreeImage.hpp"
#include "Utilities.h"
#include "Metadata/FreeImageTag.h"
#include "FreeImage/SimpleTools.h"


class PluginJpegXL
    : public fi::Plugin2
{
public:
    PluginJpegXL() = default;

    PluginJpegXL(const PluginJpegXL&) = delete;
    PluginJpegXL(PluginJpegXL&&) = delete;

    ~PluginJpegXL() override = default;

    PluginJpegXL& operator=(const PluginJpegXL&) = delete;
    PluginJpegXL& operator=(PluginJpegXL&&) = delete;

    const char* FormatProc() override
    {
        return "JXL";
    };

    const char* DescriptionProc() override
    {
        return "JPEG XL Image Format";
    }

    const char* ExtensionListProc() override
    {
        return "jxl";
    }

    const char* MimeProc() override 
    {
        return "image/jxl";
    }

    FREE_IMAGE_TYPE DeduceFromChannels(const JxlBasicInfo& info, JxlPixelFormat& format, FREE_IMAGE_TYPE fitRgba, FREE_IMAGE_TYPE fitRgb, FREE_IMAGE_TYPE fitY)
    {
        bool hasFullAlpha = false;
        if (info.alpha_bits > 0) {
            if (info.alpha_bits == info.bits_per_sample) {
                hasFullAlpha = true;
            }
            else {
                // support if necessary
                throw std::runtime_error("PluginJpegXL[DeduceImageType]: Unsupported alpha bits");
            }
        }

        if (hasFullAlpha) {
            if (info.num_color_channels == 3 && info.num_extra_channels == 1) {
                format.num_channels = 4;    // RGBA
                return fitRgba;
            }
        }
        else {
            // without transparency
            if (info.num_color_channels == 4) {
                format.num_channels = 4;    // Try as RGBA?
                return fitRgba;
            }
            else if (info.num_color_channels == 3) {
                format.num_channels = 3;    // RGB
                return fitRgb;
            }
            else if (info.num_color_channels == 1) {
                format.num_channels = 1;    // Grayscale
                return fitY;
            }
        }

        return FIT_UNKNOWN;
    }

    FREE_IMAGE_TYPE DeduceImageType(const JxlBasicInfo& info, JxlPixelFormat& format)
    {
        format.align = 4;   // FreeImage alignment of scanline
        format.endianness = JXL_NATIVE_ENDIAN;
        format.num_channels = 0;

        if (info.bits_per_sample <= 8) {
            // BITMAP
            format.data_type = JXL_TYPE_UINT8;
            return DeduceFromChannels(info, format, FIT_BITMAP, FIT_BITMAP, FIT_BITMAP);
        }

        // not BITMAP
        if (info.exponent_bits_per_sample == 0) {
            // UINT
            if (info.bits_per_sample <= 16) {
                format.data_type = JXL_TYPE_UINT16;
                return DeduceFromChannels(info, format, FIT_RGBA16, FIT_RGB16, FIT_UINT16);
            }
        }
        else {
            // FLOAT
            if (info.bits_per_sample <= 32) {
                // float16 is not supported by FreeImage -> always ask to decode Float32
                format.data_type = JXL_TYPE_FLOAT;
                return DeduceFromChannels(info, format, FIT_RGBAF, FIT_RGBF, FIT_FLOAT);
            }
        }
        return FIT_UNKNOWN;
    }

    size_t GetDataTypeBytes(JxlDataType dtype) {
        switch (dtype) {
            case JXL_TYPE_UINT8:
                return 1;
            case JXL_TYPE_UINT16:
            case JXL_TYPE_FLOAT16:
                return 2;
            case JXL_TYPE_FLOAT:
                return 4;
            default:
                throw std::logic_error("PluginJpegXL: invalid JxlDataType");
        }
    }

    struct WriteOutBlockContext
    {
        FIBITMAP* bmp{ nullptr };
        size_t width{ 0 };
        size_t height{ 0 };
        size_t pixelSize{ 0 };
    };

    static
    void WriteOutBlockCallback(void* opaque, size_t x, size_t y, size_t num_pixels, const void* pixels) {
        auto ctx = static_cast<WriteOutBlockContext*>(opaque);
        if (!ctx || !pixels || num_pixels == 0 || x >= ctx->width || y >= ctx->height) {
            return;
        }
        auto scanline = static_cast<uint8_t*>(static_cast<void*>(FreeImage_GetScanLine(ctx->bmp, static_cast<int>(ctx->height - 1 - y))));
        const size_t num = std::min(num_pixels, ctx->width - x);
        std::memcpy(scanline + x * ctx->pixelSize, pixels, num * ctx->pixelSize);
    };

    FIBITMAP* LoadProc(FreeImageIO* io, fi_handle handle, uint32_t /*page*/, uint32_t /*flags*/, void* /*data*/) override
    {
        if (!io || !handle) {
            return nullptr;
        }

        JxlDecoderPtr dec = JxlDecoderMake(nullptr);
        if (JXL_DEC_SUCCESS != JxlDecoderSubscribeEvents(dec.get(), JXL_DEC_BASIC_INFO | JXL_DEC_COLOR_ENCODING | JXL_DEC_FULL_IMAGE)) {
            throw std::runtime_error("PluginJpegXL[Load]: JxlDecoderSubscribeEvents failed");
        }

        constexpr size_t kChunkSize = 4 * 1024;
        size_t inpBufferSize = kChunkSize;
        auto inpBuffer = std::make_unique<uint8_t[]>(inpBufferSize);

        size_t inpAvailSize = io->read_proc(inpBuffer.get(), 1, inpBufferSize, handle);
        if (inpAvailSize == 0) {
            throw std::runtime_error("PluginJpegXL[Load]: Input stream is empty");
        }

        if (JXL_DEC_SUCCESS != JxlDecoderSetInput(dec.get(), inpBuffer.get(), inpAvailSize)) {
            throw std::runtime_error("PluginJpegXL[Load]: JxlDecoderSetInput failed");
        }

        JxlBasicInfo info{};
        std::vector<uint8_t> iccProfile{};

        WriteOutBlockContext writeCtx{};
        std::unique_ptr<FIBITMAP, decltype(&::FreeImage_Unload)> bmp(nullptr, &::FreeImage_Unload);

        for (;;) {
            const auto status = JxlDecoderProcessInput(dec.get());

            if (status == JXL_DEC_ERROR) {
                throw std::runtime_error("PluginJpegXL[Load]: JXL_DEC_ERROR");
            }
            else if (status == JXL_DEC_BASIC_INFO) {
                if (JXL_DEC_SUCCESS != JxlDecoderGetBasicInfo(dec.get(), &info)) {
                    throw std::runtime_error("PluginJpegXL[Load]: JxlDecoderGetBasicInfo failed");
                }
            }
            else if (status == JXL_DEC_COLOR_ENCODING) {
                size_t iccSize{};
                if (JXL_DEC_SUCCESS != JxlDecoderGetICCProfileSize(dec.get(), JXL_COLOR_PROFILE_TARGET_DATA, &iccSize)) {
                    throw std::runtime_error("PluginJpegXL[Load]: JxlDecoderGetICCProfileSize failed");
                }
                if (iccSize > 0) {
                    iccProfile.resize(iccSize);
                    if (JXL_DEC_SUCCESS != JxlDecoderGetColorAsICCProfile(dec.get(), JXL_COLOR_PROFILE_TARGET_DATA, iccProfile.data(), iccProfile.size())) {
                        throw std::runtime_error("PluginJpegXL[Load]: JxlDecoderGetColorAsICCProfile failed");
                    }
                }
            }
            else if (status == JXL_DEC_NEED_MORE_INPUT || status == JXL_DEC_SUCCESS || status == JXL_DEC_FULL_IMAGE) {
                const size_t inpRemainSize = JxlDecoderReleaseInput(dec.get());

                if (status == JXL_DEC_NEED_MORE_INPUT) {
                    // it's okay if flush fails when output is not ready yet
                    JxlDecoderFlushImage(dec.get());

                    if (inpRemainSize >= inpAvailSize) {
                        // ToDo: maybe need to increase input chuck here and try decoding again?
                        throw std::runtime_error("PluginJpegXL[Load]: Input was not processed");
                    }

                    if (inpRemainSize > 0) {
                        // keep part of old chunck
                        const size_t inpProcSize = inpAvailSize - inpRemainSize;
                        std::memmove(inpBuffer.get(), inpBuffer.get() + inpProcSize, inpRemainSize);
                    }

                    // read more
                    const size_t readSize = io->read_proc(inpBuffer.get() + inpRemainSize, 1, inpBufferSize - inpRemainSize, handle);
                    if (!readSize) {
                        // EOF, keep as much as we could read
                        break;
                    }
                    inpAvailSize = inpRemainSize + readSize;

                    if (JXL_DEC_SUCCESS != JxlDecoderSetInput(dec.get(), inpBuffer.get(), inpAvailSize)) {
                        throw std::runtime_error("PluginJpegXL[Load]: JxlDecoderSetInput failed");
                    }
                }
                else {
                    // done!
                    break;
                }
            }
            else if (status == JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
                JxlPixelFormat format{};
                const auto fit = DeduceImageType(info, format);
                if (fit == FIT_UNKNOWN) {
                    throw std::runtime_error("PluginJpegXL[Load]: Unsupported image format");
                }

                size_t outBufferSize;
                if (JXL_DEC_SUCCESS != JxlDecoderImageOutBufferSize(dec.get(), &format, &outBufferSize)) {
                    throw std::runtime_error("PluginJpegXL[Load]: JxlDecoderImageOutBufferSize failed");
                }

                if (info.xsize > static_cast<uint32_t>(std::numeric_limits<int>::max()) || info.ysize > static_cast<uint32_t>(std::numeric_limits<int>::max())) {
                    // Not compatible with 'int' interface functions...
                    throw std::runtime_error("PluginJpegXL[Load]: Image size is too large");
                }

                const size_t pixelSize = GetDataTypeBytes(format.data_type) * format.num_channels;
                bmp.reset(FreeImage_AllocateT(fit, info.xsize, info.ysize, 8 * pixelSize));

                const size_t imageSize = FreeImage_GetPitch(bmp.get()) * FreeImage_GetHeight(bmp.get());
                if (imageSize < outBufferSize) {
                    throw std::runtime_error("PluginJpegXL[Load]: Image buffer mismatch");
                }

                writeCtx.bmp       = bmp.get();
                writeCtx.width     = info.xsize;
                writeCtx.height    = info.ysize;
                writeCtx.pixelSize = pixelSize;

                // write by scanlines
                if (JXL_DEC_SUCCESS != JxlDecoderSetImageOutCallback(dec.get(), &format, &PluginJpegXL::WriteOutBlockCallback, &writeCtx)) {
                    throw std::runtime_error("PluginJpegXL[Load]: JxlDecoderSetImageOutCallback failed");
                }
            }
            else {
                throw std::runtime_error("PluginJpegXL[Load]: Unknown decoder status");
            }
        }

        if (!iccProfile.empty()) {
            FreeImage_CreateICCProfile(bmp.get(), iccProfile.data(), iccProfile.size());
        }

        return bmp.release();
    };


    bool FillImageInfo(FIBITMAP* dib, JxlBasicInfo& info, JxlPixelFormat& format)
    {
        const auto fit = FreeImage_GetImageType(dib);
        const auto bpp = FreeImage_GetBPP(dib);

        const int32_t width  = FreeImage_GetWidth(dib);
        const int32_t height = FreeImage_GetHeight(dib);
        if (width <= 0 || height <= 0) {
            return false;
        }

        info.xsize = static_cast<uint32_t>(width);
        info.ysize = static_cast<uint32_t>(height);
        info.orientation = JXL_ORIENT_FLIP_VERTICAL;    // FreeImage stores flipped image

        format.align = 4;   // FreeImage scanline alignemnt
        format.endianness = JXL_NATIVE_ENDIAN;

        if (fit == FIT_BITMAP) {
            format.data_type = JXL_TYPE_UINT8;
            if (bpp == 32) {
                info.alpha_bits = 8;
                info.num_color_channels = 3;
                info.num_extra_channels = 1;
                format.num_channels = 4;
                return true;
            }
            else if (bpp == 24) {
                info.num_color_channels = 3;
                format.num_channels = 3;
                return true;
            }
            else if (bpp == 8) {
                info.num_color_channels = 1;
                format.num_channels = 1;
                return true;
            }
        }
        else if (fit == FIT_RGBA16) {
            format.data_type = JXL_TYPE_UINT16;
            info.alpha_bits = 16;
            info.num_color_channels = 3;
            info.num_extra_channels = 1;
            format.num_channels = 4;
            return true;
        }
        else if (fit == FIT_RGB16) {
            format.data_type = JXL_TYPE_UINT16;
            info.num_color_channels = 3;
            format.num_channels = 4;
            return true;
        }
        else if (fit == FIT_UINT16) {
            format.data_type = JXL_TYPE_UINT16;
            info.num_color_channels = 1;
            format.num_channels = 1;
            return true;
        }
        else if (fit == FIT_RGBAF) {
            format.data_type = JXL_TYPE_FLOAT;
            info.alpha_bits = 32;
            info.num_color_channels = 3;
            info.num_extra_channels = 1;
            format.num_channels = 4;
            return true;
        }
        else if (fit == FIT_RGBF) {
            format.data_type = JXL_TYPE_FLOAT;
            info.num_color_channels = 3;
            format.num_channels = 3;
            return true;
        }
        else if (fit == FIT_FLOAT) {
            format.data_type = JXL_TYPE_FLOAT;
            info.num_color_channels = 1;
            format.num_channels = 1;
            return true;
        }

        return false;
    }


    struct ChunkedFrameReader
    {
        const uint8_t* bmpBits{ nullptr };
        JxlPixelFormat format{};
        size_t width{ 0 };
        size_t height{ 0 };
        size_t pitchSize{ 0 };
        size_t pixelSize{ 0 };

        static void get_color_channels_pixel_format(void* opaque, JxlPixelFormat* pixel_format)
        {
            if (opaque && pixel_format) {
                *pixel_format = static_cast<ChunkedFrameReader*>(opaque)->format;
            }
        }

        // same, interleaved alpha
        static void get_extra_channel_pixel_format(void* opaque, size_t /*ec_index*/, JxlPixelFormat* pixel_format) {
            get_color_channels_pixel_format(opaque, pixel_format);
        }

        static const void* get_color_channel_data_at(void* opaque, size_t xpos, size_t ypos, size_t xsize, size_t ysize, size_t* row_offset)
        {
            ChunkedFrameReader* thiz = static_cast<ChunkedFrameReader*>(opaque);
            if (!thiz || xpos >= thiz->width || ypos >= thiz->height || !row_offset) {
                return nullptr;
            }
            *row_offset = thiz->pitchSize;

            if (xpos + xsize > thiz->width) {
                xsize = thiz->width - xpos;
            }
            if (ypos + ysize > thiz->height) {
                ysize = thiz->height - ypos;
            }

            return thiz->bmpBits + ypos * thiz->pitchSize + xpos * thiz->pixelSize;
        }

        // same, interleaved alpha
        static const void* get_extra_channel_data_at(void* opaque, size_t /*ec_index*/, size_t xpos, size_t ypos, size_t xsize, size_t ysize, size_t* row_offset) {
            return get_color_channel_data_at(opaque, xpos, ypos, xsize, ysize, row_offset);
        }

        static void release_buffer(void* /*opaque*/, const void* /*buf*/)
        { }
    };


    struct FileWriter
    {
        std::unique_ptr<uint8_t[]> buffer{ nullptr };
        size_t maxSize{ 0 };
        FreeImageIO* io{ nullptr };
        fi_handle handle{ nullptr };

        FileWriter(size_t size)
            : buffer(std::make_unique<uint8_t[]>(size))
            , maxSize(size)
        { }

        static void* get_buffer(void* opaque, size_t* psize)
        {
            auto thiz = static_cast<FileWriter*>(opaque);
            if (!thiz || !psize) {
                return nullptr;
            }

            *psize = std::min(*psize, thiz->maxSize);
            return thiz->buffer.get();
        }

        static void release_buffer(void* opaque, size_t written_bytes)
        {
            auto thiz = static_cast<FileWriter*>(opaque);
            if (!thiz) {
                return;
            }

            const auto len = std::min(written_bytes, thiz->maxSize);
            if (len != thiz->io->write_proc(thiz->buffer.get(), 1, static_cast<uint32_t>(len), thiz->handle)) {
                throw std::runtime_error("PluginJpegXL[Save]: Failed to write output stream");
            }
        }

        static void set_finalized_position(void* /*opaque*/, uint64_t /*finalized_position*/)
        { }
    };


    bool SaveProc(FreeImageIO* io, FIBITMAP* dib, fi_handle handle, uint32_t /*page*/, uint32_t /*flags*/, void* /*data*/) override
    {
        if (!dib || !FreeImage_HasPixels(dib) || !io || !handle) {
            return false;
        }

        JxlEncoderPtr enc = JxlEncoderMake(nullptr);

        JxlBasicInfo info{};
        JxlEncoderInitBasicInfo(&info);

        JxlPixelFormat format{};
        if (!FillImageInfo(dib, info, format)) {
            throw std::runtime_error("PluginJpegXL[Save]: Unsupported image format");
        }

        const FIICCPROFILE* icc = FreeImage_GetICCProfile(dib);
        if (icc && icc->data && icc->size) {
            info.uses_original_profile = 1;
        }

        if (JXL_ENC_SUCCESS != JxlEncoderSetBasicInfo(enc.get(), &info)) {
            throw std::runtime_error("PluginJpegXL[Save]: JxlEncoderSetBasicInfo failed");
        }

        if (icc && icc->data && icc->size) {
            if (JXL_ENC_SUCCESS != JxlEncoderSetICCProfile(enc.get(), static_cast<const uint8_t*>(icc->data), icc->size)) {
                throw std::runtime_error("PluginJpegXL[Save]: JxlEncoderSetColorEncoding failed");
            }
        }
        else {
            // sRGB by default
            JxlColorEncoding colorEncoding{};
            JxlColorEncodingSetToSRGB(&colorEncoding, /*is_gray=*/TO_JXL_BOOL(format.num_channels < 3));
            if (JXL_ENC_SUCCESS != JxlEncoderSetColorEncoding(enc.get(), &colorEncoding)) {
                throw std::runtime_error("PluginJpegXL[Save]: JxlEncoderSetColorEncoding failed");
            }
        }

        JxlEncoderFrameSettings* frameSettings = JxlEncoderFrameSettingsCreate(enc.get(), nullptr);
        if (!frameSettings) {
            throw std::runtime_error("PluginJpegXL[Save]: JxlEncoderFrameSettingsCreate failed");
        }


        constexpr size_t kOutBufferSize = 4 * 1024;
        FileWriter fileWriter{ kOutBufferSize };
        fileWriter.io = io;
        fileWriter.handle = handle;

        JxlEncoderOutputProcessor fileWriterCallbacks{};
        fileWriterCallbacks.opaque = &fileWriter;
        fileWriterCallbacks.get_buffer = &FileWriter::get_buffer;
        fileWriterCallbacks.release_buffer = &FileWriter::release_buffer;
        fileWriterCallbacks.set_finalized_position = &FileWriter::set_finalized_position;

        if (JXL_ENC_SUCCESS != JxlEncoderSetOutputProcessor(enc.get(), fileWriterCallbacks)) {
            throw std::runtime_error("PluginJpegXL[Save]: JxlEncoderSetOutputProcessor failed");
        }


        ChunkedFrameReader frameReader{};
        frameReader.bmpBits = FreeImage_GetBits(dib);
        frameReader.format = format;
        frameReader.width  = FreeImage_GetWidth(dib);
        frameReader.height = FreeImage_GetWidth(dib);
        frameReader.pixelSize = GetDataTypeBytes(format.data_type) * format.num_channels;
        frameReader.pitchSize = FreeImage_GetPitch(dib);

        JxlChunkedFrameInputSource frameReaderCallbacks{};
        frameReaderCallbacks.opaque = &frameReader;
        frameReaderCallbacks.get_color_channels_pixel_format = &ChunkedFrameReader::get_color_channels_pixel_format;
        frameReaderCallbacks.get_extra_channel_pixel_format = &ChunkedFrameReader::get_extra_channel_pixel_format;
        frameReaderCallbacks.get_color_channel_data_at = &ChunkedFrameReader::get_color_channel_data_at;
        frameReaderCallbacks.get_extra_channel_data_at = &ChunkedFrameReader::get_extra_channel_data_at;
        frameReaderCallbacks.release_buffer = &ChunkedFrameReader::release_buffer;


        if (JXL_ENC_SUCCESS != JxlEncoderAddChunkedFrame(frameSettings, /*is_last_frame=*/JXL_TRUE, frameReaderCallbacks)) {
            throw std::runtime_error("PluginJpegXL[Save]: JxlEncoderAddChunkedFrame failed");
        }
        JxlEncoderCloseInput(enc.get());

        if (JXL_ENC_SUCCESS != JxlEncoderFlushInput(enc.get())) {
            throw std::runtime_error("PluginJpegXL[Save]: JxlEncoderFlushInput failed");
        }

        return true;
    }

    bool ValidateProc(FreeImageIO* io, fi_handle handle) override
    {
        std::array<uint8_t, 12> header = {};
        const unsigned readCount = io->read_proc(header.data(), 1U, header.size(), handle);

        switch (JxlSignatureCheck(header.data(), readCount)) {
        case JXL_SIG_CODESTREAM:
        case JXL_SIG_CONTAINER:
            return true;
        default:
            return false;
        }
    }
};



// ==========================================================
// Init
// ==========================================================


std::unique_ptr<fi::Plugin2> CreatePluginJPEGXL()
try
{
    return std::make_unique<PluginJpegXL>();
}
catch (...) {
    return nullptr;
}

#define GET_BROTLI_MAJOR(hex) static_cast<uint32_t>(((hex) >> 24) & 0xFFF)
#define GET_BROTLI_MINOR(hex) static_cast<uint32_t>(((hex) >> 12) & 0xFFF)
#define GET_BROTLI_PATCH(hex) static_cast<uint32_t>((hex) & 0xFFF)

#define GET_LCMS2_MAJOR(version) static_cast<uint32_t>((version) / 100)
#define GET_LCMS2_MINOR(version) static_cast<uint32_t>(((version) % 100) / 10)
#define GET_LCMS2_MICRO(version) static_cast<uint32_t>((version) % 10)

namespace
{
    const char* MakeLcmsVersionString()
    {
        static char buffer[128] = { };
        std::snprintf(buffer, std::size(buffer), "Little-CMS v%u.%u.%u",
            GET_LCMS2_MAJOR(LCMS_VERSION), GET_LCMS2_MINOR(LCMS_VERSION), GET_LCMS2_MICRO(LCMS_VERSION));
        return buffer;
    }

    const char* MakeBrotliVersionString(uint32_t decVer, uint32_t encVer)
    {
        static char buffer[128] = { };
        if (decVer != encVer) {
            std::snprintf(buffer, std::size(buffer), "brotli: dec v%u.%u.%u, enc v%u.%u.%u",
                GET_BROTLI_MAJOR(decVer), GET_BROTLI_MINOR(decVer), GET_BROTLI_PATCH(decVer),
                GET_BROTLI_MAJOR(encVer), GET_BROTLI_MINOR(encVer), GET_BROTLI_PATCH(encVer));
        }
        else {
            std::snprintf(buffer, std::size(buffer), "brotli v%u.%u.%u",
                GET_BROTLI_MAJOR(decVer), GET_BROTLI_MINOR(decVer), GET_BROTLI_PATCH(decVer));
        }
        return buffer;
    }

} // namespace


const FIDEPENDENCY* GetJpegXlDependencyInfo()
try {
    static const FIDEPENDENCY lcmsInfo = {
        .name = "Little-CMS",
        .fullVersion = MakeLcmsVersionString(),
        .majorVersion = GET_LCMS2_MAJOR(LCMS_VERSION),
        .minorVersion = GET_LCMS2_MINOR(LCMS_VERSION),
        .type = FIDEP_STATIC,
        .next = nullptr
    };

    const uint32_t brotliDecVer = BrotliDecoderVersion();
    const uint32_t brotliEncVer = BrotliEncoderVersion();

    static const FIDEPENDENCY brotliInfo = {
        .name = "brotli",
        .fullVersion  = MakeBrotliVersionString(brotliDecVer, brotliEncVer),
        .majorVersion = GET_BROTLI_MAJOR(brotliDecVer),
        .minorVersion = GET_BROTLI_MINOR(brotliDecVer),
        .type = FIDEP_STATIC,
        .next = &lcmsInfo
    };

    static const FIDEPENDENCY hwyInfo = {
        .name = "highway",
        .fullVersion = "highway v" FI_QUOTE(HWY_MAJOR) "." FI_QUOTE(HWY_MINOR) "." FI_QUOTE(HWY_PATCH),
        .majorVersion = HWY_MAJOR,
        .minorVersion = HWY_MINOR,
        .type = FIDEP_STATIC,
        .next = &brotliInfo
    };

    static const FIDEPENDENCY info = {
        .name = "LibJXL",
        .fullVersion = "LibJXL v" FI_QUOTE(JPEGXL_MAJOR_VERSION) "." FI_QUOTE(JPEGXL_MINOR_VERSION) "." FI_QUOTE(JPEGXL_PATCH_VERSION),
        .majorVersion = JPEGXL_MAJOR_VERSION,
        .minorVersion = JPEGXL_MINOR_VERSION,
        .type = FIDEP_STATIC,
        .next = &hwyInfo
    };

    return &info;
}
catch (...) {
    return nullptr;
}
