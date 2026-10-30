#!/usr/bin/env python3
"""Exercise the real eMMC open/ioctl functions and XNU's spec_open geometry path."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys


def block(text, start):
    begin = text.index('{', start)
    depth = 1
    end = begin + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return end


def function(text, name):
    signature = re.search(r'^' + re.escape(name) + r'\(', text, re.M)
    if signature is None:
        raise SystemExit(f'missing driver function {name}')
    start = text.rfind('\nstatic ', 0, signature.start()) + 1
    return text[start:block(text, signature.start())]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cc', default=os.environ.get('CC') or shutil.which('clang') or 'cc')
    args = parser.parse_args()
    driver_path = args.source / 'bsd/dev/whyred_emmc_pio_probe.c'
    spec_path = args.source / 'bsd/miscfs/specfs/spec_vnops.c'
    driver = driver_path.read_text()
    spec = spec_path.read_text()
    spec_start = spec.index('spec_open(struct vnop_open_args *ap)')
    start = spec.index('if (!VNOP_IOCTL(vp, DKIOCGETBLOCKSIZE,', spec_start)
    geometry = spec[start:block(spec, start)]
    harness = r'''
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#ifndef __unused
#define __unused __attribute__((unused))
#endif
typedef uint32_t whyred_test_dev_t;
#define dev_t whyred_test_dev_t
#define caddr_t char *
#define u_long unsigned long
#define minor(d) ((d) & 0xffffffU)
#define FREAD 1
#define FWRITE 2
#define WHYRED_GPT_MAX_ENTRIES 64
#define WHYRED_GPT_BLOCK_BYTES 512
enum {
    DKIOCGETBLOCKSIZE = 1, DKIOCGETPHYSICALBLOCKSIZE, DKIOCSETBLOCKSIZE,
    DKIOCGETBLOCKCOUNT, DKIOCISWRITABLE, DKIOCGETMAXBLOCKCOUNTREAD,
    DKIOCGETMAXSEGMENTCOUNTREAD, DKIOCGETMAXBLOCKCOUNTWRITE,
    DKIOCGETMAXSEGMENTCOUNTWRITE
};
struct proc { int unused; };
struct whyred_gpt_partition { uint32_t index; uint64_t first, last; };
static struct whyred_gpt_partition whyred_gpt_parts[WHYRED_GPT_MAX_ENTRIES];
static uint32_t whyred_gpt_part_count;
static bool whyred_emmc_block_ready;
''' + '\n'.join(function(driver, name) for name in (
        'whyred_emmc_partition', 'whyred_emmc_open', 'whyred_emmc_ioctl')) + r'''
struct vnode { dev_t v_rdev; };
struct vnop_open_args { void *a_context; };
static int emulate_missing_setblocksize;
static int
test_ioctl(struct vnode *vp, u_long cmd, caddr_t data, int flags,
    __unused void *context)
{
    if (emulate_missing_setblocksize && cmd == DKIOCSETBLOCKSIZE)
        return ENOTTY;
    return whyred_emmc_ioctl(vp->v_rdev, cmd, data, flags, NULL);
}
#define VNOP_IOCTL test_ioctl
static int
root_geometry_open(dev_t dev, int flags)
{
    int error = whyred_emmc_open(dev, flags, 0, NULL);
    if (error) return error;
    struct vnode node = { dev };
    struct vnode *vp = &node;
    struct vnop_open_args arguments = { NULL };
    struct vnop_open_args *ap = &arguments;
    uint64_t blkcnt = 0;
    uint32_t blksize = 0, size512 = 512;
    int setsize = 0;
''' + geometry + r'''
    if (!error && (!setsize || blksize != 512 || blkcnt != 1024)) return EIO;
    return error;
}
#define CHECK(expr) do { checks++; if (!(expr)) { \
    fprintf(stderr, "failed at line %d: %s\n", __LINE__, #expr); return 1; \
} } while (0)
int main(void)
{
    unsigned int checks = 0;
    uint32_t value32;
    uint64_t value64;
    const dev_t disk = 61;
    whyred_gpt_parts[0].index = disk;
    whyred_gpt_parts[0].first = 128;
    whyred_gpt_parts[0].last = 1151;
    whyred_gpt_part_count = 1;
    whyred_emmc_block_ready = true;
    emulate_missing_setblocksize = 1;
    CHECK(root_geometry_open(disk, FREAD) == ENXIO);
    emulate_missing_setblocksize = 0;
    CHECK(root_geometry_open(disk, FREAD) == 0);
    CHECK(root_geometry_open(disk, FREAD | FWRITE) == EROFS);
    CHECK(whyred_emmc_open(0, FREAD, 0, NULL) == ENXIO);
    CHECK(whyred_emmc_open(65, FREAD, 0, NULL) == ENXIO);
    value32 = 0;
    CHECK(whyred_emmc_ioctl(disk, DKIOCGETBLOCKSIZE, (caddr_t)&value32, 0, NULL) == 0 && value32 == 512);
    value32 = 512;
    CHECK(whyred_emmc_ioctl(disk, DKIOCSETBLOCKSIZE, (caddr_t)&value32, FWRITE, NULL) == 0);
    value32 = 4096;
    CHECK(whyred_emmc_ioctl(disk, DKIOCSETBLOCKSIZE, (caddr_t)&value32, FWRITE, NULL) == EINVAL);
    value32 = 0;
    CHECK(whyred_emmc_ioctl(disk, DKIOCSETBLOCKSIZE, (caddr_t)&value32, FWRITE, NULL) == EINVAL);
    value32 = 1;
    CHECK(whyred_emmc_ioctl(disk, DKIOCISWRITABLE, (caddr_t)&value32, 0, NULL) == 0 && value32 == 0);
    value64 = 0;
    CHECK(whyred_emmc_ioctl(disk, DKIOCGETBLOCKCOUNT, (caddr_t)&value64, 0, NULL) == 0 && value64 == 1024);
    value64 = 1;
    CHECK(whyred_emmc_ioctl(disk, DKIOCGETMAXBLOCKCOUNTWRITE, (caddr_t)&value64, 0, NULL) == 0 && value64 == 0);
    CHECK(whyred_emmc_ioctl(disk, 9999, (caddr_t)&value64, FWRITE, NULL) == ENOTTY);
    whyred_emmc_block_ready = false;
    CHECK(whyred_emmc_open(disk, FREAD, 0, NULL) == ENXIO);
    printf("whyred eMMC contract: %u checks passed; historical spec_open failure reproduced; FWRITE denied\n", checks);
    return 0;
}
'''
    args.output.mkdir(parents=True, exist_ok=True)
    source = args.output / 'emmc-contract.c'
    executable = args.output / ('emmc-contract.exe' if os.name == 'nt' else 'emmc-contract')
    source.write_text(harness)
    host_env = os.environ.copy()
    host_env.pop('SDKROOT', None)
    compiler = [args.cc, '-std=c11', '-Wall', '-Wextra', '-Werror', '-O1']
    if sys.platform == 'darwin':
        sdk = subprocess.check_output(['xcrun', '--sdk', 'macosx', '--show-sdk-path'],
                                      text=True, env=host_env).strip()
        compiler += ['-isysroot', sdk]
    subprocess.run([*compiler, str(source), '-o', str(executable)],
                   check=True, env=host_env)
    result = subprocess.check_output([str(executable.resolve())], text=True, env=host_env)
    print(result, end='')
    report = {'result': result.strip(), 'source_sha256': {
        p.relative_to(args.source).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
        for p in (driver_path, spec_path)}, 'hardware_emulated': False}
    (args.output / 'report.json').write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
