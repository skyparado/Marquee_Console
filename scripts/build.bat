@echo off
REM Builds build\marquee.exe and leaves nothing else behind.
REM Usage: scripts\build.bat        (from the repository root, or anywhere)

setlocal
set "ROOT=%~dp0.."
set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

if not exist "%VCVARS%" (
    echo ERROR: Could not find vcvars64.bat at:
    echo   %VCVARS%
    echo Install Visual Studio 2022 with the "Desktop development with C++" workload.
    exit /b 1
)

REM "call" matters: without it the batch file transfers control and never returns.
call "%VCVARS%" >nul

cd /d "%ROOT%"
if not exist build mkdir build
if not exist build\obj mkdir build\obj

cl /nologo /std:c++17 /EHsc /W4 /O2 /Iinclude ^
   src\main.cpp src\commands.cpp src\display.cpp src\ascii_art.cpp ^
   src\engine.cpp src\input.cpp src\input_windows.cpp src\config.cpp ^
   /Fo:build\obj\ /Fe:build\marquee.exe
set "STATUS=%ERRORLEVEL%"

REM Intermediates are not needed once the exe is linked.
rd /s /q build\obj 2>nul

if not "%STATUS%"=="0" (
    echo.
    echo BUILD FAILED
    exit /b %STATUS%
)

echo.
echo Built build\marquee.exe
exit /b 0
