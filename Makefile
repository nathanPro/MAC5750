.PHONY: test clean

SHELL = /bin/bash

PYTHON_VENV = source .venv/bin/activate &&

test: build 
	$(PYTHON_VENV) ninja -j 4 -C build test
dev-build:
	$(PYTHON_VENV) meson --buildtype=debug --warnlevel=3 dev-build
build:
	$(PYTHON_VENV) meson --buildtype=release build
bcc: build
	$(PYTHON_VENV) ninja -j 4 -C build
clean:
	$(PYTHON_VENV) ninja -C dev-build clean
	$(PYTHON_VENV) ninja -C build clean
