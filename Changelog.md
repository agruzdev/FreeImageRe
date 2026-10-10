# Changelog

All changes relative to FreeImage v3.18 are listed below.

## Version 4.2.2

Current project version. See the repository history for changes not listed in the release notes below.

## Version 4.2.1

- Fixed support of Android aarch64
- Fixed unstable downloading of WebP sources
- Added support of the MSVC ARM64 target
- Added support for compilation with system dependencies
- Added libaom v3.15.1
- Removed the dav1d and svtav1 dependencies
- Updated libde265 to v1.1.3
- Updated libheif to v1.23.5
- Updated highway to v1.4.0
- Updated imath to v3.2.3
- Updated jpeg to v10
- Updated jpeg-turbo to v3.2.0
- Updated jpegXL to v0.12.0
- Updated LCMS2 to v2.19.1
- Updated OpenEXR to v3.5.2
- Updated OpenJPH to v0.32.0
- Updated libpng to v1.6.59
- Updated libraw to v0.22.2

## Version 4.2.0

- Extended Plugin2 API to support opening multibitmap memory only once for all pages
- Fixed crashes due to plugin object lifetime
- Fixed backward compatible behaviour of the FreeImage_FIFSupports... functions in case of disabled plugins
- Error-safe writing of output files in FreeImage_Save
- Updated turbojpeg to v3.1.4.1
- Updated jpegxl to v0.11.2
- Updated OpenEXR to v3.4.10
- Updated OpenJPH to v0.27.0
- Updated libpng to v1.6.58
- Updated libraw to v0.22.1

## Version 4.1.1

- Updated zlib to v1.3.2
- Updated LibPNG to v1.6.55
- PluginTIFF: fixed wrongly disabled ICC for CMYK without conversion
- PluginHEIF supports writing raw Exif
- CMake configuration supports Android build

## Version 4.1.0

- New plugin support for `.jxl` format, OpenXL
- Added libjxl v0.11.1
- Added brotli v1.2.0
- Added highway v1.3.0
- Added Little-CMS v2.18, at hash 6ae7e97c
- Fixed macOS compilation
- Fixed read-after-free error in PluginTIFF
- Fixed backward compatible behaviour of FreeImage_GetFIFCount()
- More accurate refcounting and deinitialization
- Updated version macro in FreeImage.h

## Version 4.0.0

- New versioning: FreeImageRe 4.0 as next step after FreeImage 3.18
- Added support of extra TIFF image formats
- Added support for opening FIMULTIBITMAP from Unicode path
- Added support for 2-level dependencies info reporting
- Added new version of API for processing diagnostic messages
- Fixed infinite loop in TIFF thumbnail loading
- Fixed multiple CVEs in TIFF, RAS, ICO, HDR, PSD, XBM, EXR, JXR plugins
- Fixed compilation for x32
- Updated jpeg-turbo to v3.1.3
- Updated OpenEXR to v3.4.4
- Updated LibPNG to v1.6.54
- Updated LibRaw to v0.22.0
- Updated LibDav1d to v1.5.3

## Version 0.5

- Updated LibDE265 to v1.0.16
- Updated jpeg-turbo to v3.1.2
- Updated LibKvazaar to v2.3.2
- Updated OpenEXR to v3.3.5
- Updated OpenJPEG to v2.5.4
- Updated LibPNG to v1.6.50
- Updated LibSvtav1 to v3.1.2
- Updated LibTIFF to v4.7.1
- Updated LibWebP to v1.6.0
- Updated LibHEIF to v1.20.2

## Version 0.4

- Building with libjpeg-turbo by default
- Introduced a new Plugin2 API for plugins with state
- Added limited support for HEIC and AVIF formats
- Extended FIF_* enums range and added function for mapping FIF index to FIF value
- Updated jpeg-turbo to v3.1.0
- Updated OpenEXR to v3.3.3
- Updated OpenJPEG to v2.5.3
- Updated LibPNG to v1.6.48
- Updated LibRaw to v0.21.4
- Updated LibWebP to v1.5.0
- Updated LibSvtav1 to v3.0.2
- Updated LibDav1d to v1.5.1
- Updated LibKvazaar to v2.3.1
- Updated LibDE265 to v1.0.15
- Updated LibHEIF to v1.19.7

## Version 0.3

- Fixed the vulnerabilities: CVE-2021-33367, CVE-2023-47992, CVE-2023-47993, CVE-2023-47994, CVE-2023-47995, CVE-2023-47996, CVE-2023-47997
- Added API for querying versions of compiled dependencies
- Added Python 3 bindings
- Ability to enable or disable each image library dependency
- FreeImage_ConvertToRGBF supports FI_DOUBLE input
- Limited support of 2-bit bitmaps
- Updated OpenEXR to v3.3.0
- Updated LibPNG to v1.6.44
- Updated jpeg-turbo to v3.0.4
- Updated LibTIFF to v4.7.0
- Updated LibWebP to v1.4.0
- Updated LibRaw to v0.21.3

## Version 0.2

- Removed Windows datatypes to avoid collisions
- Added C++ wrappers
- Added basic support for YUV images
- Added basic support for Float32 complex images
- Added function FreeImage_ConvertToColor
- Added functions FreeImage_FindMinMax and FreeImage_FindMinMaxValue
- Added function FreeImage_TmoClamp and corresponding enum FITMO_CLAMP
- Added function FreeImage_TmoLinear and corresponding enum FITMO_LINEAR
- Added function FreeImage_DrawBitmap
- Added function FreeImage_GetColorType2
- Added function FreeImage_MakeHistogram
- Updated zlib to v1.3.1
- Updated OpenEXR to v3.2.2
- Updated OpenJPEG to v2.5.2
- Updated LibJPEG to jpeg-9f
- Updated LibPNG to v1.6.43
- Updated LibTIFF to v4.6.0
- Updated LibWebP to v1.3.2
- Updated LibRaw to v0.21.2

## Version 0.1

- Compilation fix for FREEIMAGE_COLORORDER_RGB
- Export Utility.h functions from DLL
- Linking image format dependencies as static libraries
- Updated zlib to v1.2.13
- Updated OpenEXR to v3.1.4
- Updated OpenJPEG to v2.5.0 (alternatively JPEG-turbo v2.1.4)
- Updated LibPNG to v1.6.37
- Updated LibTIFF to v4.4.0
- Updated LibWebP to v1.2.4
- Updated LibRaw to v0.20.0
- PluginTIFF fixed to read images with packed bits
- Minimalistic support of RGB(A) 32 bits per channel for loading and conversion
- Added functions FreeImageRe_GetVersion() and FreeImageRe_GetVersionNumbers()
