#!/bin/bash
set -e

SRC="media/car.png"
TMP="/tmp/icon.iconset/"
OUT="media/app.icns"

rm -fr "${TMP}/"
mkdir -p "${TMP}/"

if ! [ -f "${SRC}" ] ; then
    echo "must be runned from top level, runned from ${PWD}"
    exit 1
fi

OS="$(uname)"

require_cmd() {
    command -v "$1" >/dev/null 2>&1 || {
        echo "missing: $1"
        if [[ "$OS" == "Linux" ]]; then
            if command -v apt >/dev/null 2>&1; then
                sudo apt update
                sudo apt install -y imagemagick icnsutils
            else
                echo "install imagemagick and icnsutils manually"
                exit 1
            fi
        else
            echo "install $1"
            exit 1
        fi
    }
}

if [[ "$OS" == "Darwin" ]]; then
    require_cmd sips
    require_cmd iconutil

    sips -z 16 16     "$SRC" --out ${TMP}/icon_16x16.png
    sips -z 32 32     "$SRC" --out ${TMP}/icon_16x16@2x.png
    sips -z 32 32     "$SRC" --out ${TMP}/icon_32x32.png
    sips -z 64 64     "$SRC" --out ${TMP}/icon_32x32@2x.png
    sips -z 128 128   "$SRC" --out ${TMP}/icon_128x128.png
    sips -z 256 256   "$SRC" --out ${TMP}/icon_128x128@2x.png
    sips -z 256 256   "$SRC" --out ${TMP}/icon_256x256.png
    sips -z 512 512   "$SRC" --out ${TMP}/icon_256x256@2x.png
    sips -z 512 512   "$SRC" --out ${TMP}/icon_512x512.png
    sips -z 1024 1024 "$SRC" --out ${TMP}/icon_512x512@2x.png

    iconutil -c icns ${TMP}/ -o "$OUT"

elif [[ "$OS" == "Linux" ]]; then
    require_cmd convert
    require_cmd png2icns

    convert "$SRC" -resize 16x16     ${TMP}/icon_16.png
    convert "$SRC" -resize 32x32     ${TMP}/icon_32.png
    convert "$SRC" -resize 128x128   ${TMP}/icon_128.png
    convert "$SRC" -resize 256x256   ${TMP}/icon_256.png
    convert "$SRC" -resize 512x512   ${TMP}/icon_512.png
    convert "$SRC" -resize 1024x1024 ${TMP}/icon_1024.png

    png2icns "$OUT" ${TMP}/icon_16.png ${TMP}/icon_32.png ${TMP}/icon_128.png ${TMP}/icon_256.png ${TMP}/icon_512.png ${TMP}/icon_1024.png

else
    echo "unsupported OS: $OS"
    exit 1
fi