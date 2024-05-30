#! /usr/bin/env python

# Copyright (c) 2021, The Linux Foundation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#     * Neither the name of The Linux Foundation nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
# ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os
import os.path
import subprocess
import sys
import shutil

install_dir = '/ipa-kernel-tests'  # unlikely to ever change, so 'file constant'


def get_args():
    class Args:
        pass

    args = Args()

    try:
        args.cc_path = os.path.dirname(os.environ['CROSS_COMPILE'])
    except:
        args.cc_path = None

    try:
        args.arch = os.environ['ARCH']
    except:
        raise Exception("ARCH must be set")

    try:
        args.kdir = os.environ['KDIR']
    except:
        raise Exception("KDIR must be set")

    try:
        args.dest = os.environ['DESTDIR']
    except:
        raise Exception("DESTDIR must be set")

    return args


def do(cmd, wdir=None):
    cwd = None
    if wdir:
        cwd = os.getcwd()
        os.chdir(wdir)
    subprocess.check_call(cmd)
    if cwd:
        os.chdir(cwd)


def build(args):

    if args.cc_path:
        os.environ['PATH'] = args.cc_path + ':' + os.environ['PATH']

    args.uapi = args.kdir + '/usr/include'
    args.src = args.kdir + '/techpack/dataipa/kernel-tests'
    args.inc = args.kdir + '/techpack/dataipa/drivers/platform/msm/ipa/ipa_test_module'

    full_uapi = os.path.abspath(args.uapi)
    os.environ['CPPFLAGS'] = ('-I' + full_uapi)
    full_inc = os.path.abspath(args.inc)
    os.environ['CPPFLAGS'] += (' -I' + full_inc)

    configure(args, args.src)

    do(['make'], args.src)
    do(['make', 'DESTDIR=' + args.dest, 'install'], args.src)


def configure(args, wdir):

    if os.path.isfile(os.path.join(wdir, 'config.h')):
        return

    do(['libtoolize'], wdir)
    do(['./autogen.sh'], wdir)

    full_idir = os.path.abspath(os.path.join(wdir, install_dir))
    host_str = 'arm-linux-gnueabihf'
    config_extra = ''
    if args.arch == 'arm64':
        host_str = 'aarch64-linux-gnu'
        config_extra = '--disable-swp'
    do(['./configure',
        '--host=' + host_str,
        '--prefix=' + full_idir,
        config_extra], wdir)


def main():
    rc = 0
    try:
        args = get_args()
        build(args)
    except Exception as e:
        rc = 1
        print(e)
        sys.exit(rc)

if __name__ == '__main__':
    main()
