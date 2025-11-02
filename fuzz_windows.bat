@echo off

cl /Zi /fsanitize=fuzzer fuzz.cpp /link Ole32.lib WindowsCodecs.lib shlwapi.lib /OUT:"fuzz.exe"
.\fuzz.exe
