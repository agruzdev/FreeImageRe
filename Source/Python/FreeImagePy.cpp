// ==========================================================
// FreeImageRe Python bindings
//
// ==========================================================

#include <array>
#include <iostream>
#include <filesystem>
#include "nanobind/nanobind.h"
#include "nanobind/ndarray.h"
#include "nanobind/stl/filesystem.h"
#include "FreeImage.hpp"

namespace
{
    using CpuArray = nanobind::ndarray<nanobind::numpy, nanobind::device::cpu, nanobind::c_contig>;

    nanobind::dlpack::dtype DeduceDType(const fi::Bitmap& bmp)
    {
        switch (bmp.GetImageType()) {
        case fi::ImageType::eBitmap:
            return nanobind::dtype<uint8_t>();
        case fi::ImageType::eUInt16:
        case fi::ImageType::eRgb16:
        case fi::ImageType::eRgba16:
            return nanobind::dtype<uint16_t>();
        case fi::ImageType::eInt16:
            return nanobind::dtype<int16_t>();
        case fi::ImageType::eUint32:
        case fi::ImageType::eRgb32:
        case fi::ImageType::eRgba32:
            return nanobind::dtype<uint32_t>();
        case fi::ImageType::eInt32:
            return nanobind::dtype<int32_t>();
        case fi::ImageType::eFloat:
        case fi::ImageType::eComplexF:
        case fi::ImageType::eRgbF:
        case fi::ImageType::eRgbaF:
            return nanobind::dtype<float>();
        case fi::ImageType::eDouble:
        case fi::ImageType::eComplex:
            return nanobind::dtype<double>();
        default:
            throw std::runtime_error("Unsupported image type: " + std::to_string(static_cast<FREE_IMAGE_TYPE>(bmp.GetImageType())));
        }
    }

    std::tuple<CpuArray, fi::ImageFormat> LoadWithFormat(const std::filesystem::path& path, int flags)
    {
        const auto format = fi::DetectFormat(path);
        if (format == fi::ImageFormat::eUnknown) {
            throw std::runtime_error("Failed to deduce image format");
        }
        auto  bitmapCapsule = nanobind::capsule(new fi::Bitmap(format, path, flags), "BitmapRef", [](void* p) noexcept { delete static_cast<fi::Bitmap*>(p); });
        auto& bitmap = *static_cast<fi::Bitmap*>(bitmapCapsule.data()); // not null
        const auto bpp = bitmap.GetBPP();
        if (bpp % 8 != 0) {
            throw std::runtime_error("Invalid bitmap. Bitdepth is not multiple of 8.");
        }
        const auto elemSize   = bpp / 8 / bitmap.GetChannelsNumber();
        const auto bytesPitch = bitmap.GetPitch();  // always alinged till 32-bit
        if (elemSize <= 0 || bytesPitch <= 0 || bytesPitch % elemSize != 0) {
            throw std::runtime_error("Not suppoted yet. Image pitch is not a multiple of element size.");
        }
        const std::array<size_t, 3>  shape   = { bitmap.GetHeight(), bitmap.GetWidth(), bitmap.GetChannelsNumber() };
        const std::array<int64_t, 3> strides = { bytesPitch / elemSize, bitmap.GetChannelsNumber(), 1 };

        auto array = CpuArray(bitmap.GetBits(), shape.size(), shape.data(), bitmapCapsule, strides.data(), DeduceDType(bitmap));
        return std::make_tuple(array, format);
    }

    fi::ImageType DeduceFiType(const nanobind::dlpack::dtype& dtype, size_t channels)
    {
        static_assert(sizeof(short) == 2, "Invalid short type");
        static_assert(sizeof(int) == 4, "Invalid int type");

        if (dtype.lanes != 1) {
            throw std::runtime_error("Unsupported dtype with lanes != 1");
        }

        if (dtype.code == static_cast<uint8_t>(nanobind::dlpack::dtype_code::UInt)) {
            if ((dtype.bits == 8) && (channels == 1 || channels == 3 || channels == 4)) {
                return fi::ImageType::eBitmap;
            }
            else if (dtype.bits == 16) {
                if (channels == 1) {
                    return fi::ImageType::eUInt16;
                }
                else if (channels == 3) {
                    return fi::ImageType::eRgb16;
                }
                else if (channels == 4) {
                    return fi::ImageType::eRgba16;
                }
            }
            else if (dtype.bits == 32) {
                if (channels == 1) {
                    return fi::ImageType::eUint32;
                }
                else if (channels == 3) {
                    return fi::ImageType::eRgb32;
                }
                else if (channels == 4) {
                    return fi::ImageType::eRgba32;
                }
            }
        }
        else if (dtype.code == static_cast<uint8_t>(nanobind::dlpack::dtype_code::Int)) {
            if (dtype.bits == 16 && channels == 1) {
                return fi::ImageType::eInt16;
            }
            else if (dtype.bits == 32 && channels == 1) {
                return fi::ImageType::eInt32;
            }
        }
        else if (dtype.code == static_cast<uint8_t>(nanobind::dlpack::dtype_code::Float)) {
            if (dtype.bits == 32) {
                if (channels == 1) {
                    return fi::ImageType::eFloat;
                }
                else if (channels == 3) {
                    return fi::ImageType::eRgbF;
                }
                else if (channels == 4) {
                    return fi::ImageType::eRgbaF;
                }
            }
            else if (dtype.bits == 64) {
                if (channels == 1) {
                    return fi::ImageType::eDouble;
                }
            }
        }
        else if (dtype.code == static_cast<uint8_t>(nanobind::dlpack::dtype_code::Complex)) {
            if (channels == 2) {
                if (dtype.bits == 32) {
                    return fi::ImageType::eComplexF;
                }
                else if (dtype.bits == 64) {
                    return fi::ImageType::eComplex;
                }
            }
        }
        throw std::runtime_error("Unsupported combination of dtype and channels number: dtype("
            + std::to_string(dtype.code) + ", " + std::to_string(dtype.bits) + ", " + std::to_string(dtype.lanes) + "), channels = " + std::to_string(channels));
    }

    uint32_t NarrowCastSize(size_t s)
    {
        if (s > std::numeric_limits<int>::max()) {
            throw std::runtime_error("Size of of bounds, must be less than MAX_INT");
        }
        return static_cast<uint32_t>(s);
    }

    uint32_t NarrowCastSize(int64_t s)
    {
        if (s > std::numeric_limits<int>::max()) {
            throw std::runtime_error("Size of of bounds, must be less than MAX_INT");
        }
        return static_cast<uint32_t>(s);
    }

    bool Save(fi::ImageFormat format, const CpuArray& arr, const std::filesystem::path& path, int flags)
    {
        if (arr.ndim() < 2 && arr.ndim() > 3) {
            throw std::runtime_error("Array must have 2 or 3 dims.");
        }

        std::array<size_t, 3>  shape   = {1, 1, 1};
        std::array<int64_t, 3> strides = {1, 1, 1};
        for (size_t i = 0; i < arr.ndim(); ++i) {
            shape.at(i)   = arr.shape(i);
            strides.at(i) = arr.stride(i);
        }
        if (arr.ndim() == 2) {
            // broadcast 2D to 3D
            strides.at(2) = arr.stride(1);
        }

        // deduce bpp from dtype and channels
        size_t channels = shape.at(2);
        if (arr.dtype().code == static_cast<uint8_t>(nanobind::dlpack::dtype_code::Complex)) {
            channels *= 2;
        }
        const auto imageType = DeduceFiType(arr.dtype(), channels);
        const auto elemBits = arr.dtype().bits;
        const auto bpp = elemBits * channels;

        // FreeImage doesn't support arbitrary pixel stride
        if (strides.at(1) != channels || strides.at(2) != 1) {
            throw std::runtime_error("Unsupported pixel stride");
        }

        // Attach header and save
        const auto bmpView = fi::Bitmap::FromRawBits(false, static_cast<uint8_t*>(const_cast<void*>(arr.data())), imageType, NarrowCastSize(shape.at(1)), NarrowCastSize(shape.at(0)),
            NarrowCastSize(strides.at(0)), NarrowCastSize(bpp), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);

        return bmpView.Save(format, path.c_str(), flags);
    }

} // namespace


NB_MODULE(_freeimage, m)
{
    using namespace nanobind::literals;

    m.doc() = "Python bindigs for FreeImageRe";

    // Save/Load functions
    m.def("load", [](const std::filesystem::path& path, int flags) {
        auto [img, f] = LoadWithFormat(path, flags);
        (void)f;
        return img;
    }, "path"_a, "flags"_a = 0);

    m.def("loadf", [](const std::filesystem::path& path, int flags) {
        auto [img, f] = LoadWithFormat(path, flags);
        return nanobind::make_tuple(std::move(img), nanobind::int_(static_cast<int>(f)));
    }, "path"_a, "flags"_a = 0);

    m.def("save", [](int format, const CpuArray& arr, const std::filesystem::path& path, int flags) {
        return Save(static_cast<fi::ImageFormat>(format), arr, path, flags);
    }, "format"_a, "arr"_a, "path"_a, "flags"_a = 0);


    // Format constants
    m.attr("FIF_BMP")    = nanobind::int_(static_cast<int>(FIF_BMP));
    m.attr("FIF_ICO")    = nanobind::int_(static_cast<int>(FIF_ICO));
    m.attr("FIF_JPEG")   = nanobind::int_(static_cast<int>(FIF_JPEG));
    m.attr("FIF_JNG")    = nanobind::int_(static_cast<int>(FIF_JNG));
    m.attr("FIF_KOALA")  = nanobind::int_(static_cast<int>(FIF_KOALA));
    m.attr("FIF_LBM")    = nanobind::int_(static_cast<int>(FIF_LBM));
    m.attr("FIF_IFF")    = nanobind::int_(static_cast<int>(FIF_IFF));
    m.attr("FIF_MNG")    = nanobind::int_(static_cast<int>(FIF_MNG));
    m.attr("FIF_PBM")    = nanobind::int_(static_cast<int>(FIF_PBM));
    m.attr("FIF_PBMRAW") = nanobind::int_(static_cast<int>(FIF_PBMRAW));
    m.attr("FIF_PCD")    = nanobind::int_(static_cast<int>(FIF_PCD));
    m.attr("FIF_PCX")    = nanobind::int_(static_cast<int>(FIF_PCX));
    m.attr("FIF_PGM")    = nanobind::int_(static_cast<int>(FIF_PGM));
    m.attr("FIF_PGMRAW") = nanobind::int_(static_cast<int>(FIF_PGMRAW));
    m.attr("FIF_PNG")    = nanobind::int_(static_cast<int>(FIF_PNG));
    m.attr("FIF_PPM")    = nanobind::int_(static_cast<int>(FIF_PPM));
    m.attr("FIF_PPMRAW") = nanobind::int_(static_cast<int>(FIF_PPMRAW));
    m.attr("FIF_RAS")    = nanobind::int_(static_cast<int>(FIF_RAS));
    m.attr("FIF_TARGA")  = nanobind::int_(static_cast<int>(FIF_TARGA));
    m.attr("FIF_TIFF")   = nanobind::int_(static_cast<int>(FIF_TIFF));
    m.attr("FIF_WBMP")   = nanobind::int_(static_cast<int>(FIF_WBMP));
    m.attr("FIF_PSD")    = nanobind::int_(static_cast<int>(FIF_PSD));
    m.attr("FIF_CUT")    = nanobind::int_(static_cast<int>(FIF_CUT));
    m.attr("FIF_XBM")    = nanobind::int_(static_cast<int>(FIF_XBM));
    m.attr("FIF_XPM")    = nanobind::int_(static_cast<int>(FIF_XPM));
    m.attr("FIF_DDS")    = nanobind::int_(static_cast<int>(FIF_DDS));
    m.attr("FIF_GIF")    = nanobind::int_(static_cast<int>(FIF_GIF));
    m.attr("FIF_HDR")    = nanobind::int_(static_cast<int>(FIF_HDR));
    m.attr("FIF_FAXG3")  = nanobind::int_(static_cast<int>(FIF_FAXG3));
    m.attr("FIF_SGI")    = nanobind::int_(static_cast<int>(FIF_SGI));
    m.attr("FIF_EXR")    = nanobind::int_(static_cast<int>(FIF_EXR));
    m.attr("FIF_J2K")    = nanobind::int_(static_cast<int>(FIF_J2K));
    m.attr("FIF_JP2")    = nanobind::int_(static_cast<int>(FIF_JP2));
    m.attr("FIF_PFM")    = nanobind::int_(static_cast<int>(FIF_PFM));
    m.attr("FIF_PICT")   = nanobind::int_(static_cast<int>(FIF_PICT));
    m.attr("FIF_RAW")    = nanobind::int_(static_cast<int>(FIF_RAW));
    m.attr("FIF_WEBP")   = nanobind::int_(static_cast<int>(FIF_WEBP));
    m.attr("FIF_JXR")    = nanobind::int_(static_cast<int>(FIF_JXR));
    m.attr("FIF_HEIF")   = nanobind::int_(static_cast<int>(FIF_HEIF));
    m.attr("FIF_AVIF")   = nanobind::int_(static_cast<int>(FIF_AVIF));
    m.attr("FIF_JPEGXL") = nanobind::int_(static_cast<int>(FIF_JPEGXL));


    // Save/Load constants
    // FIF_LOAD_NOPIXELS is skipped, because has no sense for loading into numpy
    m.attr("BMP_DEFAULT")       = nanobind::int_(BMP_DEFAULT);
    m.attr("BMP_SAVE_RLE")      = nanobind::int_(BMP_SAVE_RLE);
    m.attr("CUT_DEFAULT")       = nanobind::int_(CUT_DEFAULT);
    m.attr("DDS_DEFAULT")       = nanobind::int_(DDS_DEFAULT);
    m.attr("EXR_DEFAULT")       = nanobind::int_(EXR_DEFAULT);
    m.attr("EXR_FLOAT")         = nanobind::int_(EXR_FLOAT);
    m.attr("EXR_NONE")          = nanobind::int_(EXR_NONE);
    m.attr("EXR_ZIP")           = nanobind::int_(EXR_ZIP);
    m.attr("EXR_PIZ")           = nanobind::int_(EXR_PIZ);
    m.attr("EXR_PXR24")         = nanobind::int_(EXR_PXR24);
    m.attr("EXR_B44")           = nanobind::int_(EXR_B44);
    m.attr("EXR_LC")            = nanobind::int_(EXR_LC);
    m.attr("FAXG3_DEFAULT")     = nanobind::int_(FAXG3_DEFAULT);
    m.attr("GIF_DEFAULT")       = nanobind::int_(GIF_DEFAULT);
    m.attr("GIF_LOAD256")       = nanobind::int_(GIF_LOAD256);
    m.attr("GIF_PLAYBACK")      = nanobind::int_(GIF_PLAYBACK);
    m.attr("HDR_DEFAULT")       = nanobind::int_(HDR_DEFAULT);
    m.attr("ICO_MAKEALPHA")     = nanobind::int_(ICO_MAKEALPHA);
    m.attr("IFF_DEFAULT")       = nanobind::int_(IFF_DEFAULT);
    m.attr("J2K_DEFAULT")       = nanobind::int_(J2K_DEFAULT);
    m.attr("JP2_DEFAULT")       = nanobind::int_(JP2_DEFAULT);
    m.attr("JPEG_DEFAULT")      = nanobind::int_(JPEG_DEFAULT);
    m.attr("JPEG_FAST")         = nanobind::int_(JPEG_FAST);
    m.attr("JPEG_ACCURATE")     = nanobind::int_(JPEG_ACCURATE);
    m.attr("JPEG_CMYK")         = nanobind::int_(JPEG_CMYK);
    m.attr("JPEG_EXIFROTATE")     = nanobind::int_(JPEG_EXIFROTATE);
    m.attr("JPEG_GREYSCALE")      = nanobind::int_(JPEG_GREYSCALE);
    m.attr("JPEG_QUALITYSUPERB")  = nanobind::int_(JPEG_QUALITYSUPERB);
    m.attr("JPEG_QUALITYGOOD")    = nanobind::int_(JPEG_QUALITYGOOD);
    m.attr("JPEG_QUALITYNORMAL")  = nanobind::int_(JPEG_QUALITYNORMAL);
    m.attr("JPEG_QUALITYAVERAGE") = nanobind::int_(JPEG_QUALITYAVERAGE);
    m.attr("JPEG_QUALITYBAD")     = nanobind::int_(JPEG_QUALITYBAD);
    m.attr("JPEG_PROGRESSIVE")    = nanobind::int_(JPEG_PROGRESSIVE);
    m.attr("JPEG_SUBSAMPLING_411") = nanobind::int_(JPEG_SUBSAMPLING_411);
    m.attr("JPEG_SUBSAMPLING_420") = nanobind::int_(JPEG_SUBSAMPLING_420);
    m.attr("JPEG_SUBSAMPLING_422") = nanobind::int_(JPEG_SUBSAMPLING_422);
    m.attr("JPEG_SUBSAMPLING_444") = nanobind::int_(JPEG_SUBSAMPLING_444);
    m.attr("JPEG_OPTIMIZE")     = nanobind::int_(JPEG_OPTIMIZE);
    m.attr("JPEG_BASELINE")     = nanobind::int_(JPEG_BASELINE);
    m.attr("KOALA_DEFAULT")     = nanobind::int_(KOALA_DEFAULT);
    m.attr("LBM_DEFAULT")       = nanobind::int_(LBM_DEFAULT);
    m.attr("MNG_DEFAULT")       = nanobind::int_(MNG_DEFAULT);
    m.attr("PCD_DEFAULT")       = nanobind::int_(PCD_DEFAULT);
    m.attr("PCD_BASE")          = nanobind::int_(PCD_BASE);
    m.attr("PCD_BASEDIV4")      = nanobind::int_(PCD_BASEDIV4);
    m.attr("PCD_BASEDIV16")     = nanobind::int_(PCD_BASEDIV16);
    m.attr("PCX_DEFAULT")       = nanobind::int_(PCX_DEFAULT);
    m.attr("PFM_DEFAULT")       = nanobind::int_(PFM_DEFAULT);
    m.attr("PICT_DEFAULT")      = nanobind::int_(PICT_DEFAULT);
    m.attr("PNG_DEFAULT")       = nanobind::int_(PNG_DEFAULT);
    m.attr("PNG_IGNOREGAMMA")   = nanobind::int_(PNG_IGNOREGAMMA);
    m.attr("PNG_Z_BEST_SPEED")  = nanobind::int_(PNG_Z_BEST_SPEED);
    m.attr("PNG_Z_DEFAULT_COMPRESSION") = nanobind::int_(PNG_Z_DEFAULT_COMPRESSION);
    m.attr("PNG_Z_BEST_COMPRESSION") = nanobind::int_(PNG_Z_BEST_COMPRESSION);
    m.attr("PNG_Z_NO_COMPRESSION") = nanobind::int_(PNG_Z_NO_COMPRESSION);
    m.attr("PNG_INTERLACED")    = nanobind::int_(PNG_INTERLACED);
    m.attr("PNM_DEFAULT")       = nanobind::int_(PNM_DEFAULT);
    m.attr("PNM_SAVE_RAW")      = nanobind::int_(PNM_SAVE_RAW);
    m.attr("PNM_SAVE_ASCII")    = nanobind::int_(PNM_SAVE_ASCII);
    m.attr("PSD_DEFAULT")       = nanobind::int_(PSD_DEFAULT);
    m.attr("PSD_CMYK")          = nanobind::int_(PSD_CMYK);
    m.attr("PSD_LAB")           = nanobind::int_(PSD_LAB);
    m.attr("PSD_NONE")          = nanobind::int_(PSD_NONE);
    m.attr("PSD_RLE")           = nanobind::int_(PSD_RLE);
    m.attr("PSD_PSB")           = nanobind::int_(PSD_PSB);
    m.attr("RAS_DEFAULT")       = nanobind::int_(RAS_DEFAULT);
    m.attr("RAW_DEFAULT")       = nanobind::int_(RAW_DEFAULT);
    m.attr("RAW_PREVIEW")       = nanobind::int_(RAW_PREVIEW);
    m.attr("RAW_DISPLAY")       = nanobind::int_(RAW_DISPLAY);
    m.attr("RAW_HALFSIZE")      = nanobind::int_(RAW_HALFSIZE);
    m.attr("RAW_UNPROCESSED")   = nanobind::int_(RAW_UNPROCESSED);
    m.attr("SGI_DEFAULT")       = nanobind::int_(SGI_DEFAULT);
    m.attr("TARGA_DEFAULT")     = nanobind::int_(TARGA_DEFAULT);
    m.attr("TARGA_LOAD_RGB888") = nanobind::int_(TARGA_LOAD_RGB888);
    m.attr("TARGA_SAVE_RLE")    = nanobind::int_(TARGA_SAVE_RLE);
    m.attr("TIFF_DEFAULT")      = nanobind::int_(TIFF_DEFAULT);
    m.attr("TIFF_CMYK")         = nanobind::int_(TIFF_CMYK);
    m.attr("TIFF_PACKBITS")     = nanobind::int_(TIFF_PACKBITS);
    m.attr("TIFF_DEFLATE")      = nanobind::int_(TIFF_DEFLATE);
    m.attr("TIFF_ADOBE_DEFLATE") = nanobind::int_(TIFF_ADOBE_DEFLATE);
    m.attr("TIFF_NONE")         = nanobind::int_(TIFF_NONE);
    m.attr("TIFF_CCITTFAX3")    = nanobind::int_(TIFF_CCITTFAX3);
    m.attr("TIFF_CCITTFAX4")    = nanobind::int_(TIFF_CCITTFAX4);
    m.attr("TIFF_LZW")          = nanobind::int_(TIFF_LZW);
    m.attr("TIFF_JPEG")         = nanobind::int_(TIFF_JPEG);
    m.attr("TIFF_LOGLUV")       = nanobind::int_(TIFF_LOGLUV);
    m.attr("WBMP_DEFAULT")      = nanobind::int_(WBMP_DEFAULT);
    m.attr("XBM_DEFAULT")       = nanobind::int_(XBM_DEFAULT);
    m.attr("XPM_DEFAULT")       = nanobind::int_(XPM_DEFAULT);
    m.attr("WEBP_DEFAULT")      = nanobind::int_(WEBP_DEFAULT);
    m.attr("WEBP_LOSSLESS")     = nanobind::int_(WEBP_LOSSLESS);
    m.attr("JXR_DEFAULT")       = nanobind::int_(JXR_DEFAULT);
    m.attr("JXR_LOSSLESS")      = nanobind::int_(JXR_LOSSLESS);
    m.attr("JXR_PROGRESSIVE")   = nanobind::int_(JXR_PROGRESSIVE);
}
