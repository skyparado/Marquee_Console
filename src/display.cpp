#include "marquee/display.hpp"
#include "marquee/ascii_art.hpp"

#include <iostream>
#include <algorithm>
#include <sstream>
#include <vector>

namespace marquee {

// moves the cursor to the top of the console, so the next frame overwrites the previous one
void reset_cursor_to_top() {
    std::cout << "\033[H";
}

// clears the entire console screen, so each frame starts from a blank window
void clear_entire_screen() {
    std::cout << "\033[2J";
}

// prints the welcome message, the group developers, and the version date
void render_header() {
    std::cout << "Welcome to CSOPESY!\033[K\n\n";
    std::cout << "Group developers:\033[K\n";
    std::cout << "Garcia, Theodore\033[K\n";
    std::cout << "Magbatoc, Ethan\033[K\n";
    std::cout << "Parado, Sky\033[K\n";
    std::cout << "Villorente, Khyle\033[K\n\n";
    std::cout << "Version date: Sept 20, 2026\033[K\n\n";
}

// combines the ascii_art helpers into the five rendered banner rows
void render_ascii_marquee(const std::string& text, int position, int width) {
    std::cout << "\033[K\n\n\n";
    // blank rows keep the frame the same height when there is no text to draw,
    // so nothing below the banner shifts up
    if (!text.empty()) {
        std::vector<std::string> ascii_banner = generate_ascii_art(text);
        for (int row = 0; row < 5; ++row) {
            std::string line = compute_scrolled_line(ascii_banner[row], position, width);
            std::cout << line << "\033[K\n";
        }
    } else {
        for (int i = 0; i < 5; ++i) {
            std::cout << "\033[K\n";
        }
    }
    std::cout << "\033[K\n\n";
}

// prints the last command's response; an empty message clears the line instead,
// so a response never lingers after the next command
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

// prints the prompt and the line being typed; the trailing escape codes clear
// the rest of the line and everything below it
void render_prompt(const std::string& input_text) {
    std::cout << "Command> " << input_text << "\033[K\033[J" << std::flush;
}

}