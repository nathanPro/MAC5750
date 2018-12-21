.PHONY: test
test: build
	ninja -j 4 -C build test
bcc:
	ninja -j 4 -C build
build:
	meson build

