#!/bin/sh

# autogen.sh -- Autotools bootstrapping
#

AUTO_TOOLS_VER=$(automake --version | grep ^automake | sed 's/^.* //g' | cut -d'.' -f1-2)

aclocal-${AUTO_TOOLS_VER} &&\
autoheader &&\
autoconf &&\
automake-${AUTO_TOOLS_VER} --add-missing --copy

