#!/usr/bin/env bash

clang -g test.cpp -lturbojpeg -lpng -o test
./test
rm ./test