@echo off

set flags=-MT -nologo -Z7 -FC
set linkerFlags=user32.lib gdi32.lib opengl32.lib glew32s.lib

IF NOT EXIST build mkdir build
pushd build
cl -DGAME_SLOW=1 -DGAME_INTERNAL=1 %flags% /I../vendor/include ../src/win32_platform.cpp /link /LIBPATH:../vendor/lib %linkerFlags%
popd