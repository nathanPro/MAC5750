#!/usr/bin/fish

function run_file
    eval ./$argv
end

cd "$MESON_BUILD_ROOT"
./test_parser
./test_IR
./test_catamorphism
./test_translation
./test_helper
