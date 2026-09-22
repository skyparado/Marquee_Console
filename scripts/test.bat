@echo off
REM Builds and runs the component tests; leaves only build\command_tests.exe behind.

setlocal
set "ROOT=%~dp0.."
set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

if not exist "%VCVARS%" (
    echo ERROR: Could not find vcvars64.bat at:
    echo   %VCVARS%
    exit /b 1
)

call "%VCVARS%" >nul

cd /d "%ROOT%"
if not exist build mkdir build
if not exist build\obj mkdir build\obj

cl /nologo /std:c++17 /EHsc /W4 /Iinclude ^
   src\commands.cpp src\ascii_art.cpp src\input.cpp src\input_windows.cpp ^
   tests\command_tests.cpp ^
   /Fo:build\obj\ /Fe:build\command_tests.exe
set "STATUS=%ERRORLEVEL%"

rd /s /q build\obj 2>nul

if not "%STATUS%"=="0" (
    echo.
    echo BUILD FAILED
    exit /b %STATUS%
)

build\command_tests.exe
exit /b %ERRORLEVEL%
