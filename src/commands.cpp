#include "marquee/commands.hpp"

#include <charconv>
#include <cctype>
#include <system_error>

namespace marquee {
std::string_view trim(std::string_view text) {
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front())))
        text.remove_prefix(1);
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())))
        text.remove_suffix(1);
    return text;
}

State SharedState::snapshot() const {
    std::lock_guard<std::mutex> lock(mutex);
    return value;
}

std::string_view help_text() {
    return "Commands (case-sensitive):\n"
           "  help                 Show commands and descriptions.\n"
           "  start_marquee        Start or resume the marquee.\n"
           "  stop_marquee         Pause the marquee at its current position.\n"
           "  set_text <text>      Set text; spaces are allowed. Resets position to 0.\n"
           "                       Optional matching single/double quotes preserve edge spaces.\n"
           "  set_speed <ms>       Set refresh interval: integer 1..2147483647 milliseconds.\n"
           "  exit                 Request console shutdown and stop the marquee.\n";
}

CommandResult execute_command(std::string_view line, SharedState& shared) {
    line = trim(line);
    if (line.empty()) return {true, ""};
    const auto split = line.find_first_of(" \t\r\n\v\f");
    const auto command = line.substr(0, split);
    auto argument = split == std::string_view::npos ? std::string_view{} : trim(line.substr(split));

    if (command == "set_text") {
        if (argument.empty()) return {false, "Usage: set_text <non-empty text>"};
        if (argument.front() == '\"' || argument.front() == '\'') {
            if (argument.size() < 2 || argument.back() != argument.front())
                return {false, "Text must end with its matching opening quote."};
            argument = argument.substr(1, argument.size() - 2);
        }
        if (trim(argument).empty()) return {false, "Text must contain a non-whitespace character."};
        for (unsigned char ch : argument) {
            if (ch < 32 || ch == 127) return {false, "Text cannot contain control characters."};
        }
        std::lock_guard<std::mutex> lock(shared.mutex);
        shared.value.text = std::string(argument);
        shared.value.position = 0;
        return {true, "Marquee text updated."};
    }
    if (command == "set_speed") {
        int speed = 0;
        if (argument.empty()) return {false, "Usage: set_speed <positive integer milliseconds>"};
        const auto parsed = std::from_chars(argument.data(), argument.data() + argument.size(), speed);
        if (parsed.ec != std::errc{} || parsed.ptr != argument.data() + argument.size() || speed <= 0)
            return {false, "Speed must be an integer from 1 to 2147483647 milliseconds."};
        std::lock_guard<std::mutex> lock(shared.mutex);
        shared.value.speed_ms = speed;
        return {true, "Marquee speed updated."};
    }
    if (command != "help" && command != "start_marquee" && command != "stop_marquee" && command != "exit")
        return {false, "Unknown command. Type help for available commands."};
    if (!argument.empty()) return {false, std::string(command) + " does not accept arguments."};
    if (command == "help") return {true, std::string(help_text())};

    std::lock_guard<std::mutex> lock(shared.mutex);
    if (command == "exit") {
        shared.value.exit_requested = true;
        shared.value.status = Status::Stopped;
        return {true, "Exit requested."};
    }
    if (shared.value.exit_requested) return {false, "Console shutdown has already been requested."};
    shared.value.status = command == "start_marquee" ? Status::Running : Status::Stopped;
    return {true, command == "start_marquee" ? "Marquee running." : "Marquee stopped."};
}
}
