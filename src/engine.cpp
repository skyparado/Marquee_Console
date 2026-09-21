#include "marquee/engine.hpp"
#include "marquee/display.hpp"
#include "marquee/ascii_art.hpp"

#include <chrono>
#include <thread>
#include <vector>

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
    constexpr int console_width = 80;

    while (!shutdown_) {
        State state = shared_state_.snapshot();

        if (state.exit_requested) break;

        //generate ascii art and correct its position
        if (state.status == Status::Running) {

            std::vector<std::string> ascii_art = generate_ascii_art(state.text);

            // width of text based on longest line in ASCII art to fit the console width
            int text_width = 0;
            int cycle_length = console_width + text_width;

            {
                // moves marquee position by 1 to do the wrap around effect
                std::lock_guard<std::mutex> lock(shared_state_.mutex);
                shared_state_.value.position = advance_position(state.position, cycle_length);
            }

            render();
            //for set_speed command
            std::this_thread::sleep_for(std::chrono::milliseconds(state.speed_ms));
        }

        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
    }


//renders marquee and other messages
void Engine::render() {
    State state = shared_state_.snapshot();

    reset_cursor_to_top();
    clear_entire_screen();

    render_header();

    render_ascii_marquee(state.text, state.position, 80);
    render_response_message(state.last_message);
    render_prompt(input_buffer_.current_line());
    }

}