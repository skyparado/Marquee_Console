#include "marquee/engine.hpp"
#include "marquee/display.hpp"
#include "marquee/ascii_art.hpp"

#include <chrono>

namespace marquee {

constexpr int MARQUEE_WIDTH = 120; // set your desired scrolling width here (aka how wide is the screen for the marquee to scroll across)

Engine::Engine(SharedState& shared_state, const InputBuffer& input_buffer) 
    : shared_state_(shared_state), input_buffer_(input_buffer) {}

Engine::~Engine() {
    stop();
}

void Engine::start() {
    shutdown_ = false;
    worker_ = std::thread(&Engine::run, this);
}

void Engine::stop() {
    shutdown_ = true;
    if (worker_.joinable()) {
        worker_.join();
    }
}

void Engine::run() {
    clear_entire_screen();

    while (!shutdown_) {
        State current_state = shared_state_.snapshot();

        if (current_state.exit_requested) {
            break;
        }

        render();

        // this is where the position of the text is updated for the scrolling bit, keeps things static if u hit stop
        if (current_state.status == Status::Running) {
            std::lock_guard<std::mutex> lock(shared_state_.mutex);

            // compute total cycle length: Screen Width + Length of ASCII Art Row
            int text_len = 0;
            if (!shared_state_.value.text.empty()) {
                text_len = static_cast<int>(generate_ascii_art(shared_state_.value.text)[0].length());
            }
            int total_cycle = MARQUEE_WIDTH + text_len;

            shared_state_.value.position = advance_position(shared_state_.value.position, total_cycle);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(current_state.speed_ms));
    }
}

void Engine::render() {
    State state = shared_state_.snapshot();
    // u guys can edit the order here, i just think its nicer if header comes after marquee !
    reset_cursor_to_top();
    render_ascii_marquee(state.text, state.position, MARQUEE_WIDTH);
    render_header();
    render_response_message(state.last_message);
    render_prompt(input_buffer_.current_line());
}

} 