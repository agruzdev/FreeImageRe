# FreeImageRe - FreeImage Re(surrected) Fork

FreeImageRe is a maintained fork of the [FreeImage project](https://freeimage.sourceforge.io/). It updates the original FreeImage 3.18 library for modern compilers and dependency versions while preserving binary compatibility with the FreeImage 3.18 dynamic library. This fork also includes vulnerability fixes and small API extensions.

## About

FreeImageRe is a C/C++ image codec library, bitmap library, image loading library, image conversion library and FreeImage-compatible replacement. It hides a wide range of image-codec backends and platform-specific dependencies behind one stable, universal C API. Applications use the same API for loading, saving and converting images while FreeImageRe handles the underlying codec libraries and their versions.


## Licensing

FreeImageRe uses the original FreeImage [dual license](https://freeimage.sourceforge.io/license.html): the FreeImage Public License (FIPL) or GNU GPL v2. Third-party dependencies have their own licenses.

## Supported platforms

The following configurations are built by the GitHub Actions workflow:

| Platform | Architecture | Toolchain | CI status | Configure preset or method |
|---|---|---|---|---|
| Windows | x64 | MSVC | Tested in CI | `msvc-release` or `msvc-debug` |
| Windows | ARM64 | MSVC | Tested in CI | CMake with `-A ARM64` |
| Ubuntu | x64 | GCC/G++ | Tested in CI | `linux-release` |
| Android | `arm64-v8a` | Android NDK | Tested in CI | `android-release-unix` |
| macOS | Native architecture | CMake and a C++20 compiler | N/A | `linux-release` or equivalent Unix generator |

The presets are defined in [`CMakePresets.json`](CMakePresets.json). The Android preset requires `ANDROID_NDK_HOME` and targets Android platform `28`.

### Windows

```powershell
cmake --preset msvc-release
cmake --build build-vs18-release --config Release
```

### Linux

```bash
cmake --preset linux-release
cmake --build build-linux-release
```

### Android

```bash
export ANDROID_NDK_HOME=/path/to/android-ndk
cmake --preset android-release-unix
cmake --build build-android-release
```

## Supported formats

Formats supported in FreeImageRe:

| Format | `FREE_IMAGE_FORMAT` | Library | Extensions |
|---|---|---|---|
| Windows Bitmap | `FIF_BMP` | Built-in plugin | `.bmp`, `.dib` |
| Icon / Cursor | `FIF_ICO` | Built-in plugin | `.ico`, `.cur` |
| JPEG | `FIF_JPEG` | [libjpeg-turbo](https://github.com/libjpeg-turbo/libjpeg-turbo) or [libjpeg](https://www.ijg.org) | `.jpg`, `.jpeg`, `.jpe`, `.jif`, `.jfif` |
| JPEG Network Graphics | `FIF_JNG` | Not supported | `.jng` |
| C64 Koala | `FIF_KOALA` | Built-in plugin | `.koa` |
| Amiga IFF / LBM | `FIF_LBM`, `FIF_IFF` | Built-in plugin | `.iff`, `.lbm` |
| Multiple-image Network Graphics | `FIF_MNG` | Built-in plugin | `.mng` |
| Portable Bitmap | `FIF_PBM` | Built-in plugin | `.pbm` |
| Portable Bitmap, raw | `FIF_PBMRAW` | Built-in plugin | `.pbm` |
| Kodak PhotoCD | `FIF_PCD` | Built-in plugin | `.pcd` |
| PCX | `FIF_PCX` | Built-in plugin | `.pcx` |
| Portable Graymap | `FIF_PGM` | Built-in plugin | `.pgm` |
| Portable Graymap, raw | `FIF_PGMRAW` | Built-in plugin | `.pgm` |
| PNG | `FIF_PNG` | [libpng](https://github.com/pnggroup/libpng) | `.png` |
| Portable Pixmap | `FIF_PPM` | Built-in plugin | `.ppm` |
| Portable Pixmap, raw | `FIF_PPMRAW` | Built-in plugin | `.ppm` |
| Sun Raster | `FIF_RAS` | Built-in plugin | `.ras` |
| TARGA / Truevision TGA | `FIF_TARGA` | Built-in plugin | `.tga`, `.targa` |
| TIFF | `FIF_TIFF` | [libtiff](http://download.osgeo.org/libtiff/) | `.tif`, `.tiff` |
| Wireless Bitmap | `FIF_WBMP` | Built-in plugin | `.wbmp` |
| Adobe Photoshop | `FIF_PSD` | Built-in plugin | `.psd` |
| Dr. Halo CUT | `FIF_CUT` | Built-in plugin | `.cut` |
| X Bitmap | `FIF_XBM` | Built-in plugin | `.xbm` |
| X PixMap | `FIF_XPM` | Built-in plugin | `.xpm` |
| DirectDraw Surface | `FIF_DDS` | Built-in plugin | `.dds` |
| Graphics Interchange Format | `FIF_GIF` | Built-in plugin | `.gif` |
| Radiance HDR | `FIF_HDR` | Built-in plugin | `.hdr` |
| Fax Group 3 | `FIF_FAXG3` | Built-in plugin | `.g3` |
| SGI | `FIF_SGI` | Built-in plugin | `.sgi`, `.rgb`, `.rgba`, `.bw` |
| OpenEXR | `FIF_EXR` | [OpenEXR](https://github.com/AcademySoftwareFoundation/openexr) | `.exr` |
| JPEG 2000 codestream | `FIF_J2K` | [OpenJPEG](https://github.com/uclouvain/openjpeg) | `.j2k`, `.j2c` |
| JPEG 2000 JP2 | `FIF_JP2` | [OpenJPEG](https://github.com/uclouvain/openjpeg) | `.jp2` |
| Portable FloatMap | `FIF_PFM` | Built-in plugin | `.pfm` |
| Macintosh PICT | `FIF_PICT` | Built-in plugin | `.pct`, `.pict` |
| Camera RAW | `FIF_RAW` | [LibRaw](https://github.com/LibRaw/LibRaw) | `.raw`, `.crw`, `.cr2`, `.cr3`, `.nef`, `.nrw`, `.arw`, `.dng`, `.orf`, `.rw2`, `.pef`, `.raf`, `.srw`, `.rwl`, `.mrw`, `.kdc`, `.dcr`, `.x3f`, `.iiq`, `.3fr` |
| WebP | `FIF_WEBP` | [libwebp](https://chromium.googlesource.com/webm/libwebp) | `.webp` |
| JPEG-XR / HD Photo | `FIF_JXR` | Internal LibJXR | `.jxr`, `.wdp`, `.hdp` |
| HEIF / HEIC | `FIF_HEIF` | [libheif](https://github.com/strukturag/libheif) | `.heif`, `.heifs`, `.heic`, `.heics` |
| AVIF | `FIF_AVIF` | [libheif](https://github.com/strukturag/libheif) | `.avif` |
| JPEG XL | `FIF_JPEGXL` | [libjxl](https://github.com/libjxl/libjxl) | `.jxl` |

**Runtime dependency:** `libheif` is linked as a shared library. Applications using HEIF or AVIF must make `heif.dll` available on Windows or `libheif.so` available on Linux.

## Python bindings

To build the Python bindings, configure CMake with `-DFREEIMAGE_WITH_PYTHON_BINDINGS=ON`. Python development files and Numpy are required.

After installation:

- On Windows, make a link from `FreeImage.pyd` to `FreeImage.dll` (a hard link is generated automatically when built from sources).
- On Linux, make a link from `FreeImage.so` to `libFreeImage.so` (a symbolic link is generated automatically when built from sources).
- Make sure the link and library are available on `PYTHONPATH` ([Python documentation](https://docs.python.org/3/extending/building.html)).

```python
import numpy as np
import FreeImage as fi

image = fi.load("input.jpg", fi.JPEG_EXIFROTATE)
image, image_format = fi.loadf("input.jpg")

pixels = np.zeros((128, 128), dtype=np.float32)
fi.save(fi.FIF_EXR, pixels, "output.exr")
```

`load()` returns an image as a NumPy array. `loadf()` returns an image and the detected format enum. `save()` accepts 2D or 3D NumPy arrays.

## Changes

All changes relative to FreeImage 3.18 are documented in [`Changelog.md`](Changelog.md).

