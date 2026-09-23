#pragma once

#include <mutex>
#include <string>
#include <string_view>

namespace marquee {

enum class Status { Stopped, Running };
enum class DisplayMode { Text, AsciiGraphics };

struct State {
    Status status = Status::Stopped;
    std::string text = "CSOPESY"; // change default text here if u want
    int speed_ms = 100;
    int position = 0;
    DisplayMode display_mode = DisplayMode::Text;
    bool exit_requested = false;
    std::string last_message = ""; // store response messages so i can display them (this is for the command responses)
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

std::string_view help_text();
CommandResult execute_command(std::string_view line, SharedState& shared);

}
