#!/usr/bin/env bash

clang -g -fsanitize=fuzzer fuzz.cpp -lturbojpeg -lpng -lwebp -o fuzz
./fuzz