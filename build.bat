@echo off

mkdir build
pushd build
cl -FC -Zi ..\src\win32_handmade.cpp user32.lib gdi32.lib
popd
