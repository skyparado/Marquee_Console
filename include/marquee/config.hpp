#pragma once

#include "marquee/commands.hpp"

#include <istream>
#include <string>

namespace marquee {

struct ConfigResult {
    int polling_rate_ms = 20;
    std::string warnings; // one line per skipped setting; empty when the file was clean
};

// Reads key=value lines and applies them to shared state. A bad line is skipped
// with a warning instead of stopping the program, so a typo in config.txt
// cannot kill a live demo.
ConfigResult apply_config(std::istream& in, SharedState& shared);

}
