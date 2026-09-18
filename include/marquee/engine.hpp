#pragma once

#include "marquee/commands.hpp"

#include <atomic>
#include <thread>

namespace marquee {

class Engine {

public: 
    Engine(SharedState& shared_state);
    ~Engine();

    void start();
    void stop();

private:
    void run();
    void render();

    SharedState& shared_state_;

    std::atomic<bool> shutdown_{false};
    std::thread worker_;
    };
}
