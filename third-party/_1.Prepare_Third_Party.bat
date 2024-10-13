@echo off
REM clone vcpkg repository
git clone https://github.com/microsoft/vcpkg.git

REM run bootstrap-vcpkg.bat
cd vcpkg
call bootstrap-vcpkg.bat

REM install assimp
vcpkg install assimp:x64-windows

REM install directxtk
vcpkg install directxtk:x64-windows

REM install fp16
vcpkg install fp16:x64-windows

REM install directxtex[openexr]
vcpkg install directxtex[openexr]:x64-windows

REM install directxmesh
vcpkg install directxmesh:x64-windows

REM integrate to visual studio
vcpkg integrate install

echo complete!
pause
