//===========================================================
// FreeImage Re(surrected)
// Modified fork from the original FreeImage 3.18
// with updated dependencies and extended features.
//===========================================================

#include "FreeImage.h"

#ifndef FREEIMAGE_SIMPLE_TOOLS_H_
#define FREEIMAGE_SIMPLE_TOOLS_H_

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

#endif //FREEIMAGE_SIMPLE_TOOLS_H_
