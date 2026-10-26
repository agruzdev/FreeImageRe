#!/usr/bin/env bash
# Loads every sample of samples.csv in its own process and compares the outcome with the manifest.
#
# usage: run-samples.sh [loader] [sample-dir]
#   loader      LoadSample executable (cmake -DFREEIMAGE_BUILD_SAMPLE_LOADER=ON); default: found below build*/
#   sample-dir  folder filled by fetch-samples.sh; default: samples/ next to this script
#
# A sample "crashes" if the loader exits with a non-zero code or a signal (with an AddressSanitizer build, on
# every memory error). Build with -fsanitize=address (or /fsanitize=address) to see errors that do not crash.
# status in samples.csv:
#   fixed  the current version must load or reject the file cleanly; a crash is a regression (counted)
#   info   the file is run and shown but never counted (it does not reproduce the reported defect)
#   open   the defect is known to be still present; a crash is reported, not counted. If the file no longer crashes,
#          the manifest line is outdated.
# Samples that were not fetched are skipped. Exit code: number of regressions.

set -u
here=$(cd "$(dirname "$0")" && pwd)
loader=${1:-}
dir=${2:-$here/samples}
manifest=$here/samples.csv

if [ -z "$loader" ]; then
    for c in "$here"/../build*/bin/LoadSample "$here"/../build*/bin/LoadSample.exe \
             "$here"/../build*/bin/*/LoadSample.exe "$here"/../build*/bin/*/LoadSample; do
        [ -x "$c" ] && loader=$c && break
    done
fi
[ -x "$loader" ] || { echo "LoadSample not found; build with -DFREEIMAGE_BUILD_SAMPLE_LOADER=ON and pass it as first argument" >&2; exit 2; }
loader=$(cd "$(dirname "$loader")" && pwd)/$(basename "$loader")
export ASAN_OPTIONS=${ASAN_OPTIONS:-allocator_may_return_null=1}

limit=()
command -v timeout >/dev/null 2>&1 && limit=(timeout 300)

regress=0 ok=0 open_crash=0 open_ok=0 skipped=0
while IFS=';' read -r cve sample url sha archive format status note; do
    status=${status%$'\r'}
    case $cve in ''|\#*|cve) continue ;; esac
    file=$dir/$sample
    if ! [ -f "$file" ]; then skipped=$((skipped+1)); continue; fi
    out=$(${limit[@]+"${limit[@]}"} "$loader" "$file" 2>&1 < /dev/null); code=$?
    if [ "$status" = info ]; then echo "info    $sample ($cve): exit $code $(printf "%s" "$out" | grep -m1 -E "ERROR: AddressSanitizer|loaded|rejected|unknown")"; continue; fi
    if [ $code -eq 0 ]; then
        if [ "$status" = open ]; then
            echo "NOTE    $sample ($cve): no crash any more, manifest says open ($out)"; open_ok=$((open_ok+1))
        else
            ok=$((ok+1))
        fi
    else
        err=$(printf '%s\n' "$out" | grep -m1 -E 'ERROR: AddressSanitizer|SUMMARY' || true)
        if [ "$status" = open ]; then
            echo "open    $sample ($cve): crash, exit $code ${err}"; open_crash=$((open_crash+1))
        else
            echo "FAIL    $sample ($cve): crash, exit $code ${err}"; regress=$((regress+1))
        fi
    fi
done < "$manifest"

echo "ok $ok, regressions $regress, known open and still crashing $open_crash, open but no longer crashing $open_ok, not fetched $skipped"
exit "$regress"
