#pragma once

#include <string>
#include <vector>

namespace marquee {

std::vector<std::string> generate_ascii_art(const std::string& text);
std::string compute_scrolled_line(const std::string& text, int position, int console_width);
int advance_position(int current_pos, int max_width);

}