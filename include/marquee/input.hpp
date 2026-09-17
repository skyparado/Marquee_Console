#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace marquee {
struct InputBatch {
    std::vector<std::string> lines;
    bool limit_reached = false;
};

class InputBuffer {
public:
    static constexpr std::size_t max_length = 4096;
    void accept(int key, InputBatch& batch);
    const std::string& current_line() const { return line_; }
private:
    std::string line_;
    bool extended_key_ = false;
    bool previous_cr_ = false;
    bool overflow_ = false;
};

InputBatch poll_console(InputBuffer& input, std::size_t key_budget = 64);
}
