#!/usr/bin/env bash
set -e
clang -g -fsanitize=address,fuzzer fuzz.cpp $(pkg-config --cflags --libs gdk-3.0) -lturbojpeg -lpng -o fuzz
./fuzz