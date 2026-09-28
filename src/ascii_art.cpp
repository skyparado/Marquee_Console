#include "marquee/ascii_art.hpp"

#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace marquee {

namespace {

// font dictionary: each character maps to the five rows that draw it
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

// pads every row of a glyph to the same width, so the five banner rows stay aligned
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

// builds the five-row banner for a string, one glyph at a time
std::vector<std::string> generate_ascii_art(const std::string& text) {
    std::vector<std::string> ascii_rows(5, "");
    for (char ch : text) {
        char upper_ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        auto it = RAW_ASCII_FONT.find(upper_ch);

        // characters missing from the font fall back to '?' rather than being skipped,
        // so the banner width still matches the text length
        const auto& raw_glyph = (it != RAW_ASCII_FONT.end()) ? it->second : RAW_ASCII_FONT.at('?');

        std::vector<std::string> glyph = get_padded_glyph(raw_glyph);
        for (int r = 0; r < 5; ++r) {
            ascii_rows[r] += glyph[r] + " ";
        }
    }
    return ascii_rows;
}

// windows one banner row at the current scroll offset, so the animation looks seamless
std::string compute_scrolled_line(const std::string& text, int position, int console_width) {
    if (text.empty() || console_width <= 0) return "";

    // the last column is left free: filling it makes the terminal auto-wrap, which
    // costs an extra screen line per row and scrolls the whole frame
    int usable_width = (console_width > 1) ? console_width - 1 : 1;
    int len = static_cast<int>(text.length());
    // total cycle length includes both console width AND the text length so that
    // the text can scroll completely off-screen before wrapping around
    int cycle_len = console_width + len;

    std::string line(usable_width, ' ');
    int max_col = -1; // track the rightmost visible character to trim trailing spaces

    for (int i = 0; i < len; ++i) {
        int col = (position + i) % cycle_len;
        // only characters that land strictly inside the visible width are drawn;
        // leaving the off-screen ones out is what makes the wrap-around look real
        if (col >= 0 && col < usable_width)
        {
            line[col] = text[i];
            if (text[i] != ' ') {
                max_col = std::max(max_col, col);
            }
        }
    }

    // trailing padding is dropped because the renderer's \033[K already clears
    // the rest of the line; an entirely off-screen row becomes an empty string
    if (max_col < 0) return "";
    line.resize(max_col + 1);
    return line;
}

// steps the marquee one column along, wrapping back to zero once a full cycle
// has been covered so the scroll keeps looping
int advance_position(int current_pos, int max_cycle) {
    if (max_cycle <= 0) return 0;
    return (current_pos + 1) % max_cycle;
}
}