@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
mkdir ..\build
pushd ..\build
cl -Zi ..\src\win32_game.cpp user32.lib gdi32.lib
popd