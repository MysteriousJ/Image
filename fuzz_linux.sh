#!/usr/bin/env bash

clang -g -fsanitize=fuzzer fuzz.cpp -lturbojpeg -lpng -o fuzz
./fuzz