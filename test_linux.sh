#!/usr/bin/env bash
set -e

clang -g test.cpp $(pkg-config --cflags --libs gdk-3.0) -lturbojpeg -lpng -o test
./test
rm ./test
