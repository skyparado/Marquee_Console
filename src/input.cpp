#include "marquee/input.hpp"

namespace marquee {
void InputBuffer::accept(int key, InputBatch& batch) {
    if (extended_key_) { extended_key_ = false; return; }
    if (key == 0 || key == 224) { extended_key_ = true; return; }
    if (key == '\n' && previous_cr_) { previous_cr_ = false; return; }
    previous_cr_ = key == '\r';
    if (key == '\r' || key == '\n') {
        if (overflow_) batch.limit_reached = true;
        else batch.lines.push_back(line_);
        line_.clear();
        overflow_ = false;
    } else if (key == 27) {
        line_.clear();
        overflow_ = false;
    } else if (key == '\b' || key == 127) {
        if (!line_.empty()) line_.pop_back();
    } else if (key >= 32 && key <= 126) {
        if (line_.size() < max_length && !overflow_) line_.push_back(static_cast<char>(key));
        else { overflow_ = true; batch.limit_reached = true; }
    }
}
}
