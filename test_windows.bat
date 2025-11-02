@echo off

cl /Zi test.cpp /link Ole32.lib WindowsCodecs.lib shlwapi.lib /OUT:"test.exe"
.\test.exe
del .\test.exe