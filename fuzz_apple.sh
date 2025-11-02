#!/usr/bin/env bash

/usr/local/opt/llvm/bin/clang -g -x objective-c++ -fsanitize=address,fuzzer fuzz.cpp -framework CoreImage -framework AppKit -L/usr/local/opt/llvm/lib/c++ -o fuzz
./fuzz -rss_limit_mb=0