// emulator.h
#pragma once
#include <vector>
#include <string>
#include <unordered_set>

enum class EmulatorState {
    Stopped,
    Running,
    Exited
};

struct Instruction {
    std::string op;
    int arg;
};

class Emulator {
public:
    Emulator();

    void loadProgram(const std::vector<Instruction>& prog);
    void reset();

    void run();         // до брейкпоинта или конца
    void step();        // один шаг
    void stop();        // принудительная остановка

    void addBreakpoint(int pc);
    void removeBreakpoint(int pc);
    void clearBreakpoints();

    int getPC() const { return pc; }
    int getAcc() const { return acc; }
    EmulatorState getState() const { return state; }

private:
    void executeCurrent();

    std::vector<Instruction> program;
    std::unordered_set<int> breakpoints;
    int pc;
    int acc;
    EmulatorState state;
};
