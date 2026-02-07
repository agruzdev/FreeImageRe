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

    FIBITMAP* LoadProc(FreeImageIO* io, fi_handle handle, uint32_t /*page*/, uint32_t /*flags*/, void* /*data*/) override
    {
        if (!io || !handle) {
            return nullptr;
        }

        JxlDecoderPtr dec = JxlDecoderMake(nullptr);
        if (JXL_DEC_SUCCESS != JxlDecoderSubscribeEvents(dec.get(), JXL_DEC_BASIC_INFO | JXL_DEC_COLOR_ENCODING | JXL_DEC_FULL_IMAGE)) {
            throw std::runtime_error("PluginJpegXL[Load]: JxlDecoderSubscribeEvents failed");
        }

        io->seek_proc(handle, 0, SEEK_END);
        const size_t fileSize = io->tell_proc(handle);
        io->seek_proc(handle, 0, SEEK_SET);

        // ToDo: Is it possible to read by blocks?
        auto inputBuffer = std::make_unique<uint8_t[]>(fileSize);
        if (fileSize != io->read_proc(inputBuffer.get(), 1, fileSize, handle)) {
            throw std::runtime_error("PluginJpegXL[Load]: Failed to read whole input stream");
        }

        if (JXL_DEC_SUCCESS != JxlDecoderSetInput(dec.get(), inputBuffer.get(), fileSize)) {
            throw std::runtime_error("PluginJpegXL[Load]: JxlDecoderSetInput failed");
        }
        JxlDecoderCloseInput(dec.get());

        JxlBasicInfo info{};
        std::vector<uint8_t> iccProfile{};

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
                if (!iccSize) {
                    continue;
                }
                iccProfile.resize(iccSize);
                if (JXL_DEC_SUCCESS != JxlDecoderGetColorAsICCProfile( dec.get(), JXL_COLOR_PROFILE_TARGET_DATA, iccProfile.data(), iccProfile.size())) {
                    throw std::runtime_error("PluginJpegXL[Load]: JxlDecoderGetColorAsICCProfile failed");
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

                bmp.reset(FreeImage_AllocateT(fit, info.xsize, info.ysize, info.bits_per_sample * info.num_color_channels));
                if (outBufferSize != FreeImage_GetPitch(bmp.get()) * FreeImage_GetHeight(bmp.get())) {
                    throw std::runtime_error("PluginJpegXL[Load]: Image buffer mismatch");
                }

                if (JXL_DEC_SUCCESS != JxlDecoderSetImageOutBuffer(dec.get(), &format, FreeImage_GetBits(bmp.get()), outBufferSize)) {
                    throw std::runtime_error("PluginJpegXL[Load]: JxlDecoderSetImageOutBuffer failed");
                }
            }
            else if (status == JXL_DEC_FULL_IMAGE) {
                continue;
            }
            else if (status == JXL_DEC_SUCCESS) {
                break;
            }
            else {
                throw std::runtime_error("PluginJpegXL[Load]: Unknown decoder status");
            }
        }

        // Can avoid when giving output buffer line-by-line?
        FreeImage_FlipVertical(bmp.get());

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

        if (JXL_ENC_SUCCESS != JxlEncoderSetBasicInfo(enc.get(), &info)) {
            throw std::runtime_error("PluginJpegXL[Save]: JxlEncoderSetBasicInfo failed");
        }

        JxlColorEncoding colorEncoding{};
        JxlColorEncodingSetToSRGB(&colorEncoding, /*is_gray=*/TO_JXL_BOOL(format.num_channels < 3));
        if (JXL_ENC_SUCCESS != JxlEncoderSetColorEncoding(enc.get(), &colorEncoding)) {
            throw std::runtime_error("PluginJpegXL[Save]: JxlEncoderSetColorEncoding failed");
        }

        JxlEncoderFrameSettings* frameSettings = JxlEncoderFrameSettingsCreate(enc.get(), nullptr);
        if (!frameSettings) {
            throw std::runtime_error("PluginJpegXL[Save]: JxlEncoderFrameSettingsCreate failed");
        }

        const size_t imageSize = static_cast<size_t>(FreeImage_GetPitch(dib) * FreeImage_GetHeight(dib));
        if (JXL_ENC_SUCCESS != JxlEncoderAddImageFrame(frameSettings, &format, FreeImage_GetBits(dib), imageSize)) {
            throw std::runtime_error("PluginJpegXL[Save]: JxlEncoderAddImageFrame failed");
        }
        JxlEncoderCloseInput(enc.get());

        constexpr size_t kOutBufferSize = 4 * 1024;
        auto safeWrite = [&](uint8_t* beg, uint8_t* end) -> bool {
            if (end < beg) {
                return false;
            }
            const size_t len = static_cast<size_t>(end - beg);
            if (len > kOutBufferSize) {
                return false;
            }
            if (len != io->write_proc(beg, 1, static_cast<uint32_t>(len), handle)) {
                return false;
            }
            return true;
        };

        auto compressed = std::make_unique<uint8_t[]>(kOutBufferSize);
        for (;;) {
            uint8_t* nextOut = compressed.get();
            size_t availOut = kOutBufferSize;
            const auto result = JxlEncoderProcessOutput(enc.get(), &nextOut, &availOut);

            if (result == JXL_ENC_NEED_MORE_OUTPUT || result == JXL_ENC_SUCCESS) {
                if (!safeWrite(compressed.get(), nextOut)) {
                    throw std::runtime_error("PluginJpegXL[Save]: Failed to write an output block");
                }
                if (result == JXL_ENC_SUCCESS) {
                    break;
                }
            }
            else if (result == JXL_ENC_ERROR) {
                throw std::runtime_error("PluginJpegXL[Save]: JXL_ENC_ERROR");
            }
            else {
                assert(false && "Unknown encoder status");
            }
        }
        compressed.reset();

        return true;
    }

    bool ValidateProc(FreeImageIO* io, fi_handle handle) override
    {
        const std::array<uint8_t, 2>  magic1 = { 0xFF, 0x0A };
        const std::array<uint8_t, 12> magic2 = { 0x00, 0x00, 0x00, 0x0C, 0x4A, 0x58, 0x4C, 0x20, 0x0D, 0x0A, 0x87, 0x0A };
        std::array<uint8_t, 12> header = {};
        const unsigned readCount = io->read_proc(header.data(), 1U, header.size(), handle);
        if (readCount >= magic1.size()) {
            if (std::equal(magic1.cbegin(), magic1.cend(), header.cbegin())) {
                return true;
            }
        }
        if (readCount >= magic2.size()) {
            if (std::equal(magic2.cbegin(), magic2.cend(), header.cbegin())) {
                return true;
            }
        }
        return false;
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
