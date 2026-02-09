## FreeImage Re(surrected)

Fork of [the FreeImage project](https://freeimage.sourceforge.io/) in order to support FreeImage library for modern compilers and dependencies versions.

Also small extensions and fixes can be added.

The dynamic library is binary compatible with FreeImage 3.18 and can replace it.


### Licensing

Same to the original FreeImage [dual license](https://freeimage.sourceforge.io/license.html).

All changes are described below in this file.


### Python bindings

To import FreeImage python package do the following steps:

* On Windows make link FreeImage.pyd ==> FreeImage.dll     (a hard link is generated automatically when built from sources)
* On Linux make link FreeImage.so ==> libFreeImage.so    (a symbolic link is generated automatically when built from sources)
* Make sure the link and library is available on PYTHONPATH (https://docs.python.org/3/extending/building.html)

```python

import FreeImage as fi

img = fi.load(r"myimage.jpg", fi.JPEG_EXIFROTATE) # Loads as numpy array
...

img, format = fi.loadf(r"myimage.bin") # Returns image and format enum deduced from name or file data
...

import numpy
zero = numpy.zeros((128, 128), dtype=numpy.float32)
fi.save(fi.FIF_EXR, zero, "zero.exr")  # Accepts 2D or 3D numpy arrays
...


```


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
 - Added function FreeImage_FindMinMax and FreeImage_FindMinMaxValue
 - Added function FreeImage_TmoClamp and corresponding enum FITMO_CLAMP
 - Added function FreeImage_TmoLinear and corresponding enum FITMO_LINEAR
 - Added function FreeImage_DrawBitmap
 - Added function FreeImage_GetColorType2
 - Added function FreeImage_MakeHistogram
 - Updated zlib till v1.3.1
 - Updated OpenEXR till v3.2.2
 - Updated OpenJPEG till v2.5.2
 - Updaged LibJPEG till jpeg-9f
 - Updated LibPNG till v1.6.43
 - Updated LibTIFF till v4.6.0
 - Updated LibWebP till v1.3.2
 - Updated LibRaw till v0.21.2

Version 0.3:
 - Fixed the vulnerabilities: CVE-2021-33367, CVE-2023-47992, CVE-2023-47993, CVE-2023-47994, CVE-2023-47995, CVE-2023-47996, CVE-2023-47997
 - Added API for querying versions of compiled dependencies
 - Added Python3 bindings
 - Ability to enable/disable each image library dependency
 - FreeImage_ConvertToRGBF supports FI_DOUBLE input
 - Limited support of 2bit bitmaps
 - Updated OpenEXR till v3.3.0
 - Updated LibPNG till v1.6.44
 - Updated jpeg-turbo till v3.0.4
 - Updated LibTIFF till v4.7.0
 - Updated LibWebP till v1.4.0
 - Updated LibRaw till v0.21.3

Version 0.4:
 - Building with libjpeg-turbo by default
 - Introduced a new Plugin2 API for plugins with state
 - Added a limited support for HEIC and AVIF formats
 - Extended FIF_* enums range and added function for mapping FIF index to FIF value
 - Updated jpeg-turbo till v3.1.0
 - Updated OpenEXR till v3.3.3
 - Updated OpenJPEG till v2.5.3
 - Updated LibPNG till v1.6.48
 - Updated LibRaw till v0.21.4
 - Updated LibWebP till v1.5.0
 - Updated LibSvtav1 till v3.0.2
 - Updated LibDav1d till v1.5.1
 - Updated LibKvazaar till v2.3.1
 - Updated LibDE265 till v1.0.15
 - Updated LibHEIF till v1.19.7

Version 0.5:
 - Updated LibDE265 till v1.0.16
 - Updated jpeg-turbo till v3.1.2
 - Updated LibKvazaar till v2.3.2
 - Updated OpenEXR till v3.3.5
 - Updated OpenJPEG till v2.5.4
 - Updated LibPNG till v1.6.50
 - Updated LibSvtav1 till v3.1.2
 - Updated LibTIFF till v4.7.1
 - Updated LibWebP till v1.6.0
 - Updated LibHEIF till v1.20.2

Version 4.0.0:
 - New versioning: FreeImageRe 4.0 as next step after FreeImage 3.18
 - Added support of extra TIFF image formats
 - Added support for opening FIMULTIBITMAP from Unicode path
 - Added support for 2-level dependencies info reporting
 - Added new version of API for processing diagnostic messages
 - Fixed infinite loop in TIFF thumbnail loading
 - Fixed multiple CVEs in TIFF, RAS, ICO, HDR, PSD, XBM, EXR, JXR plugins.
 - Fixed compilation for x32
 - Updated jpeg-turbo till v3.1.3
 - Updated OpenEXR till v3.4.4
 - Updated LibPNG till v1.6.54
 - Updated LibRaw till v0.22.0
 - Updated LibDav1d till v1.5.3

Version 4.1.0:
 - New plugin support .jxl format, OpenXL
 - Added libjxl v0.11.1
 - Added brotli v1.2.0
 - Added highway v1.3.0
 - Added Little-CMS v2.18, at hash 6ae7e97c
 - Fixed MacOX compilation
 - Fixed read-after-free error in PluginTIFF
 - Fixed backward compatible behaviour of FreeImage_GetFIFCount()
 - More accurate refcounting and deinitialization
 - Updated version macro in FreeImage.h


