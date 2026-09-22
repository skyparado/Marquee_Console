#include "marquee/display.hpp"
#include "marquee/ascii_art.hpp"

#include <iostream>
#include <algorithm>
#include <sstream>
#include <vector>

namespace marquee {

// this function resets the cursor to the top of the console, so we can overwrite the previous frame
void reset_cursor_to_top() {
    std::cout << "\033[H";
}

// this cleas the entire console screen, so we can start fresh with a new frame
void clear_entire_screen() {
    std::cout << "\033[2J";
}

// print header obvi
void render_header() {
    std::cout << "Welcome to CSOPESY!\033[K\n\n";
    std::cout << "Group developers:\033[K\n";
    std::cout << "Garcia, Theodore\033[K\n";
    std::cout << "Magbatoc, Ethan\033[K\n";
    std::cout << "Parado, Sky\033[K\n";
    std::cout << "Villorente, Khyle\033[K\n\n";
    std::cout << "Version date: Sept 20, 2026\033[K\n\n";
}

// compute_scrolled_line pads every row out to exactly `width` characters. Printing
// all of them fills the last cell when the window is exactly that wide, which makes
// the console auto-wrap; the "\n" that follows then costs a second line, the frame
// outgrows the window, and the whole display scrolls instead of repainting in place.
// The trailing "\033[K" already erases to end of line, so the padding buys nothing.
// Also never occupy the final column: a glyph scrolling through column width-1
// would fill the last cell and wrap even after the padding is gone.
std::string fit_to_line(const std::string& line, int width) {
    const std::size_t end = line.find_last_not_of(' ');
    if (end == std::string::npos) return std::string();
    const std::size_t usable = width > 1 ? static_cast<std::size_t>(width - 1) : 1;
    return line.substr(0, std::min(end + 1, usable));
}

// okay now we combine all helper funcs from ascii_art here to do the marquee thing
void render_ascii_marquee(const std::string& text, int position, int width) {
    std::cout << "\033[K\n\n\n";
    // if the text input is not empty the ascii art is generated, otherwise just print empty lines (duh)
    if (!text.empty()) {
        std::vector<std::string> ascii_banner = generate_ascii_art(text);
        for (int row = 0; row < 5; ++row) {
            std::string line = compute_scrolled_line(ascii_banner[row], position, width);
            std::cout << fit_to_line(line, width) << "\033[K\n";
        }
    } else {
        for (int i = 0; i < 5; ++i) {
            std::cout << "\033[K\n";
        }
    }
    std::cout << "\033[K\n\n";
}

// this is just so the command responses are printed on the screen,
// also clears it after printing so that if the next command has no response it wont print anything
void render_response_message(const std::string& message) {
    if (message.empty()) {
        std::cout << "\033[K\n";
        return;
    }
    std::stringstream ss(message);
    std::string line;
    while (std::getline(ss, line)) {
        std::cout << line << "\033[K\n";
    }
    std::cout << "\033[K\n";
}

// this is to indicate where to type, clears it after each command is inputted 
void render_prompt(const std::string& input_text) {
    std::cout << "Command> " << input_text << "\033[K\033[J" << std::flush;
}

}