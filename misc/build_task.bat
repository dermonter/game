@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
mkdir %1\build
cd %1\build
cl -Zi %1\src\win32_game.cpp user32.lib gdi32.lib