@echo off
REM Keep environment changes in the calling build/test script's setlocal scope.
where cl >nul 2>nul
if not errorlevel 1 exit /b 0
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo ERROR: Install Visual Studio Build Tools with Desktop development with C++.
    exit /b 1
)
set "VSINSTALL="
for /f "usebackq tokens=*" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSINSTALL=%%I"
if not defined VSINSTALL (
    echo ERROR: No Visual Studio installation with C++ tools was found.
    exit /b 1
)
REM Some vcvars installations invoke vswhere by name.
set "PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer;%PATH%"
call "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
where cl >nul 2>nul
exit /b %ERRORLEVEL%
