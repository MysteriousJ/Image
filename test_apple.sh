#!/usr/bin/env bash

clang -g -x objective-c++ -arch arm64 test.cpp -framework CoreImage -framework AppKit -o test
./test
rm ./test
