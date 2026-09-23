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
    while (!shutdown_) {
        State state = shared_state_.snapshot();

        if (state.exit_requested) break;

        // Must match the width the renderer actually draws with, or the banner
        // wraps back around before it has crossed the visible screen.
        const int console_width = get_console_width();

        //generate ascii art and correct its position
        if (state.status == Status::Running) {

            std::vector<std::string> ascii_art = generate_ascii_art(state.text);

            // width of the rendered banner, so the text scrolls all the way
            // off-screen before it wraps back around
            int text_width = static_cast<int>(ascii_art[0].size());
            int cycle_length = console_width + text_width;

            {
                // moves marquee position by 1 to do the wrap around effect
                std::lock_guard<std::mutex> lock(shared_state_.mutex);
                shared_state_.value.position = advance_position(state.position, cycle_length);
            }
        }

        // render every frame, not just while running, so typed characters echo
        // at the prompt while the marquee is stopped
        render();

        //for set_speed command
        // Sleep in short slices instead of one long block: a single sleep_for of
        // speed_ms would make stop() wait out the whole interval (up to ~24 days
        // at the maximum accepted speed) before the thread could be joined.
        const int total_ms = state.status == Status::Running ? state.speed_ms : 50;
        constexpr int slice_ms = 20;
        for (int slept = 0; slept < total_ms && !shutdown_; slept += slice_ms) {
            const int remaining = total_ms - slept;
            std::this_thread::sleep_for(
                std::chrono::milliseconds(remaining < slice_ms ? remaining : slice_ms));
        }
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