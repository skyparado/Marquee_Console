#pragma once

#include "marquee/commands.hpp"
#include "marquee/input.hpp"

#include <atomic>
#include <thread>

namespace marquee {

class Engine {

public: 
    Engine(SharedState& shared_state, InputBuffer& input_buffer);
    ~Engine();

    void start();
    void stop();

private:
    void run();
    void render();

    SharedState& shared_state_;
    InputBuffer& input_buffer_;

    std::atomic<bool> shutdown_{false};
    std::thread worker_;
};

}