#!/bin/bash

cd python-uds
pip install .
python doip.py
cd "$OLDPWD"

