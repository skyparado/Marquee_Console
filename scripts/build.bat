@echo off
REM Builds build\marquee.exe using the installed MSVC toolchain.
REM Usage: scripts\build.bat        (from the repository root, or anywhere)

setlocal
set "ROOT=%~dp0.."
call "%~dp0setup_msvc.bat"
if errorlevel 1 exit /b 1

cd /d "%ROOT%"
if not exist build mkdir build
if not exist build\obj mkdir build\obj

cl /nologo /std:c++17 /EHsc /W4 /O2 /Iinclude ^
   src\main.cpp src\commands.cpp src\display.cpp src\ascii_art.cpp ^
   src\engine.cpp src\input.cpp src\input_windows.cpp src\config.cpp ^
   /Fo:build\obj\ /Fe:build\marquee.exe
set "STATUS=%ERRORLEVEL%"

REM Keep intermediate files in build\obj for inspection.

if not "%STATUS%"=="0" (
    echo.
    echo BUILD FAILED
    exit /b %STATUS%
)

echo.
echo Built build\marquee.exe
exit /b 0
