#include "marquee/engine.hpp"
#include "marquee/display.hpp"
#include "marquee/ascii_art.hpp"

#include <chrono>
#include <thread>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

int get_console_width() {
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) {
        return w.ws_col;
    }
#endif
    return 80; // Default width if unable to determine
}

namespace marquee {

//constructor that accepts InputBuffer, removed const
Engine::Engine(SharedState& shared_state, InputBuffer& input_buffer)
             : shared_state_(shared_state), input_buffer_(input_buffer) {}


//destructor
Engine::~Engine() {
    stop();
    }


//starts a new thread
void Engine::start() {
    if (worker_.joinable())
        return;

        shutdown_ = false;
        worker_ = std::thread(&Engine::run, this);
    }


//stops the thread
void Engine::stop() {
    shutdown_ = true;

    if (worker_.joinable())
        worker_.join();
    }


//main animation logic
void Engine::run() {
    using Clock = std::chrono::steady_clock;
    auto next_step = Clock::now();
    auto next_redraw = next_step;
    int previous_speed = -1;
    Status previous_status = Status::Stopped;
    while (!shutdown_) {
        const int console_width = get_console_width();
        const auto now = Clock::now();
        bool moved = false;
        {
            // Check and advance the current state under one lock so a command's
            // stop or position reset cannot be overwritten by a stale snapshot.
            std::lock_guard<std::mutex> lock(shared_state_.mutex);
            auto& state = shared_state_.value;
            if (state.exit_requested) break;
            if (state.speed_ms != previous_speed || state.status != previous_status) {
                next_step = now + std::chrono::milliseconds(state.speed_ms);
                previous_speed = state.speed_ms;
                previous_status = state.status;
            }
            if (state.status == Status::Running && now >= next_step) {
                const auto banner = generate_ascii_art(state.text);
                const int cycle = console_width + static_cast<int>(banner[0].size());
                state.position = advance_position(state.position, cycle);
                next_step = now + std::chrono::milliseconds(state.speed_ms);
                moved = true;
            }
        }
        // Keep feedback responsive even when movement is slow or stopped.
        if (moved || now >= next_redraw) {
            render();
            next_redraw = Clock::now() + std::chrono::milliseconds(20);
        }
        auto wake = next_redraw;
        if (previous_status == Status::Running && next_step < wake)
            wake = next_step;
        std::this_thread::sleep_until(wake);
    }
}


//renders marquee and other messages
void Engine::render() {
    State state = shared_state_.snapshot();

    reset_cursor_to_top();
    clear_entire_screen();

    render_header();

    render_ascii_marquee(state.text, state.position, get_console_width());
    render_response_message(state.last_message);
    // Read the published copy, not input_buffer_: that buffer is mutated by the
    // input thread and reading it here would be an unsynchronized data race.
    render_prompt(state.input_line);
    }

}
