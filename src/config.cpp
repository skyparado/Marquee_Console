#include "marquee/config.hpp"

#include <charconv>
#include <system_error>

namespace marquee {

ConfigResult apply_config(std::istream& in, SharedState& shared) {
    ConfigResult result;
    std::string raw;
    int line_no = 0;

    auto warn = [&](const std::string& why) {
        result.warnings += "config.txt line " + std::to_string(line_no) + ": " + why + "\n";
    };

    while (std::getline(in, raw)) {
        ++line_no;
        std::string_view line = raw;
        // Notepad and PowerShell can save UTF-8 with a byte-order mark; without
        // this the first key would never match.
        if (line_no == 1 && line.substr(0, 3) == "\xEF\xBB\xBF") line.remove_prefix(3);
        line = trim(line);
        if (line.empty() || line.front() == '#') continue;

        const auto eq = line.find('=');
        if (eq == std::string_view::npos) { warn("expected key=value, skipped."); continue; }
        const std::string_view key = trim(line.substr(0, eq));
        const std::string_view value = trim(line.substr(eq + 1));

        if (key == "marquee_text" || key == "refresh_rate_ms") {
            // Route through the real commands so the file and the prompt accept
            // exactly the same values, with one set of validation rules.
            const std::string command = (key == "marquee_text" ? "set_text " : "set_speed ") + std::string(value);
            const CommandResult applied = execute_command(command, shared);
            if (!applied.ok) warn(applied.message + " Kept the default.");
        } else if (key == "polling_rate_ms") {
            // ponytail: capped at 1000 ms; past that typing is unusable, and a
            // huge value would leave no way to type exit.
            int ms = 0;
            const auto parsed = std::from_chars(value.data(), value.data() + value.size(), ms);
            if (parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size() || ms < 1 || ms > 1000)
                warn("polling_rate_ms must be an integer from 1 to 1000. Kept the default.");
            else
                result.polling_rate_ms = ms;
        } else if (key == "start_running") {
            if (value == "true") execute_command("start_marquee", shared);
            else if (value != "false") warn("start_running must be true or false. Kept the default.");
        } else {
            warn("unknown setting '" + std::string(key) + "', skipped.");
        }
    }
    return result;
}

}
