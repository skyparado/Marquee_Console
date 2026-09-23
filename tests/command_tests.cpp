#include "marquee/ascii_art.hpp"
#include "marquee/commands.hpp"
#include "marquee/input.hpp"

#include <iostream>
#include <stdexcept>

int line_of_last_check = 0;

// __LINE__ is captured so a failure names the assertion that broke.
#define check(condition) do { line_of_last_check = __LINE__; check_impl(condition); } while (0)

void check_impl(bool condition) {
    if (!condition) throw std::runtime_error("Test failed");
}

void run_tests() {
    using namespace marquee;
    SharedState state;
    check(state.snapshot().status == Status::Stopped);
    check(execute_command(" help ", state).message.find("set_speed") != std::string::npos);
    check(execute_command("start_marquee", state).ok);
    check(state.snapshot().status == Status::Running);
    check(execute_command("start_marquee", state).ok);
    {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.value.position = 42;
    }
    check(execute_command("stop_marquee", state).ok);
    check(state.snapshot().position == 42);
    check(execute_command("set_text Hello from Sky!", state).ok);
    check(state.snapshot().text == "Hello from Sky!");
    check(state.snapshot().position == 0);
    check(execute_command("set_text \"  spaced text  \"", state).ok);
    check(state.snapshot().text == "  spaced text  ");
    check(execute_command("set_speed 1", state).ok);
    check(execute_command("set_speed 2147483647", state).ok);
    const auto before = state.snapshot();
    for (const auto* bad : {"set_speed", "set_speed 0", "set_speed -1", "set_speed +1",
                           "set_speed 1.5", "set_speed 20ms", "set_speed 10 20",
                           "set_speed 2147483648", "set_text", "set_text \"\"",
                           "set_text \"unterminated", "set_text a\tb", "set_text '   '",
                           "start_marquee extra", "stop_marquee extra", "exit extra", "help extra", "unknown"}) {
        check(!execute_command(bad, state).ok);
        const auto after = state.snapshot();
        check(after.text == before.text && after.speed_ms == before.speed_ms &&
              after.position == before.position && after.status == before.status && !after.exit_requested);
    }
    check(execute_command("start_marquee", state).ok);
    check(execute_command("set_speed 25", state).ok);
    check(state.snapshot().status == Status::Running && state.snapshot().speed_ms == 25);
    check(execute_command("set_text live update", state).ok);
    check(state.snapshot().status == Status::Running);
    check(execute_command("exit", state).ok);
    check(state.snapshot().exit_requested && state.snapshot().status == Status::Stopped);
    check(!execute_command("start_marquee", state).ok);

    InputBuffer input;
    InputBatch batch;
    for (char key : std::string("helx\bp\r\nset_speed 25\r")) input.accept(key, batch);
    check(batch.lines.size() == 2 && batch.lines[0] == "help" && batch.lines[1] == "set_speed 25");
    input.accept(224, batch);
    input.accept(75, batch);
    check(input.current_line().empty());
    for (char key : std::string("discard")) input.accept(key, batch);
    input.accept(27, batch);
    check(input.current_line().empty());
    for (std::size_t i = 0; i <= InputBuffer::max_length; ++i) input.accept('x', batch);
    input.accept('\r', batch);
    check(batch.limit_reached && batch.lines.size() == 2);
    for (char key : std::string("help\r")) input.accept(key, batch);
    check(batch.lines.size() == 3 && batch.lines.back() == "help");
    // scroll math: the banner must leave the screen entirely before it wraps,
    // which only holds when the cycle covers console width + banner width
    const int width = 20;
    const std::string banner = "ABCDE";
    const int cycle = width + static_cast<int>(banner.size());
    check(advance_position(cycle - 1, cycle) == 0);
    check(advance_position(0, 0) == 0);
    // Rendered rows are trimmed: trailing padding is dropped (the renderer's
    // \033[K clears the rest of the line) and the last column is never used,
    // so a glyph cannot fill it and make the terminal auto-wrap.
    check(compute_scrolled_line(banner, 0, width) == banner);
    check(compute_scrolled_line(banner, width, width).empty());
    check(compute_scrolled_line("", 0, width).empty());
    check(static_cast<int>(compute_scrolled_line(banner, 0, width).size()) <= width - 1);
    // The banner must sit one column further right after a single step.
    check(compute_scrolled_line(banner, 1, width) == " " + banner);

    std::cout << "All command, input-buffer, and scroll tests passed.\n";
}

// An uncaught throw from check() terminated the process with no output at all,
// which made a failing suite look like a crash. Report the failure instead.
int main() {
    try {
        run_tests();
    } catch (const std::exception& ex) {
        std::cout << "FAILED at " << __FILE__ << ":" << line_of_last_check
                  << " - " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
