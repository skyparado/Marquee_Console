@echo off
REM Builds, then launches marquee.exe in its own console window.
REM The program needs a real interactive console (_kbhit/_getch and
REM GetConsoleScreenBufferInfo), so it must not run inside an editor's
REM output pane.

call "%~dp0build.bat"
if not "%ERRORLEVEL%"=="0" exit /b %ERRORLEVEL%

start "Marquee Console" "%~dp0..\build\marquee.exe"
