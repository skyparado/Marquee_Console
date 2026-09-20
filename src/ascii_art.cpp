#include "marquee/ascii_art.hpp"

#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace marquee {

namespace {

// font dictionary for ascii so i can just call each array and display them
const std::unordered_map<char, std::vector<std::string>> RAW_ASCII_FONT = 
{
    // A-Z Letters
    {'A', {"  ___  ", " / _ \\ ", "| |_| |", "|  _  |", "|_| |_|"}},
    {'B', {" ____  ", "|  _ \\ ", "| |_) |", "|  _ < ", "|____/ "}},
    {'C', {" ____ ", "/ ___|", "| |   ", "| |___", "\\____|"}},
    {'D', {" ____ ", "|  _ \\", "| | | |", "| |_| |", "|____/ "}},
    {'E', {" _____", "|  ___|", "| |___ ", "|  ___|", "|_____|"}},
    {'F', {" _____", "|  ___|", "| |___ ", "|  ___|", "|_|    "}},
    {'G', {" ____ ", "/ ___|", "| |  _", "| |_| |", "\\____/ "}},
    {'H', {" _   _ ", "| | | |", "| |_| |", "|  _  |", "|_| |_|"}},
    {'I', {" _____ ", "|_   _|", "  | |  ", " _| |_ ", "|_____|"}},
    {'J', {"     _ ", "    | |", " _  | |", "| |_| |", " \\___/ "}},
    {'K', {" _  __", "| |/ /", "| ' / ", "| . \\ ", "|_|\\_\\"}},
    {'L', {" _    ", "| |   ", "| |   ", "| |___", "|_____|"}},
    {'M', {" __  __ ", "|  \\/  |", "| |\\/| |", "| |  | |", "|_|  |_|"}},
    {'N', {" _   _ ", "| \\ | |", "|  \\| |", "| |\\  |", "|_| \\_|"}},
    {'O', {"  ___  ", " / _ \\ ", "| | | |", "| |_| |", " \\___/ "}},
    {'P', {" ____ ", "|  _ \\", "| |_) |", "|  __/ ", "|_|    "}},
    {'Q', {"  ___  ", " / _ \\ ", "| | | |", "| |_| |", " \\__\\_\\"}},
    {'R', {" ____ ", "|  _ \\", "| |_) |", "|  _ < ", "|_| \\_\\"}},
    {'S', {" ____ ", "/ ___|", "\\___ \\", " ___) |", "|____/ "}},
    {'T', {" _____ ", "|_   _|", "  | |  ", "  | |  ", "  |_|  "}},
    {'U', {" _   _ ", "| | | |", "| | | |", "| |_| |", " \\___/ "}},
    {'V', {" _   _ ", "| | | |", "| | | |", "| \\_/ |", " \\___/ "}},
    {'W', {" _     _ ", "| |   | |", "| |   | |", "| \\/ \\/ |", " \\_/ \\_/ "}},
    {'X', {" __  __ ", " \\ \\/ / ", "  \\  /  ", "  /  \\  ", " /_/\\_\\ "}},
    {'Y', {" __  __ ", " \\ \\/ / ", "  \\  /  ", "   | |  ", "   |_|  "}},
    {'Z', {" _____ ", "|___  |", "   / / ", "  / /  ", " /____|"}},

    // --- Digits ---
    {'0', {"  ___  ", " / _ \\ ", "| | | |", "| |_| |", " \\___/ "}},
    {'1', {"  __ ", " /_ |", "  | |", "  | |", "  |_|"}},
    {'2', {"  ___  ", " |__ \\ ", "   / / ", "  / /_ ", " |____|"}},
    {'3', {"  ____  ", " |___ \\ ", "   __) |", "  |__ < ", " |____/ "}},
    {'4', {"  _  _  ", " | || | ", " | || |_", " |__   _|", "    |_| "}},
    {'5', {"  _____ ", " | ____|", " | |__  ", " |___ \\ ", " |____/ "}},
    {'6', {"   ____ ", "  / ___|", " | |___ ", " | ___ \\", "  \\____/"}},
    {'7', {"  ______", " |___  /", "    / / ", "   / /  ", "  /_/   "}},
    {'8', {"  ___  ", " ( _ ) ", " / _ \\ ", "| (_) |", " \\___/ "}},
    {'9', {"  ___  ", " / _ \\ ", "| (_) |", " \\__, |", "   /_/ "}},

    // --- Punctuation & Symbols ---
    {' ', {"   ", "   ", "   ", "   ", "   "}},
    {'!', {" _ ", "| |", "| |", "|_|", "(_)"}},
    {'?', {"  ___  ", " /__ \\ ", "   / / ", "  |_|  ", "  (_)  "}},
    {'-', {"       ", " _____ ", "(_____)", "  ", "       "}},
    {'.', {"   ", "   ", "   ", " _ ", "(_)"}},
    {':', {" _ ", "(_)", "   ", " _ ", "(_)"}},
    {'*', {"       ", "  \\|/  ", "  -*-  ", "  /|\\  ", "       "}},
    {'#', {" # # ", "#####", " # # ", "#####", " # # "}},
    {'<', {"   _ ","  / / ", " / /  ", " \\ \\  ", "  \\_\\ "}},
    {'>', {" _   "," \\ \\  ", "  \\ \\ ", "  / / ", " /_/  "}},
    {'+', {"   _   ", "  | |  ", " _| |_ ", "|_   _|", "  |_|  "}},
    {'=', {"       ", " _____ ", "(_____)"," _____ ", "(_____)"}},
    {'/', {"     __", "    / /", "   / / ", "  / /  ", " /_/   "}},
    {'\\',{"__     ","\\ \\    "," \\ \\   ","  \\ \\  ","   \\_\\ "}},
    {'(', {"  __", " / /", "| | ", "| | ", " \\_\\"}},
    {')', {"__  ", "\\ \\ ", " | |", " | |", "/_/ "}},
};

// pad each row of the glyph so all rows are even; keeps alignment consistent and pretty
std::vector<std::string> get_padded_glyph(const std::vector<std::string>& raw_glyph) {
    std::size_t max_len = 0;
    
    for (const auto& row : raw_glyph) {
        max_len = std::max(max_len, row.length());
    }

    std::vector<std::string> padded = raw_glyph;
    for (auto& row : padded) {
        if (row.length() < max_len) {
            row.append(max_len - row.length(), ' ');
        }
    }
    return padded;
}

} 

// this is the actual function generating ascii art, calls the dictionary and the get_padded_glyph function
std::vector<std::string> generate_ascii_art(const std::string& text) {
    std::vector<std::string> ascii_rows(5, "");
    //loop through each inputted char, convert to uppercase, search for the glyph, then append to ascii_rows
    for (char ch : text) {
        char upper_ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        auto it = RAW_ASCII_FONT.find(upper_ch);
        
        // this bit just checks if the char is in the dictionary, if not it uses the '?' glyph
        const auto& raw_glyph = (it != RAW_ASCII_FONT.end()) ? it->second : RAW_ASCII_FONT.at('?');

        std::vector<std::string> glyph = get_padded_glyph(raw_glyph);
        for (int r = 0; r < 5; ++r) {
            ascii_rows[r] += glyph[r] + " ";
        }
    }
    return ascii_rows;
}

// now we compute scroll line so that the "animation" looks seamless
std::string compute_scrolled_line(const std::string& text, int position, int console_width) {
    std::string display_line(console_width, ' ');
    if (text.empty()) return display_line;

    int len = static_cast<int>(text.length());
    // total cycle length includes both console width AND the text length so that
    // the text can scroll completely off-screen before wrapping around!
    int cycle_len = console_width + len;

    for (int i = 0; i < len; ++i) {
        int col = (position + i) % cycle_len;
        // this is to make sure we only render characters that fall strictly inside the visible console width
        // aka: don't render the ones "off-screen" to make the wrap around effect look real
        if (col >= 0 && col < console_width) {
            display_line[col] = text[i];
        }
    }
    return display_line;
}

// this is the function that akshually moves the text along the screen, 
// just increments it until it reaches the complete cycle length then wraps back around so we can keep looping
int advance_position(int current_pos, int max_cycle) {
    if (max_cycle <= 0) return 0;
    return (current_pos + 1) % max_cycle;
}
}