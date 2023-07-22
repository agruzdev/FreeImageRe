//===========================================================
// FreeImage Re(surrected)
// Modified fork from the original FreeImage 3.18
// with updated dependencies and extended features.
//===========================================================

#include "SimpleTools.h"



FIBOOL FreeImage_FindMinMax(FIBITMAP* dib, double* min_brightness, double* max_brightness, void** min_ptr, void** max_ptr)
{
	if (!FreeImage_HasPixels(dib)) {
		return FALSE;
	}
	std::tuple<void*, void*, double, double> res{};
	bool success = false;

	const auto colorType = FreeImage_GetColorType(dib);
	
	switch (FreeImage_GetImageType(dib))
	{
	case FIT_BITMAP: {		
			const auto bpp = FreeImage_GetBPP(dib);
			if (bpp == 32) {
				if (colorType == FIC_RGBALPHA) {
					res = FindMinMax<FIRGBA8>(dib);
					success = true;
				}
				else if (colorType == FIC_YUV) {
					res = FindMinMax<FIRGBA8, YuvBrightness>(dib);
					success = true;
				}
			}
			else if (bpp == 24) {
				if (colorType == FIC_RGB) {
					res = FindMinMax<FIRGB8>(dib);
					success = true;
				}
				else if (colorType == FIC_YUV) {
					res = FindMinMax<FIRGB8, YuvBrightness>(dib);
					success = true;
				}
			}
			else if (bpp == 8) {
				if (colorType == FIC_MINISBLACK) {
					res = FindMinMax<uint8_t>(dib);
					success = true;
				}
			}
		}		
		break;
	case FIT_RGBAF:
		if (colorType == FIC_RGBALPHA) {
			res = FindMinMax<FIRGBAF>(dib);
			success = true;
		}
		else if (colorType == FIC_YUV) {
			res = FindMinMax<FIRGBAF, YuvBrightness>(dib);
			success = true;
		}
		break;
	case FIT_RGBF:
		if (colorType == FIC_RGB) {
			res = FindMinMax<FIRGBF>(dib);
			success = true;
		}
		else if (colorType == FIC_YUV) {
			res = FindMinMax<FIRGBF, YuvBrightness>(dib);
			success = true;
		}
		break;
	case FIT_RGBA32:
		res = FindMinMax<FIRGBA32>(dib);
		success = true;
		break;
	case FIT_RGB32:
		res = FindMinMax<FIRGB32>(dib);
		success = true;
		break;
	case FIT_RGBA16:
		res = FindMinMax<FIRGBA16>(dib);
		success = true;
		break;
	case FIT_RGB16:
		res = FindMinMax<FIRGB16>(dib);
		success = true;
		break;
	case FIT_DOUBLE:
		res = FindMinMax<double>(dib);
		success = true;
		break;
	case FIT_FLOAT:
		res = FindMinMax<float>(dib);
		success = true;
		break;
	case FIT_UINT32:
		res = FindMinMax<uint32_t>(dib);
		success = true;
		break;
	case FIT_INT32:
		res = FindMinMax<int32_t>(dib);
		success = true;
		break;
	case FIT_UINT16:
		res = FindMinMax<uint16_t>(dib);
		success = true;
		break;
	case FIT_INT16:
		res = FindMinMax<int32_t>(dib);
		success = true;
		break;
	default:
		break;
	}

	if (success) {
		if (min_brightness) {
			*min_brightness = std::get<2>(res);
		}
		if (max_brightness) {
			*max_brightness = std::get<3>(res);
		}
		if (min_ptr) {
			*min_ptr = std::get<0>(res);
		}
		if (max_ptr) {
			*max_ptr = std::get<1>(res);
		}
	}

	return success ? TRUE : FALSE;
}

