#!/bin/bash

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd)"

OUT_DIR="${REPO_ROOT}/data/data"

if ! [ -d "$OUT_DIR" ] ; then
    echo "output dir ${OUT_DIR} not found"
    exit 1
fi

wget -P "${OUT_DIR}/" https://github.com/autodiag2/database/releases/latest/download/ad_database.sqlite