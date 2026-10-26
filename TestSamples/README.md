# Public crash samples

`samples.csv` lists the public sample files that belong to the CVEs of [`SECURITY-CVE.md`](../SECURITY-CVE.md): where each file is published, its SHA-256 and whether the current version still crashes on it. The files themselves are not part of the repository; they stay where their authors published them, and `fetch-samples.sh` downloads them.

**The samples are crash files of other people's reports. Do not open them with anything else than `run-samples.sh`.**

## Run

```bash
cmake -S . -B build-samples -DFREEIMAGE_BUILD_SAMPLE_LOADER=ON \
      -DCMAKE_C_FLAGS=-fsanitize=address -DCMAKE_CXX_FLAGS=-fsanitize=address
cmake --build build-samples
TestSamples/fetch-samples.sh                  # downloads into TestSamples/samples/
TestSamples/run-samples.sh build-samples/bin/LoadSample
```

With MSVC use `/fsanitize=address`; the sanitizer runtime (`clang_rt.asan_dynamic-*.dll`) must be on `PATH`. The scripts need bash (Linux, macOS, Git Bash on Windows), `curl`, `sha256sum` or `shasum`, and for samples inside archives one of `unzip`, `7z`, `unrar`, `bsdtar`.

`LoadSample` loads one file through the public C API and exits with 0 if the file was loaded, rejected or not recognised. A crash or a sanitizer abort ends the process with another code. `run-samples.sh` starts one process per file and compares the outcome with the `status` column:

| status | meaning |
|---|---|
| `fixed` | the file must be handled cleanly; a crash is a regression and counts in the exit code |
| `open` | the defect is known to be still present; a crash is reported but not counted; a line is printed when the file no longer crashes so that the manifest and `SECURITY-CVE.md` can be updated |
| `info` | the file is run and shown but not counted (it does not reproduce the reported defect) |

Some samples only show a problem on a 64-bit build (large allocations). Run them with a 64-bit build as well; `ASAN_OPTIONS=allocator_may_return_null=1` is set by default so that oversized requests are not fatal.

## Manifest

`samples.csv` is semicolon separated: `cve;sample;url;sha256;archive_member;format;status`. `sample` is the file name under `samples/`; for a file inside an archive `url` is the archive and `archive_member` the member, `sha256` is always the hash of the sample file itself. `fetch-samples.sh` refuses a file whose hash differs, so a changed or replaced download is noticed.

To add a sample: add a line with its CVE, the address where it is published (prefer an address pinned to a commit), the SHA-256, the format and the status, then update `SECURITY-CVE.md`.
