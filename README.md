# Marquee_Console

## Files

- `include/marquee/commands.hpp`, `src/commands.cpp`: shared state, validation, command execution, help.
- `include/marquee/input.hpp`, `src/input.cpp`: editable input buffer.
- `src/input_windows.cpp`: non-blocking Windows console polling with `_kbhit` / `_getch`.
- `include/marquee/display.hpp`, `src/display.cpp`: header, marquee, message, and prompt rendering.
- `include/marquee/ascii_art.hpp`, `src/ascii_art.cpp`: ASCII glyph font and scroll math.
- `include/marquee/engine.hpp`, `src/engine.cpp`: animation thread; owns all console output.
- `include/marquee/config.hpp`, `src/config.cpp`: reads `config.txt` settings at startup.
- `src/main.cpp`: application entry point (`main()` lives here); loads config, polls input, runs commands, owns shutdown.
- `tests/command_tests.cpp`: component test entry point; this is not the application's main.

## Commands

| Command | Behavior |
| --- | --- |
| `help` | Return command descriptions. |
| `start_marquee` | Set RUNNING; retain position. Repeated starts are harmless. |
| `stop_marquee` | Set STOPPED; retain position. Repeated stops are harmless. |
| `set_text Hello everyone` | Replace text, reset position to zero; retain running/stopped state. |
| `set_text "  Hello everyone  "` | Matching single or double outer quotes preserve edge spaces. No escape syntax. |
| `set_speed 100` | Set refresh milliseconds (1 through 2147483647); retain other state. |
| `exit` | Set shutdown request and STOPPED. The application owns joining threads and exiting. |

Commands are case-sensitive. Blank lines do nothing. Errors return a message and
leave state unchanged. Empty/whitespace-only text and control characters are rejected.
Default state: STOPPED, `CSOPESY`, 100 ms, position 0, text display mode.
The display owner may set `display_mode` to `AsciiGraphics`; there is no additional mode command in the assignment.

## Configuration (`config.txt`)

Settings live in `config.txt` in the repository root. Edit, save, and restart the
program; no recompiling. Format is `key=value`; `#` starts a comment.

| Key | Meaning | Accepted values | Default |
| --- | --- | --- | --- |
| `marquee_text` | Text shown at startup (same rules as `set_text`) | non-empty text; quotes keep edge spaces | `CSOPESY` |
| `refresh_rate_ms` | Milliseconds between marquee steps (same as `set_speed`) | 1 to 2147483647 | `100` |
| `polling_rate_ms` | Milliseconds between keyboard checks | 1 to 1000 | `20` |
| `start_running` | Start scrolling immediately | `true` / `false` | `false` |

The program looks in the working directory, then beside the exe, then one folder
above the exe, so it finds the root `config.txt` whether it is launched from the
repository root, from `build\`, or by an IDE. The first frame shows which file was
loaded. A bad line is skipped with an on-screen warning and the default is kept; a
missing file means built-in defaults.

## Integration

Create one `marquee::SharedState` and one `marquee::InputBuffer`. On each input
poll, call `poll_console(input)` and pass each returned `lines` entry to
`execute_command(line, shared)`. Hand the returned messages and a copy of
`input.current_line()` to the display layer, which owns the `Command>` prompt,
echo, redraw, and output serialization. Report `limit_reached` as an input-length
error.

Each poll reads up to 64 available key bytes. Poll independently of the animation
refresh interval. Do not use `getline` alongside this input.
Input supports printable ASCII, Enter, Backspace, and Escape (clear line).
Arrow/function keys are ignored. Unicode editing and command history are not provided.
Lines longer than 4096 characters are discarded on Enter; Escape clears the error.
Redirected stdin is unsupported and raises `std::runtime_error`; the application
should catch and display the error.

The input buffer belongs to one thread; copy its line before passing it to another
thread using the integration layer's synchronization. All direct shared-state
access must hold `shared.mutex`. `shared.snapshot()` returns a locked copy.
The scheduler must check status/shutdown and update position under that same lock.
Release the lock before sleeping or rendering. Stop consuming commands after exit.

## Build and verify

From an x64 Native Tools Command Prompt for Visual Studio, in the repository root:

```bat
if not exist build mkdir build
cl /nologo /std:c++17 /EHsc /W4 /Iinclude src\main.cpp src\commands.cpp src\display.cpp src\ascii_art.cpp src\engine.cpp src\input.cpp src\input_windows.cpp src\config.cpp /Fo:build\ /Fe:build\marquee.exe
build\marquee.exe
```

Tests:

```bat
cl /nologo /std:c++17 /EHsc /W4 /Iinclude src\commands.cpp src\ascii_art.cpp src\input.cpp src\input_windows.cpp src\config.cpp tests\command_tests.cpp /Fo:build\ /Fe:build\command_tests.exe
build\command_tests.exe
```
