#!/usr/bin/env bash

clang test.cpp -lturbojpeg -lpng -o test
./test
rm ./test