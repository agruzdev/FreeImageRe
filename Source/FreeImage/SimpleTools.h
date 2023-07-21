//===========================================================
// FreeImage Re(surrected)
// Modified fork from the original FreeImage 3.18
// with updated dependencies and extended features.
//===========================================================

#ifndef FREEIMAGE_SIMPLE_TOOLS_H_
#define FREEIMAGE_SIMPLE_TOOLS_H_

#include "FreeImage.h"
#include <cmath>
#include <tuple>
#include "ConversionYUV.h"


template <typename DstPixel_, typename SrcPixel_ = DstPixel_, typename UnaryOperation_>
void BitmapTransform(FIBITMAP* dst, FIBITMAP* src, UnaryOperation_ unary_op)
{
	const unsigned width = FreeImage_GetWidth(src);
	const unsigned height = FreeImage_GetHeight(src);
	const unsigned src_pitch = FreeImage_GetPitch(src);
	const unsigned dst_pitch = FreeImage_GetPitch(dst);

	const uint8_t* src_bits = FreeImage_GetBits(src);
	uint8_t* dst_bits = FreeImage_GetBits(dst);

	for (unsigned y = 0; y < height; ++y) {
		auto src_pixel = static_cast<const SrcPixel_*>(static_cast<const void*>(src_bits));
		auto dst_pixel = static_cast<DstPixel_*>(static_cast<void*>(dst_bits));
		for (unsigned x = 0; x < width; ++x) {
			dst_pixel[x] = unary_op(src_pixel[x]);
		}
		src_bits += src_pitch;
		dst_bits += dst_pitch;
	}
}

template <typename Ty_>
using IsIntPixelType = std::integral_constant<bool,
    std::is_same_v<Ty_, FIRGB8> ||
    std::is_same_v<Ty_, FIRGBA8> ||
    std::is_same_v<Ty_, FIRGB16> ||
    std::is_same_v<Ty_, FIRGBA16> ||
    std::is_same_v<Ty_, FIRGB32> ||
    std::is_same_v<Ty_, FIRGBA32>
>;

template <typename Ty_>
using IsFloatPixelType = std::integral_constant<bool,
    std::is_same_v<Ty_, FIRGBF> ||
    std::is_same_v<Ty_, FIRGBAF>
>;

template <typename Ty_>
using IsPixelType = std::integral_constant<bool, IsIntPixelType<Ty_>::value || IsFloatPixelType<Ty_>::value>;





inline
bool IsNan(float p) {
    return std::isnan(p);
}

inline
bool IsNan(double p) {
    return std::isnan(p);
}

inline
bool IsNan(FIRGBF p) {
    return std::isnan(p.red) || std::isnan(p.green) || std::isnan(p.blue);
}

inline
bool IsNan(FIRGBAF p) {
    // Ignore alpha channel, since color can be computed without it...
    return std::isnan(p.red) || std::isnan(p.green) || std::isnan(p.blue);
}

template <typename Ty_>
inline constexpr
std::enable_if_t<std::is_integral_v<Ty_> || IsIntPixelType<Ty_>::value, bool> IsNan(Ty_ p)
{
    return false;
}



struct Brightness
{
    template <typename Ty_>
    inline
    auto operator()(const Ty_& p, std::enable_if_t<std::is_floating_point_v<Ty_> || std::is_integral_v<Ty_>, void*> = nullptr)
    {
        return p;
    }

    template <typename Ty_>
    inline
    auto operator()(const Ty_& p, std::enable_if_t<IsPixelType<Ty_>::value, void*> = nullptr)
    {
        return YuvJPEG::Y(p.red, p.green, p.blue);
    }
};

struct YuvBrightness
{
    template <typename Ty_>
    inline
    auto operator()(const Ty_& p, std::enable_if_t<IsPixelType<Ty_>::value, void*> = nullptr)
    {
        return p.red;
    }
};


template <typename PixelTy_, typename BrightnessOp_ = Brightness>
std::tuple<PixelTy_*, PixelTy_*, double, double> FindMinMax(FIBITMAP* src, BrightnessOp_ brightnessOp = BrightnessOp_{})
{
    PixelTy_* minIt = nullptr, * maxIt = nullptr;
    double minVal = 0.0, maxVal = 0.0;
    if (src) {
        const unsigned h = FreeImage_GetHeight(src);
        const unsigned w = FreeImage_GetWidth(src);
        const unsigned pitch = FreeImage_GetPitch(src);
        auto srcLine = static_cast<uint8_t*>(static_cast<void*>(FreeImage_GetBits(src)));
        for (unsigned j = 0; j < h; ++j, srcLine += pitch) {
            auto pixIt = static_cast<PixelTy_*>(static_cast<void*>(srcLine));
            for (unsigned i = 0; i < w; ++i, ++pixIt) {
                if (IsNan(*pixIt)) {
                    continue;
                }
                const auto b = static_cast<double>(brightnessOp(*pixIt));
                if (minIt == nullptr || maxIt == nullptr) {
                    minIt  = maxIt = pixIt;
                    minVal = maxVal = b;
                }
                else {
                    if (b < minVal) {
                        minIt = pixIt;
                        minVal = b;
                    }
                    if (maxVal < b) {
                        maxIt = pixIt;
                        maxVal = b;
                    }
                }
            }
        }
    }
    return std::make_tuple(minIt, maxIt, minVal, maxVal);
}



#endif //FREEIMAGE_SIMPLE_TOOLS_H_
