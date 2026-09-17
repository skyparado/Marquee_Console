#include "marquee/commands.hpp"
#include "marquee/input.hpp"

#include <iostream>
#include <stdexcept>

void check(bool condition) {
    if (!condition) throw std::runtime_error("Test failed");
}

int main() {
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
    std::cout << "All command and input-buffer tests passed.\n";
}
