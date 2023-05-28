#! /bin/sh

# Copyright (C) 2017 André Klitzing
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#
# This is a small script to copy the required files from a zlib tarball
# into 3rdparty/zlib/

if [ $# -ne 2 ]; then
    echo "Usage: $0 zlib_tarball_dir/ \$QTDIR/src/3rdparty/zlib/"
    exit 1
fi

ZLIB_DIR=$1
TARGET_DIR=$2

if [ ! -d "$ZLIB_DIR" -o ! -r "$ZLIB_DIR" -o ! -d "$TARGET_DIR" -o ! -w "$TARGET_DIR" ]; then
    echo "Either the zlib source dir or the target dir do not exist,"
    echo "are not directories or have the wrong permissions."
    exit 2
fi

# with 1 argument, copies ZLIB_DIR/$1 to TARGET_DIR/$1
# with 2 arguments, copies ZLIB_DIR/$1 to TARGET_DIR/$2
copy_file() {
    if [ $# -lt 1 -o $# -gt 2  ]; then
        echo "Wrong number of arguments to copy_file"
        exit 3
    fi

    SOURCE_FILE=$1
    if [ -n "$2" ]; then
        DEST_FILE=$2
    else
        DEST_FILE=$1
    fi

    mkdir -p "$TARGET_DIR/$(dirname "$SOURCE_FILE")"
    cp "$ZLIB_DIR/$SOURCE_FILE" "$TARGET_DIR/$DEST_FILE"
}

FILES="
   README
   ChangeLog

   adler32.c
   compress.c
   crc32.c
   crc32.h
   deflate.c
   deflate.h
   gzclose.c
   gzguts.h
   gzlib.c
   gzread.c
   gzwrite.c
   infback.c
   inffast.c
   inffast.h
   inffixed.h
   inflate.c
   inflate.h
   inftrees.c
   inftrees.h
   trees.c
   trees.h
   uncompr.c
   zconf.h
   zlib.h
   zutil.c
   zutil.h

"

for i in $FILES; do
    copy_file "$i" "src/$i"
done
