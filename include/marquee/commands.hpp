#pragma once

#include <mutex>
#include <string>
#include <string_view>

namespace marquee {

enum class Status { Stopped, Running };
enum class DisplayMode { Text, AsciiGraphics };

struct State {
    Status status = Status::Stopped;
    std::string text = "CSOPESY"; // default text, overridden by config.txt or set_text
    int speed_ms = 100;
    int position = 0;
    DisplayMode display_mode = DisplayMode::Text;
    bool exit_requested = false;
    std::string last_message = ""; // the last command's response, shown above the prompt
    // Copy of the line being typed. The InputBuffer belongs to the input thread,
    // so the engine must not read it directly; the input thread republishes it
    // here under the mutex and the engine reads it from its snapshot.
    std::string input_line = "";
};

struct SharedState {
    mutable std::mutex mutex;
    State value;
    State snapshot() const;
};

struct CommandResult {
    bool ok;
    std::string message;
};

std::string_view trim(std::string_view text);
std::string_view help_text();
CommandResult execute_command(std::string_view line, SharedState& shared);

}
