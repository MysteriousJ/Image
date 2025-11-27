#!/usr/bin/env bash

clang -g test.cpp -lturbojpeg -lpng -lwebp -o test
./test
rm ./test