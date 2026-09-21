#include "marquee/commands.hpp"
#include "marquee/engine.hpp"
#include "marquee/input.hpp"

#include <chrono>
#include <exception>
#include <iostream>
#include <mutex>
#include <thread>

#include <windows.h>

namespace {

void enable_ansi_escapes() {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (out != INVALID_HANDLE_VALUE && GetConsoleMode(out, &mode))
        SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

}

int main() {
    using namespace marquee;

    enable_ansi_escapes();

    SharedState shared;
    InputBuffer input;
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

                exiting = shared.value.exit_requested;
            }
            if (exiting)
                break;

            std::this_thread::sleep_for(std::chrono::milliseconds(20));
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
