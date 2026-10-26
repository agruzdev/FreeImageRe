#!/usr/bin/env bash
# Downloads the public sample files listed in samples.csv from where they are published.
#
# usage: fetch-samples.sh [sample-dir]      (default: samples/ next to this script)
#
# The samples are crash files of other people's reports. They are not stored in the repository, only their
# download address and SHA-256 are. A file is accepted only if its SHA-256 matches the manifest. Open them with
# nothing else than run-samples.sh.
# Needs: bash, curl, sha256sum (or shasum), and for samples inside archives one of bsdtar, 7z, unzip, unrar.
# Exit code: number of samples that could not be fetched or did not match.

set -u
here=$(cd "$(dirname "$0")" && pwd)
dir=${1:-$here/samples}
manifest=$here/samples.csv
mkdir -p "$dir/.download"

sha256() {
    if command -v sha256sum >/dev/null 2>&1; then sha256sum "$1" | cut -d' ' -f1
    else shasum -a 256 "$1" | cut -d' ' -f1; fi
}

# extract <archive> <member> <output>
extract() {
    # not "bsdtar -O": on Windows it writes standard output in text mode and changes the bytes
    if command -v unzip >/dev/null 2>&1 && unzip -p "$1" "$2" > "$3" 2>/dev/null && [ -s "$3" ]; then return 0; fi
    if command -v 7z >/dev/null 2>&1 && 7z e -so "$1" "$2" > "$3" 2>/dev/null && [ -s "$3" ]; then return 0; fi
    if command -v unrar >/dev/null 2>&1 && unrar p -inul "$1" "$2" > "$3" 2>/dev/null && [ -s "$3" ]; then return 0; fi
    if command -v bsdtar >/dev/null 2>&1; then
        tmp=$(mktemp -d) || return 1
        if bsdtar -xf "$1" -C "$tmp" "$2" 2>/dev/null && [ -s "$tmp/$2" ]; then cp "$tmp/$2" "$3"; rm -rf "$tmp"; return 0; fi
        rm -rf "$tmp"
    fi
    return 1
}

failed=0 fetched=0 skipped=0
while IFS=';' read -r cve sample url sha archive format status note; do
    status=${status%$'\r'}
    case $cve in ''|\#*|cve) continue ;; esac
    out=$dir/$sample
    if [ -f "$out" ] && [ "$(sha256 "$out")" = "$sha" ]; then skipped=$((skipped+1)); continue; fi
    if [ -z "$url" ]; then
        echo "no download address: $sample ($cve)"; continue
    fi
    src=$dir/.download/$(printf '%s' "$url" | { sha256sum 2>/dev/null || shasum -a 256; } | cut -c1-16)
    if ! [ -s "$src" ] && ! curl -fsSL --retry 2 -o "$src" "$url" < /dev/null; then
        echo "FAILED  $sample: download of $url"; rm -f "$src"; failed=$((failed+1)); continue
    fi
    if [ -n "$archive" ]; then
        if ! extract "$src" "$archive" "$out"; then
            echo "FAILED  $sample: cannot extract '$archive' (need bsdtar, 7z, unzip or unrar)"; rm -f "$out"
            failed=$((failed+1)); continue
        fi
    else
        cp "$src" "$out"
    fi
    if [ "$(sha256 "$out")" != "$sha" ]; then
        echo "FAILED  $sample: SHA-256 differs from the manifest (the published file changed?)"; rm -f "$out"
        failed=$((failed+1)); continue
    fi
    fetched=$((fetched+1))
done < "$manifest"

echo "fetched $fetched, already present $skipped, failed $failed"
exit "$failed"
