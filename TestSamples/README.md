# Public crash samples

`samples.csv` lists the public sample files that belong to the CVEs of [`SECURITY-CVE.md`](../SECURITY-CVE.md): where each file is published, its SHA-256 and whether the current version still crashes on it. The files themselves are not part of the repository; they stay where their authors published them, and `samples.py fetch` downloads them.

## Run

```bash
cmake -S . -B build-samples -DFREEIMAGE_BUILD_TESTS=OFF       -DCMAKE_C_FLAGS=-fsanitize=address -DCMAKE_CXX_FLAGS=-fsanitize=address
cmake --build build-samples --target FreeImage
python TestSamples/samples.py fetch                              # downloads into TestSamples/samples/
python TestSamples/samples.py run build-samples/lib/libFreeImage.so   # path of the library you built
```

Both subcommands take `--dir D` to use another sample folder. Nothing has to be built besides FreeImage itself: the script loads the already built shared library with `ctypes` (FreeImage is always built as a shared library) and calls `FreeImage_GetFileType`, `FreeImage_Load` and `FreeImage_Unload`.

Requirements: Python 3 (standard library only). Python must have the same bitness as the library (a 64-bit Python needs a 64-bit FreeImage). `fetch` extracts zip archives itself; for rar archives it needs one of `7z`, `unrar` or `bsdtar`.

With an AddressSanitizer build, use `/fsanitize=address` with MSVC; the directory with the sanitizer runtime (`clang_rt.asan_dynamic-*.dll`, e.g. `VC\Tools\MSVC\<version>\bin\Hostx64\x64`) must be on `PATH` so that it can be loaded into `python.exe`.

**The samples are crash files of other people's reports. Only open them with this script.**

`run` loads every sample in its own process, so a crash or a sanitizer abort ends only that child process. The child prints `loaded`, `rejected` or `unknown` (file type not recognised) and exits with 0; any other exit code (or a timeout of 300 s) is a crash. `run` compares the outcome with the `status` column:

| status | meaning |
|---|---|
| `fixed` | the file must be handled cleanly; a crash is a regression and counts in the exit code |
| `open` | the defect is known to be still present; a crash is reported but not counted; a line is printed when the file no longer crashes so that the manifest and `SECURITY-CVE.md` can be updated |
| `info` | the file is run and shown but not counted (it does not reproduce the reported defect) |

Some samples only show a problem on a 64-bit build (large allocations). Run them with a 64-bit build as well; `ASAN_OPTIONS=allocator_may_return_null=1` is set by default so that oversized requests are not fatal.

## Manifest

`samples.csv` is semicolon separated: `cve;sample;url;sha256;archive_member;format;status`. `sample` is the file name under `samples/`; for a file inside an archive `url` is the archive and `archive_member` the member, `sha256` is always the hash of the sample file itself. `samples.py fetch` refuses a file whose hash differs, so a changed or replaced download is noticed.

To add a sample: add a line with its CVE, the address where it is published (prefer an address pinned to a commit), the SHA-256, the format and the status, then update `SECURITY-CVE.md`.
