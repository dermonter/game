@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
cl -DUNICODE -D_UNICODE ..\src\test_open_gl.cpp /link opengl32.lib user32.lib gdi32.lib
popd