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

# This script creates symlinks for tests that are compiled as Soong modules, so
# that they may be accessed from their old location in /data/kernel-tests

#!/bin/sh

NTEST_PATH="/data/nativetest"
KTEST_PATH="/data/kernel-tests"

# Any test that is compiled as a cc_test module (which applies to any Soong
# module in kernel-tests-internal) is given a directory, in to which the
# compiled test and its supporting files are placed.  This function iterates
# over the test directories in $1 and creates symlinks to all the files
# contained within a given test directory. $2 and later arguemnts can be used to
# specify directores to skip.
create_links()
{
    BASE_PATH="$1"
    shift
    SKIP_FILES=$@ # These can also be directories

    for TEST_DIR in `ls "$BASE_PATH"`; do
        # Filter out any file / directory in SKIP_FILES.
        SHOULD_SKIP=""
        for i in $SKIP_FILES; do
            if [ "$TEST_DIR" = "$i" ]; then
                SHOULD_SKIP="TRUE"
                break
            fi
        done
        if [ ! "$SHOULD_SKIP" = "" ]; then
            continue
        fi


        # Filter out any files that aren't directories (since every test we
        # compile is given a directory)
        if [ ! -d "$BASE_PATH/$TEST_DIR" ]; then
            continue
        fi

        # Now, create the symlinks for each test file inside of TEST_DIR
        for TEST_FILE in `ls "$BASE_PATH/$TEST_DIR"`; do
            ln -fs "$BASE_PATH/$TEST_DIR/$TEST_FILE" "$KTEST_PATH/$TEST_FILE"
        done
    done
}

# Create /data/kernel-tests, if it doesn't exist already
mkdir -p "$KTEST_PATH"

# Create the symlinks
create_links "$NTEST_PATH" "vendor"
create_links "$NTEST_PATH/vendor"
