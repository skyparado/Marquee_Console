#include "marquee/input.hpp"

#include <conio.h>
#include <cstdio>
#include <io.h>
#include <stdexcept>

namespace marquee {
InputBatch poll_console(InputBuffer& input, std::size_t key_budget) {
    if (!_isatty(_fileno(stdin)))
        throw std::runtime_error("Polling input requires an interactive Windows console.");
    InputBatch batch;
    for (std::size_t count = 0; count < key_budget && _kbhit(); ++count)
        input.accept(_getch(), batch);
    return batch;
}
}
