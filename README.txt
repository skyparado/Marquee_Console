CSOPESY - Semi-Major Output 1
Marquee Console - Group 3, S02

GROUP DEVELOPERS
Garcia, Theodore
Magbatoc, Ethan
Parado, Sky
Villorente, Khyle

ENTRY POINT
The application main() is in src/main.cpp.
Repository: https://github.com/skyparado/Marquee_Console

REQUIREMENTS
Windows 10 or 11, Visual Studio or Visual Studio Build Tools with the
Desktop development with C++ workload (MSVC with C++17 support).
Use an interactive console; redirected input is unsupported.

BUILD AND RUN
From the project root in PowerShell or Command Prompt:
    scripts\build.bat
    build\marquee.exe

Alternatively, scripts\run.bat builds and opens the program in a console.
The build script automatically locates the installed C++ toolchain.
The included .vscode/tasks.json supports the VS Code build task.
Use a console at least 100 columns wide and 40 rows tall for help output.

COMMANDS
    help                 Show commands and descriptions.
    start_marquee        Start or resume animation.
    stop_marquee         Pause at the current position.
    set_text <text>      Change the banner and reset its position.
    set_speed <ms>       Change movement interval (1 to 2147483647 ms).
    exit                 Shut down the program.

Commands are case-sensitive. Text can contain spaces. Matching outer quotes
preserve leading/trailing spaces. Invalid commands display an error.

CONFIGURATION
Edit config.txt and restart; no recompilation is required.
    marquee_text=CSOPESY
    refresh_rate_ms=100
    polling_rate_ms=20
    start_running=false

Recommended baseline: 100 ms movement and 20 ms keyboard polling.
With start_running=false, enter start_marquee to begin moving.
Polling accepts 1 to 1000 ms. Refresh accepts 1 to 2147483647 ms.

SOURCE FILES
    src/main.cpp           Startup, configuration, input/command loop.
    src/commands.cpp       Command parsing and validation.
    src/config.cpp         Startup configuration parsing.
    src/display.cpp        Console header, banner, feedback, prompt.
    src/ascii_art.cpp      Glyphs and scrolling calculations.
    src/engine.cpp         Animation and display worker thread.
    src/input.cpp          Editable input buffer.
    src/input_windows.cpp  Non-blocking Windows keyboard polling.
    include/marquee/       Headers for the modules above.
    scripts/build.bat      Builds build/marquee.exe.
    scripts/setup_msvc.bat Locates and initializes MSVC for the build.
    scripts/run.bat        Builds and opens the interactive console.

DESIGN
The main thread polls input; the engine thread owns console output.
Shared state is protected by a mutex. Movement uses the configured refresh
interval. UI redraws occur approximately every 20 ms or when movement occurs,
so slow movement does not delay typing feedback. Actual timing depends on
rendering and OS scheduling. Changing speed restarts the movement timer.
