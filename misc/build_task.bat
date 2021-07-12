@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
IF NOT EXIST %1\build mkdir %1\build
cd %1\build
cl -MT -nologo -Gm- -FC -GR- -EHa- -Oi -W4 -WX -wd4100 -wd4189 -wd4456 -DSLOW=1 -DWIN32_OPEN_GL=0 -Zi -Fmwin32game.map %1\src\win32_game.cpp /link -opt:ref user32.lib gdi32.lib winmm.lib opengl32.lib