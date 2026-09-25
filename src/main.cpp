#include "marquee/commands.hpp"
#include "marquee/config.hpp"
#include "marquee/engine.hpp"
#include "marquee/input.hpp"

#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include <windows.h>

namespace {

void enable_ansi_escapes() {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (out != INVALID_HANDLE_VALUE && GetConsoleMode(out, &mode))
        SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

// The working directory depends on how the program is launched (terminal, IDE
// Run button, double-click), so look in the working directory first, then
// beside the exe, then one folder up (the repo root when the exe is in build\).
std::ifstream open_config(std::string& found_path) {
    std::vector<std::filesystem::path> candidates{"config.txt"};
    wchar_t exe[MAX_PATH];
    const DWORD length = GetModuleFileNameW(nullptr, exe, MAX_PATH);
    if (length > 0 && length < MAX_PATH) {
        const auto exe_dir = std::filesystem::path(exe).parent_path();
        candidates.push_back(exe_dir / "config.txt");
        candidates.push_back(exe_dir.parent_path() / "config.txt");
    }
    for (const auto& candidate : candidates) {
        std::ifstream file(candidate);
        if (file) {
            found_path = std::filesystem::absolute(candidate).string();
            return file;
        }
    }
    return {};
}

}

int main() {
    using namespace marquee;

    enable_ansi_escapes();

    SharedState shared;
    InputBuffer input;

    // Applied before the engine starts so the very first frame already shows
    // the configured text, speed, and running state.
    std::string config_path;
    std::ifstream config_file = open_config(config_path);
    // is_open(), not the stream's truthiness: reading to end-of-file sets the
    // fail flag, which would report a loaded file as missing.
    const bool config_found = config_file.is_open();
    ConfigResult config;
    if (config_found)
        config = apply_config(config_file, shared);
    {
        std::lock_guard<std::mutex> lock(shared.mutex);
        shared.value.last_message = config_found
            ? "Loaded " + config_path + "\n" + config.warnings
            : "config.txt not found; using built-in defaults.";
    }

    Engine engine(shared, input);
    engine.start();

    try {
        while (true) {
            InputBatch batch = poll_console(input);

            for (const std::string& line : batch.lines) {
                const CommandResult result = execute_command(line, shared);
                std::lock_guard<std::mutex> lock(shared.mutex);
                shared.value.last_message = result.message;
            }

            bool exiting = false;
            {
                std::lock_guard<std::mutex> lock(shared.mutex);
                if (batch.limit_reached)
                    shared.value.last_message = "Input too long; press Escape to clear the line.";

                // Publish the in-progress line for the engine to render. The engine
                // must never touch the InputBuffer itself; it belongs to this thread.
                shared.value.input_line = input.current_line();

                exiting = shared.value.exit_requested;
            }
            if (exiting)
                break;

            std::this_thread::sleep_for(std::chrono::milliseconds(config.polling_rate_ms));
        }
    } catch (const std::exception& ex) {
        engine.stop();
        std::cout << "\033[2J\033[H" << ex.what() << "\n";
        return 1;
    }

    engine.stop();
    std::cout << "\033[2J\033[H" << "Console shutting down.\n";
    return 0;
}
