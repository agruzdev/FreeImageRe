## FreeImage Re(surrected)

Fork of [the FreeImage project](https://freeimage.sourceforge.io/) in order to support FreeImage library for modern compilers and dependencies versions.

Also small extensions and fixes can be added.

The dynamic library is binary compatible with FreeImage 3.18 and can replace it.


### Licensing

Same to the original FreeImage [dual license](https://freeimage.sourceforge.io/license.html).

All changes are described below in this file.


### What's new

Changes made to FreeImage v3.18:

Version 0.1:
 - Compilation fix for FREEIMAGE_COLORORDER_RGB
 - Export Utility.h functions from DLL
 - Linking image formt dependencies as static libs
 - Updated zlib till v1.2.13
 - Updated OpenEXR till v3.1.4
 - Updated OpenJPEG till v2.5.0 (alternatively JPEG-turbo v2.1.4)
 - Updated LibPNG till v1.6.37
 - Updated LibTIFF till v4.4.0
 - Updated LibWebP till v1.2.4
 - Updated LibRaw till v0.20.0
 - PluginTIFF fixed to read images with packed bits
 - Minimalistic support of RGB(A) 32bits per channel for loading and conversion
 - Added functions FreeImageRe_GetVersion() and FreeImageRe_GetVersionNumbers()

Version 0.2:
 - Removed Windows datatypes to avoid collisions
 - Added C++ wrappers
 - Added basic support for YUV images
 - Added basic support for Float32 complex images
 - Added function FreeImage_ConvertToColor
 - Added function FreeImage_FindMinMax
 - Added function FreeImage_TmoClamp and corresponding enum FITMO_CLAMP
 - Added function FreeImage_TmoLinear and corresponding enum FITMO_LINEAR
 - Added function FreeImage_DrawBitmap
 - Added function FreeImage_GetColorType2

