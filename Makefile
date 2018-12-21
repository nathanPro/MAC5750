.PHONY: test clean
test: dev-build 
	ninja -j 4 -C dev-build test
dev-build:
	meson --buildtype=debug --warnlevel=3 dev-build
build:
	meson --buildtype=release build
bcc: build
	ninja -j 4 -C build
clean:
	rm -rf dev-build
	rm -rf build
