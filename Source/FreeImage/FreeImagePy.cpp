// ==========================================================
// FreeImageRe implementation
//
// ==========================================================

#if FREEIMAGE_WITH_PYTHON_BINDINGS

#include <array>
#include <iostream>
#include "pybind11/numpy.h"
#include "FreeImage.hpp"

#ifdef _WIN32
using PathStringType = std::wstring;
#else
using PathStringType = std::string;
#endif

namespace
{

    pybind11::dtype DeduceDType(const fi::Bitmap& bmp)
    {
        switch (bmp.GetImageType()) {
        case fi::ImageType::eBitmap:
            return pybind11::dtype::of<uint8_t>();
        case fi::ImageType::eUInt16:
        case fi::ImageType::eRgb16:
        case fi::ImageType::eRgba16:
            return pybind11::dtype::of<uint16_t>();
        case fi::ImageType::eInt16:
            return pybind11::dtype::of<int16_t>();
        case fi::ImageType::eUint32:
        case fi::ImageType::eRgb32:
        case fi::ImageType::eRgba32:
            return pybind11::dtype::of<uint32_t>();
        case fi::ImageType::eInt32:
            return pybind11::dtype::of<int32_t>();
        case fi::ImageType::eFloat:
        case fi::ImageType::eComplexF:
        case fi::ImageType::eRgbF:
        case fi::ImageType::eRgbaF:
            return pybind11::dtype::of<float>();
        case fi::ImageType::eDouble:
        case fi::ImageType::eComplex:
            return pybind11::dtype::of<double>();
        default:
            throw std::runtime_error("Unsupported image type: " + std::to_string(static_cast<FREE_IMAGE_TYPE>(bmp.GetImageType())));
        }
    }


    std::tuple<pybind11::array, fi::ImageFormat> LoadWithFormat(const PathStringType& path, int flags)
    {
        const auto format = fi::DetectFormat(path.c_str());
        if (format == fi::ImageFormat::eUnknown) {
            throw std::runtime_error("Failed to deduce image format");
        }
        const auto bitmapCapsule = pybind11::capsule(new fi::Bitmap(format, path.c_str(), flags), "BitmapRef", [](void* p) { delete static_cast<fi::Bitmap*>(p); });
        const auto& bitmap = *bitmapCapsule.get_pointer<fi::Bitmap>(); // not null
        const auto bpp = bitmap.GetBPP();
        if (bpp % 8 != 0) {
            throw std::runtime_error("Images with bitdepth not multiple of 8 are not supported.");
        }
        const std::array<pybind11::ssize_t, 3> shape = { bitmap.GetHeight(), bitmap.GetWidth(), bitmap.GetChannelsNumber() };
        const std::array<pybind11::ssize_t, 3> strides = { bitmap.GetPitch(), bpp / 8,  bpp / 8 / bitmap.GetChannelsNumber() };
        return std::make_tuple(pybind11::array(DeduceDType(bitmap), shape, strides, bitmap.GetBits(), bitmapCapsule), format);
    }


    std::tuple<fi::ImageType, size_t> DeduceFiType(const pybind11::dtype& dtype, pybind11::ssize_t channels)
    {
        static_assert(sizeof(short) == 2, "Invalid short type");
        static_assert(sizeof(int) == 4, "Invalid int type");

        switch (dtype.num())
        {
        case pybind11::detail::npy_api::NPY_UINT8_:
            if (channels == 1 || channels == 3 || channels == 4) {
                return std::make_tuple(fi::ImageType::eBitmap, 8 * channels);
            }
            break;
        case pybind11::detail::npy_api::NPY_SHORT_:
            if (channels == 1) {
                return std::make_tuple(fi::ImageType::eInt16, 16);
            }
            break;
        case pybind11::detail::npy_api::NPY_USHORT_:
            if (channels == 1) {
                return std::make_tuple(fi::ImageType::eUInt16, 16);
            }
            else if (channels == 3) {
                return std::make_tuple(fi::ImageType::eRgb16, 16 * 3);
            }
            else if (channels == 4) {
                return std::make_tuple(fi::ImageType::eRgba16, 16 * 4);
            }
            break;
        case pybind11::detail::npy_api::NPY_LONG_:
            if constexpr (sizeof(long) != sizeof(int32_t)) {
                break;
            }
            [[fallthrough]];
        case pybind11::detail::npy_api::NPY_INT_:
            if (channels == 1) {
                return std::make_tuple(fi::ImageType::eInt32, 32);
            }
            break;
        case pybind11::detail::npy_api::NPY_ULONG_:
            if constexpr (sizeof(unsigned long) != sizeof(int32_t)) {
                break;
            }
            [[fallthrough]];
        case pybind11::detail::npy_api::NPY_UINT_:
            if (channels == 1) {
                return std::make_tuple(fi::ImageType::eUint32, 32);
            }
            else if (channels == 3) {
                return std::make_tuple(fi::ImageType::eRgb32, 32 * 3);
            }
            else if (channels == 4) {
                return std::make_tuple(fi::ImageType::eRgba32, 32 * 4);
            }
            break;
        case pybind11::detail::npy_api::NPY_FLOAT_:
            if (channels == 1) {
                return std::make_tuple(fi::ImageType::eFloat, 32);
            }
            else if (channels == 3) {
                return std::make_tuple(fi::ImageType::eRgbF, 32 * 3);
            }
            else if (channels == 4) {
                return std::make_tuple(fi::ImageType::eRgbaF, 32 * 4);
            }
            break;
        case pybind11::detail::npy_api::NPY_DOUBLE_:
            if (channels == 1) {
                return std::make_tuple(fi::ImageType::eDouble, 64);
            }
            break;
        case pybind11::detail::npy_api::NPY_CFLOAT_:
            if (channels == 2) {
                return std::make_tuple(fi::ImageType::eComplexF, 32 * 2);
            }
            break;
        case pybind11::detail::npy_api::NPY_CDOUBLE_:
            if (channels == 2) {
                return std::make_tuple(fi::ImageType::eComplex, 64 * 2);
            }
            break;
        default:
            break;
        }
        throw std::runtime_error("Unsupported combination if dtype and channels number");
    }

    uint32_t NarrowCastSize(pybind11::ssize_t s)
    {
        if (s > std::numeric_limits<int>::max()) {
            throw std::runtime_error("Size of of bounds, must be less than MAX_INT");
        }
        return static_cast<uint32_t>(s);
    }

    bool Save(fi::ImageFormat format, const pybind11::array& arr, const PathStringType& path, int flags)
    {
        if (arr.ndim() < 2 && arr.ndim() > 3) {
            throw std::runtime_error("Array must have 2 or 3 dims.");
        }

        std::array<pybind11::ssize_t, 3> shape   = {1, 1, 1};
        std::array<pybind11::ssize_t, 3> strides = {1, 1, 1};
        for (pybind11::ssize_t i = 0; i < arr.ndim(); ++i) {
            shape.at(i) = arr.shape(i);
            strides.at(i) = arr.strides(i);
        }
        if (arr.ndim() == 2) {
            // broadcast 2D to 3D
            strides.at(2) = arr.strides(1);
        }

        // deduce bpp from dtype and channels
        pybind11::ssize_t channels = shape.at(2);
        if (arr.dtype().num() >= pybind11::detail::npy_api::NPY_CFLOAT_ && arr.dtype().num() <= pybind11::detail::npy_api::NPY_CLONGDOUBLE_) {
            channels *= 2;
        }
        const auto [imageType, bpp] = DeduceFiType(arr.dtype(), channels);
        const auto elemSize = bpp / channels;

        // FreeImage doesn't support arbitrary pixel stride
        if (8 * strides.at(1) != bpp || 8 * strides.at(2) != elemSize) {
            throw std::runtime_error("Unsupported pixel stride");
        }

        // Attach header and save
        const auto bmpView = fi::Bitmap::FromRawBits(false, static_cast<uint8_t*>(const_cast<void*>(arr.data())), imageType, NarrowCastSize(shape.at(1)), NarrowCastSize(shape.at(0)),
            NarrowCastSize(arr.strides(0)), NarrowCastSize(bpp), 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);

        return bmpView.Save(format, path.c_str(), flags);
    }

} // namespace

PYBIND11_MODULE(FreeImage, m) {
    m.doc() = "Python bindigs for FreeImageRe";

    // Save/Load functions
    m.def("load", [](const PathStringType& path, int flags) {
        auto [img, f] = LoadWithFormat(path, flags);
        (void)f;
        return img;
    }, pybind11::arg("path"), pybind11::arg("flags") = 0);

    m.def("loadf", [](const PathStringType& path, int flags) {
        auto [img, f] = LoadWithFormat(path, flags);
        return pybind11::make_tuple(std::move(img), pybind11::int_(static_cast<int>(f)));
    }, pybind11::arg("path"), pybind11::arg("flags") = 0);

    m.def("save", [](int format, const pybind11::array& arr, const PathStringType& path, int flags) {
        return pybind11::bool_(Save(static_cast<fi::ImageFormat>(format), arr, path, flags));
    }, pybind11::arg("format"), pybind11::arg("arr"), pybind11::arg("path"), pybind11::arg("flags") = 0);


    // Format constants
    m.attr("FIF_BMP") = pybind11::int_(static_cast<int>(FIF_BMP));
    m.attr("FIF_ICO") = pybind11::int_(static_cast<int>(FIF_ICO));
    m.attr("FIF_JPEG") = pybind11::int_(static_cast<int>(FIF_JPEG));
    m.attr("FIF_JNG") = pybind11::int_(static_cast<int>(FIF_JNG));
    m.attr("FIF_KOALA") = pybind11::int_(static_cast<int>(FIF_KOALA));
    m.attr("FIF_LBM") = pybind11::int_(static_cast<int>(FIF_LBM));
    m.attr("FIF_IFF") = pybind11::int_(static_cast<int>(FIF_IFF));
    m.attr("FIF_MNG") = pybind11::int_(static_cast<int>(FIF_MNG));
    m.attr("FIF_PBM") = pybind11::int_(static_cast<int>(FIF_PBM));
    m.attr("FIF_PBMRAW") = pybind11::int_(static_cast<int>(FIF_PBMRAW));
    m.attr("FIF_PCD") = pybind11::int_(static_cast<int>(FIF_PCD));
    m.attr("FIF_PCX") = pybind11::int_(static_cast<int>(FIF_PCX));
    m.attr("FIF_PGM") = pybind11::int_(static_cast<int>(FIF_PGM));
    m.attr("FIF_PGMRAW") = pybind11::int_(static_cast<int>(FIF_PGMRAW));
    m.attr("FIF_PNG") = pybind11::int_(static_cast<int>(FIF_PNG));
    m.attr("FIF_PPM") = pybind11::int_(static_cast<int>(FIF_PPM));
    m.attr("FIF_PPMRAW") = pybind11::int_(static_cast<int>(FIF_PPMRAW));
    m.attr("FIF_RAS") = pybind11::int_(static_cast<int>(FIF_RAS));
    m.attr("FIF_TARGA") = pybind11::int_(static_cast<int>(FIF_TARGA));
    m.attr("FIF_TIFF") = pybind11::int_(static_cast<int>(FIF_TIFF));
    m.attr("FIF_WBMP") = pybind11::int_(static_cast<int>(FIF_WBMP));
    m.attr("FIF_PSD") = pybind11::int_(static_cast<int>(FIF_PSD));
    m.attr("FIF_CUT") = pybind11::int_(static_cast<int>(FIF_CUT));
    m.attr("FIF_XBM") = pybind11::int_(static_cast<int>(FIF_XBM));
    m.attr("FIF_XPM") = pybind11::int_(static_cast<int>(FIF_XPM));
    m.attr("FIF_DDS") = pybind11::int_(static_cast<int>(FIF_DDS));
    m.attr("FIF_GIF") = pybind11::int_(static_cast<int>(FIF_GIF));
    m.attr("FIF_HDR") = pybind11::int_(static_cast<int>(FIF_HDR));
    m.attr("FIF_FAXG3") = pybind11::int_(static_cast<int>(FIF_FAXG3));
    m.attr("FIF_SGI") = pybind11::int_(static_cast<int>(FIF_SGI));
    m.attr("FIF_EXR") = pybind11::int_(static_cast<int>(FIF_EXR));
    m.attr("FIF_J2K") = pybind11::int_(static_cast<int>(FIF_J2K));
    m.attr("FIF_JP2") = pybind11::int_(static_cast<int>(FIF_JP2));
    m.attr("FIF_PFM") = pybind11::int_(static_cast<int>(FIF_PFM));
    m.attr("FIF_PICT") = pybind11::int_(static_cast<int>(FIF_PICT));
    m.attr("FIF_RAW") = pybind11::int_(static_cast<int>(FIF_RAW));
    m.attr("FIF_WEBP") = pybind11::int_(static_cast<int>(FIF_WEBP));
    m.attr("FIF_JXR") = pybind11::int_(static_cast<int>(FIF_JXR));


    // Save/Load constants
    m.attr("BMP_DEFAULT") = pybind11::int_(BMP_DEFAULT);
    m.attr("BMP_SAVE_RLE") = pybind11::int_(BMP_SAVE_RLE);
    m.attr("CUT_DEFAULT") = pybind11::int_(CUT_DEFAULT);
    m.attr("DDS_DEFAULT") = pybind11::int_(DDS_DEFAULT);
    m.attr("EXR_DEFAULT") = pybind11::int_(EXR_DEFAULT);
    m.attr("EXR_FLOAT") = pybind11::int_(EXR_FLOAT);
    m.attr("EXR_NONE") = pybind11::int_(EXR_NONE);
    m.attr("EXR_ZIP") = pybind11::int_(EXR_ZIP);
    m.attr("EXR_PIZ") = pybind11::int_(EXR_PIZ);
    m.attr("EXR_PXR24") = pybind11::int_(EXR_PXR24);
    m.attr("EXR_B44") = pybind11::int_(EXR_B44);
    m.attr("EXR_LC") = pybind11::int_(EXR_LC);
    m.attr("FAXG3_DEFAULT") = pybind11::int_(FAXG3_DEFAULT);
    m.attr("GIF_DEFAULT") = pybind11::int_(GIF_DEFAULT);
    m.attr("GIF_LOAD256") = pybind11::int_(GIF_LOAD256);
    m.attr("GIF_PLAYBACK") = pybind11::int_(GIF_PLAYBACK);
    m.attr("HDR_DEFAULT") = pybind11::int_(HDR_DEFAULT);
    m.attr("ICO_MAKEALPHA") = pybind11::int_(ICO_MAKEALPHA);
    m.attr("IFF_DEFAULT") = pybind11::int_(IFF_DEFAULT);
    m.attr("J2K_DEFAULT") = pybind11::int_(J2K_DEFAULT);
    m.attr("JP2_DEFAULT") = pybind11::int_(JP2_DEFAULT);
    m.attr("JPEG_DEFAULT") = pybind11::int_(JPEG_DEFAULT);
    m.attr("JPEG_FAST") = pybind11::int_(JPEG_FAST);
    m.attr("JPEG_ACCURATE") = pybind11::int_(JPEG_ACCURATE);
    m.attr("JPEG_CMYK") = pybind11::int_(JPEG_CMYK);
    m.attr("JPEG_EXIFROTATE") = pybind11::int_(JPEG_EXIFROTATE);
    m.attr("JPEG_GREYSCALE") = pybind11::int_(JPEG_GREYSCALE);
    m.attr("JPEG_QUALITYSUPERB") = pybind11::int_(JPEG_QUALITYSUPERB);
    m.attr("JPEG_QUALITYGOOD") = pybind11::int_(JPEG_QUALITYGOOD);
    m.attr("JPEG_QUALITYNORMAL") = pybind11::int_(JPEG_QUALITYNORMAL);
    m.attr("JPEG_QUALITYAVERAGE") = pybind11::int_(JPEG_QUALITYAVERAGE);
    m.attr("JPEG_QUALITYBAD") = pybind11::int_(JPEG_QUALITYBAD);
    m.attr("JPEG_PROGRESSIVE") = pybind11::int_(JPEG_PROGRESSIVE);
    m.attr("JPEG_SUBSAMPLING_411") = pybind11::int_(JPEG_SUBSAMPLING_411);
    m.attr("JPEG_SUBSAMPLING_420") = pybind11::int_(JPEG_SUBSAMPLING_420);
    m.attr("JPEG_SUBSAMPLING_422") = pybind11::int_(JPEG_SUBSAMPLING_422);
    m.attr("JPEG_SUBSAMPLING_444") = pybind11::int_(JPEG_SUBSAMPLING_444);
    m.attr("JPEG_OPTIMIZE") = pybind11::int_(JPEG_OPTIMIZE);
    m.attr("JPEG_BASELINE") = pybind11::int_(JPEG_BASELINE);
    m.attr("KOALA_DEFAULT") = pybind11::int_(KOALA_DEFAULT);
    m.attr("LBM_DEFAULT") = pybind11::int_(LBM_DEFAULT);
    m.attr("MNG_DEFAULT") = pybind11::int_(MNG_DEFAULT);
    m.attr("PCD_DEFAULT") = pybind11::int_(PCD_DEFAULT);
    m.attr("PCD_BASE") = pybind11::int_(PCD_BASE);
    m.attr("PCD_BASEDIV4") = pybind11::int_(PCD_BASEDIV4);
    m.attr("PCD_BASEDIV16") = pybind11::int_(PCD_BASEDIV16);
    m.attr("PCX_DEFAULT") = pybind11::int_(PCX_DEFAULT);
    m.attr("PFM_DEFAULT") = pybind11::int_(PFM_DEFAULT);
    m.attr("PICT_DEFAULT") = pybind11::int_(PICT_DEFAULT);
    m.attr("PNG_DEFAULT") = pybind11::int_(PNG_DEFAULT);
    m.attr("PNG_IGNOREGAMMA") = pybind11::int_(PNG_IGNOREGAMMA);
    m.attr("PNG_Z_BEST_SPEED") = pybind11::int_(PNG_Z_BEST_SPEED);
    m.attr("PNG_Z_DEFAULT_COMPRESSION") = pybind11::int_(PNG_Z_DEFAULT_COMPRESSION);
    m.attr("PNG_Z_BEST_COMPRESSION") = pybind11::int_(PNG_Z_BEST_COMPRESSION);
    m.attr("PNG_Z_NO_COMPRESSION") = pybind11::int_(PNG_Z_NO_COMPRESSION);
    m.attr("PNG_INTERLACED") = pybind11::int_(PNG_INTERLACED);
    m.attr("PNM_DEFAULT") = pybind11::int_(PNM_DEFAULT);
    m.attr("PNM_SAVE_RAW") = pybind11::int_(PNM_SAVE_RAW);
    m.attr("PNM_SAVE_ASCII") = pybind11::int_(PNM_SAVE_ASCII);
    m.attr("PSD_DEFAULT") = pybind11::int_(PSD_DEFAULT);
    m.attr("PSD_CMYK") = pybind11::int_(PSD_CMYK);
    m.attr("PSD_LAB") = pybind11::int_(PSD_LAB);
    m.attr("PSD_NONE") = pybind11::int_(PSD_NONE);
    m.attr("PSD_RLE") = pybind11::int_(PSD_RLE);
    m.attr("PSD_PSB") = pybind11::int_(PSD_PSB);
    m.attr("RAS_DEFAULT") = pybind11::int_(RAS_DEFAULT);
    m.attr("RAW_DEFAULT") = pybind11::int_(RAW_DEFAULT);
    m.attr("RAW_PREVIEW") = pybind11::int_(RAW_PREVIEW);
    m.attr("RAW_DISPLAY") = pybind11::int_(RAW_DISPLAY);
    m.attr("RAW_HALFSIZE") = pybind11::int_(RAW_HALFSIZE);
    m.attr("RAW_UNPROCESSED") = pybind11::int_(RAW_UNPROCESSED);
    m.attr("SGI_DEFAULT") = pybind11::int_(SGI_DEFAULT);
    m.attr("TARGA_DEFAULT") = pybind11::int_(TARGA_DEFAULT);
    m.attr("TARGA_LOAD_RGB888") = pybind11::int_(TARGA_LOAD_RGB888);
    m.attr("TARGA_SAVE_RLE") = pybind11::int_(TARGA_SAVE_RLE);
    m.attr("TIFF_DEFAULT") = pybind11::int_(TIFF_DEFAULT);
    m.attr("TIFF_CMYK") = pybind11::int_(TIFF_CMYK);
    m.attr("TIFF_PACKBITS") = pybind11::int_(TIFF_PACKBITS);
    m.attr("TIFF_DEFLATE") = pybind11::int_(TIFF_DEFLATE);
    m.attr("TIFF_ADOBE_DEFLATE") = pybind11::int_(TIFF_ADOBE_DEFLATE);
    m.attr("TIFF_NONE") = pybind11::int_(TIFF_NONE);
    m.attr("TIFF_CCITTFAX3") = pybind11::int_(TIFF_CCITTFAX3);
    m.attr("TIFF_CCITTFAX4") = pybind11::int_(TIFF_CCITTFAX4);
    m.attr("TIFF_LZW") = pybind11::int_(TIFF_LZW);
    m.attr("TIFF_JPEG") = pybind11::int_(TIFF_JPEG);
    m.attr("TIFF_LOGLUV") = pybind11::int_(TIFF_LOGLUV);
    m.attr("WBMP_DEFAULT") = pybind11::int_(WBMP_DEFAULT);
    m.attr("XBM_DEFAULT") = pybind11::int_(XBM_DEFAULT);
    m.attr("XPM_DEFAULT") = pybind11::int_(XPM_DEFAULT);
    m.attr("WEBP_DEFAULT") = pybind11::int_(WEBP_DEFAULT);
    m.attr("WEBP_LOSSLESS") = pybind11::int_(WEBP_LOSSLESS);
    m.attr("JXR_DEFAULT") = pybind11::int_(JXR_DEFAULT);
    m.attr("JXR_LOSSLESS") = pybind11::int_(JXR_LOSSLESS);
    m.attr("JXR_PROGRESSIVE") = pybind11::int_(JXR_PROGRESSIVE);
}

#endif // FREEIMAGE_WITH_PYTHON_BINDINGS
