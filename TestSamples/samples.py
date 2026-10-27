#!/usr/bin/env python3
"""Fetches and runs the public crash samples listed in samples.csv (see README.md).

usage: samples.py fetch [--dir D]
       samples.py run <FreeImage library> [--dir D]

Python 3 standard library only. The samples are crash files of other people's reports: open them with this
script and nothing else.
"""
import argparse
import csv
import ctypes
import hashlib
import os
import shutil
import struct
import subprocess
import sys
import tempfile
import urllib.request
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
MANIFEST = os.path.join(HERE, 'samples.csv')
TIMEOUT = 300


def read_manifest():
    with open(MANIFEST, newline='', encoding='utf-8') as f:
        for row in csv.DictReader(f, delimiter=';'):
            if row['cve'] and not row['cve'].startswith('#'):
                yield row


def sha256_file(path):
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for chunk in iter(lambda: f.read(1 << 20), b''):
            h.update(chunk)
    return h.hexdigest()


def run_tool(cmd, out):
    """Runs a tool that writes the member to its standard output; the bytes go to the file untouched."""
    try:
        with open(out, 'wb') as f:
            r = subprocess.run(cmd, stdout=f, stderr=subprocess.DEVNULL, stdin=subprocess.DEVNULL, timeout=TIMEOUT)
    except (OSError, subprocess.SubprocessError):
        return False
    return r.returncode == 0 and os.path.getsize(out) > 0


def extract(archive, member, out):
    if zipfile.is_zipfile(archive):
        try:
            with zipfile.ZipFile(archive) as z, z.open(member) as src, open(out, 'wb') as dst:
                shutil.copyfileobj(src, dst)
            return True
        except (KeyError, OSError, zipfile.BadZipFile):
            return False
    # rar: first available external tool
    path = shutil.which('7z') or shutil.which('7za')
    if path:
        return run_tool([path, 'e', '-so', archive, member], out)
    path = shutil.which('unrar')
    if path:
        return run_tool([path, 'p', '-inul', archive, member], out)
    path = shutil.which('bsdtar')
    if path:
        # not "-O": on Windows bsdtar writes standard output in text mode; extract to a folder instead
        tmp = tempfile.mkdtemp()
        try:
            r = subprocess.run([path, '-xf', archive, '-C', tmp, member], stdout=subprocess.DEVNULL,
                               stderr=subprocess.DEVNULL, stdin=subprocess.DEVNULL, timeout=TIMEOUT)
            got = os.path.join(tmp, *member.split('/'))
            if r.returncode == 0 and os.path.isfile(got) and os.path.getsize(got) > 0:
                shutil.copyfile(got, out)
                return True
        except (OSError, subprocess.SubprocessError):
            pass
        finally:
            shutil.rmtree(tmp, ignore_errors=True)
    return False


def download(url, dest):
    req = urllib.request.Request(url, headers={'User-Agent': 'FreeImageRe-TestSamples'})
    with urllib.request.urlopen(req, timeout=120) as r, open(dest, 'wb') as f:
        shutil.copyfileobj(r, f)


def cmd_fetch(args):
    cache = os.path.join(args.dir, '.download')
    os.makedirs(cache, exist_ok=True)
    failed = fetched = present = 0
    for row in read_manifest():
        name, url, sha, member = row['sample'], row['url'], row['sha256'], row['archive_member']
        out = os.path.join(args.dir, name)
        if os.path.isfile(out) and sha256_file(out) == sha:
            present += 1
            continue
        if not url:
            print('no download address: %s (%s)' % (name, row['cve']))
            continue
        src = os.path.join(cache, hashlib.sha256(url.encode()).hexdigest()[:16])
        if not (os.path.isfile(src) and os.path.getsize(src) > 0):
            try:
                download(url, src)
            except Exception as e:
                print('FAILED  %s: download of %s (%s)' % (name, url, e))
                if os.path.exists(src):
                    os.remove(src)
                failed += 1
                continue
        if member:
            if not extract(src, member, out):
                print("FAILED  %s: cannot extract '%s' (zip: built in; rar: need 7z, unrar or bsdtar)" % (name, member))
                if os.path.exists(out):
                    os.remove(out)
                failed += 1
                continue
        else:
            shutil.copyfile(src, out)
        if sha256_file(out) != sha:
            print('FAILED  %s: SHA-256 differs from the manifest (the published file changed?)' % name)
            os.remove(out)
            failed += 1
            continue
        fetched += 1
    print('fetched %d, already present %d, failed %d' % (fetched, present, failed))
    return failed


def cmd_load(args):
    """Child process: loads one file through the public C API of the FreeImage shared library."""
    lib = os.path.abspath(args.lib)
    if os.name == 'nt':
        # winmode=0: search PATH too (sanitizer runtime DLL); stdcall only on 32-bit Windows
        if struct.calcsize('P') == 4:
            fi = ctypes.WinDLL(lib, winmode=0)
        else:
            fi = ctypes.CDLL(lib, winmode=0)
    else:
        fi = ctypes.CDLL(lib)
    fi.FreeImage_GetFileType.argtypes = [ctypes.c_char_p, ctypes.c_int]
    fi.FreeImage_GetFileType.restype = ctypes.c_int
    fi.FreeImage_Load.argtypes = [ctypes.c_int, ctypes.c_char_p, ctypes.c_int]
    fi.FreeImage_Load.restype = ctypes.c_void_p
    fi.FreeImage_Unload.argtypes = [ctypes.c_void_p]
    fi.FreeImage_Unload.restype = None
    path = os.fsencode(os.path.abspath(args.file))
    fif = fi.FreeImage_GetFileType(path, 0)
    if fif < 0:
        print('unknown')
        sys.stdout.flush()
        return 0
    dib = fi.FreeImage_Load(fif, path, 0)
    if dib:
        fi.FreeImage_Unload(dib)
        print('loaded')
    else:
        print('rejected')
    sys.stdout.flush()
    return 0


def first_line(text, *needles):
    for line in text.splitlines():
        if any(n in line for n in needles):
            return line.strip()
    return ''


def cmd_run(args):
    lib = os.path.abspath(args.lib)
    if not os.path.isfile(lib):
        print('library not found: %s' % args.lib, file=sys.stderr)
        return 2
    env = dict(os.environ)
    env.setdefault('ASAN_OPTIONS', 'allocator_may_return_null=1')
    regress = ok = open_crash = open_ok = skipped = 0
    for row in read_manifest():
        name, cve, status = row['sample'], row['cve'], row['status'].strip()
        path = os.path.join(args.dir, name)
        if not os.path.isfile(path):
            skipped += 1
            continue
        try:
            r = subprocess.run([sys.executable, '-I', os.path.abspath(__file__), 'load', lib, path],
                               stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                               env=env, timeout=TIMEOUT)
            code = r.returncode
            out = r.stdout.decode('utf-8', 'replace')
        except subprocess.TimeoutExpired:
            code, out = 'timeout', ''
        if status == 'info':
            print('info    %s (%s): exit %s %s' % (name, cve, code,
                  first_line(out, 'ERROR: AddressSanitizer', 'loaded', 'rejected', 'unknown')))
            continue
        if code == 0:
            if status == 'open':
                print('NOTE    %s (%s): no crash any more, manifest says open (%s)' % (name, cve, out.strip()))
                open_ok += 1
            else:
                ok += 1
        else:
            err = first_line(out, 'ERROR: AddressSanitizer', 'SUMMARY')
            if status == 'open':
                print('open    %s (%s): crash, exit %s %s' % (name, cve, code, err))
                open_crash += 1
            else:
                print('FAIL    %s (%s): crash, exit %s %s' % (name, cve, code, err))
                regress += 1
    print('ok %d, regressions %d, known open and still crashing %d, open but no longer crashing %d, not fetched %d'
          % (ok, regress, open_crash, open_ok, skipped))
    return regress


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest='cmd', required=True)
    f = sub.add_parser('fetch', help='download the samples of samples.csv')
    f.add_argument('--dir', default=os.path.join(HERE, 'samples'))
    r = sub.add_parser('run', help='load every fetched sample in its own process')
    r.add_argument('lib', help='the built FreeImage shared library')
    r.add_argument('--dir', default=os.path.join(HERE, 'samples'))
    ld = sub.add_parser('load')  # internal: one file in this process
    ld.add_argument('lib')
    ld.add_argument('file')
    args = p.parse_args()
    if hasattr(args, 'dir'):
        args.dir = os.path.abspath(args.dir)
    return {'fetch': cmd_fetch, 'run': cmd_run, 'load': cmd_load}[args.cmd](args)


if __name__ == '__main__':
    sys.exit(main())
