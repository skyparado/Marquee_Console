#pragma once

#include "marquee/commands.hpp"
#include <string>

namespace marquee {

void reset_cursor_to_top();
void clear_entire_screen();
void render_header();
void render_ascii_marquee(const std::string& text, int position, int width);
void render_response_message(const std::string& message);
void render_prompt(const std::string& input_text);

}