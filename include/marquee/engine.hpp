#pragma once

#include "marquee/commands.hpp"
#include "marquee/input.hpp" // Added to access InputBuffer

#include <atomic>
#include <thread>

namespace marquee {

class Engine {

public: 
    // Updated constructor to accept InputBuffer
    Engine(SharedState& shared_state, InputBuffer& input_buffer);
    ~Engine();

    void start();
    void stop();

private:
    void run();
    void render();

    SharedState& shared_state_;
    InputBuffer& input_buffer_; // Added reference

    std::atomic<bool> shutdown_{false};
    std::thread worker_;
};

}