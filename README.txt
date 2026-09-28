================================================================================
CSOPESY - Semi-Major Output 1
Marquee Console (OS Emulator: Command Interpreter + Display)
================================================================================

GROUP DEVELOPERS
----------------
Garcia, Theodore
Magbatoc, Ethan
Parado, Sky
Villorente, Khyle

Repository: https://github.com/skyparado/Marquee_Console


================================================================================
ENTRY POINT
================================================================================

The main() function is located in:

    src/main.cpp

This is the entry file for the application. Note that tests/command_tests.cpp
also contains a main() function, but that is the test runner and is NOT the
application entry point. The two are never compiled together.


================================================================================
REQUIREMENTS
================================================================================

- Windows 10 or 11
- Visual Studio 2022 with the "Desktop development with C++" workload
  (this provides cl.exe, the MSVC compiler)
- C++17

The program uses the Windows Console API (_kbhit / _getch for non-blocking
input, and GetConsoleScreenBufferInfo for the window width), so it is
Windows-only and must be run in a real interactive console. It will not run
with redirected or piped input.


================================================================================
HOW TO BUILD AND RUN
================================================================================

OPTION 1 - Build script (simplest)
----------------------------------
From the repository root, in Command Prompt or PowerShell:

    scripts\build.bat
    build\marquee.exe

The script locates and loads vcvars64.bat on its own, so you do NOT need to
open a "Native Tools Command Prompt" first. It compiles all source files and
leaves only build\marquee.exe behind.


OPTION 2 - Visual Studio Code (Run / Debug button)
--------------------------------------------------
1. Open the repository folder in VS Code.
2. Press F5, or open the Run and Debug panel (Ctrl+Shift+D) and click the
   green play button next to "Run Marquee Console".

This compiles the project first, then launches it in an external console
window. Configuration lives in .vscode/launch.json and .vscode/tasks.json.

IMPORTANT: Do not use the small play button in the top-right corner of the
editor. That is "Run C/C++ File" and only compiles the single file you are
viewing, which will fail on this multi-file project.


OPTION 3 - Manual compile
-------------------------
From an "x64 Native Tools Command Prompt for VS 2022", in the repository root:

    if not exist build mkdir build
    cl /nologo /std:c++17 /EHsc /W4 /Iinclude src\main.cpp src\commands.cpp ^
       src\display.cpp src\ascii_art.cpp src\engine.cpp src\input.cpp ^
       src\input_windows.cpp src\config.cpp /Fo:build\ /Fe:build\marquee.exe
    build\marquee.exe


RECOMMENDED CONSOLE SETTINGS
----------------------------
Before running, set the console window to at least 100 columns wide and
30 rows tall (right-click the title bar -> Properties -> Layout). The display
renders a 21-line frame; a shorter window will cause the screen to scroll
instead of repainting in place.


================================================================================
COMMANDS
================================================================================

All commands are case-sensitive. Blank lines do nothing.

    help                 Show all commands and their descriptions.

    start_marquee        Start or resume the marquee animation. Resumes from
                         the current position rather than restarting.

    stop_marquee         Pause the marquee at its current position.

    set_text <text>      Set the marquee text. Spaces are allowed.
                         Resets the scroll position to 0.
                         Matching single or double quotes preserve leading and
                         trailing spaces, e.g.  set_text "  hello  "
                         Rejects empty text and control characters.

    set_speed <ms>       Set the refresh interval in milliseconds.
                         Accepts an integer from 1 to 2147483647.
                         Lower values scroll faster.

    exit                 Request shutdown. The animation thread is joined
                         cleanly before the program returns.

Invalid input always returns a message and leaves the program state unchanged;
the marquee keeps running through errors.

Editing keys supported at the prompt: printable ASCII, Enter, Backspace, and
Escape (clears the current line). Arrow and function keys are ignored. There
is no command history.


================================================================================
CONFIGURATION (config.txt)
================================================================================

On startup the program looks for config.txt in this order:

    1. the current working directory
    2. the folder containing marquee.exe
    3. the parent of that folder (the repository root when run from build\)

If no file is found, built-in defaults are used and the program still runs.
A malformed line is skipped with a warning rather than stopping the program.

Settings:

    marquee_text=CSOPESY        Initial text. Same validation as set_text.
    refresh_rate_ms=100         Initial refresh interval. Same as set_speed.
    polling_rate_ms=20          Milliseconds between keyboard polls (1-1000).
    start_running=false         true begins scrolling immediately.

Lines beginning with # are comments.


================================================================================
RUNNING THE TESTS
================================================================================

    scripts\test.bat

This builds and runs the component tests in tests/command_tests.cpp, covering
command parsing and validation, the input buffer, the scroll math, and config
parsing. On success it prints:

    All command, input-buffer, scroll, and config tests passed.

A failure prints the file and line number of the assertion that broke and
returns exit code 1.


================================================================================
SOURCE FILE OVERVIEW
================================================================================

    src/main.cpp            Entry point. Loads config, starts the engine, and
                            runs the input-poll / execute-command loop.
    src/commands.cpp        Command parsing, validation, execution, help text.
    src/config.cpp          config.txt parsing; routes values through the same
                            command validation used by the prompt.
    src/display.cpp         Header, marquee, message, and prompt rendering.
    src/ascii_art.cpp       ASCII glyph font and scroll math.
    src/engine.cpp          Animation thread; owns all console output.
    src/input.cpp           Editable input buffer (key handling).
    src/input_windows.cpp   Non-blocking Windows console polling.

    include/marquee/        Public headers for each module above.
    tests/command_tests.cpp Component test runner (separate main()).
    scripts/build.bat       Builds build\marquee.exe.
    scripts/test.bat        Builds and runs the tests.
    config.txt              Startup configuration.


================================================================================
DESIGN NOTES
================================================================================

The program runs two threads sharing one State struct guarded by a single
mutex:

  - The main thread polls the keyboard every polling_rate_ms (default 20 ms)
    and executes any completed command lines.

  - The engine thread advances the scroll position and redraws the screen
    every refresh_rate_ms (default 100 ms), and is the only thread that writes
    to the console.

All shared-state access holds the mutex, and SharedState::snapshot() returns a
locked copy so the lock is never held across a render or a sleep. The engine
sleeps in 20 ms slices so that exit responds promptly even at large refresh
intervals.

Because the prompt is only repainted on an engine frame, perceived typing
latency is up to one polling interval plus one refresh interval. This is why
very large refresh_rate_ms values make typing appear delayed even though input
is still being read on schedule.
