#!/bin/bash

python -m venv .venv
source .venv/bin/activate
pip install ninja
pip install meson
deactivate
