// emulator.cpp
#include "emulator.h"

Emulator::Emulator() : pc(0), acc(0), state(EmulatorState::Stopped) {}

void Emulator::loadProgram(const std::vector<Instruction>& prog) {
    program = prog;
    reset();
}

void Emulator::reset() {
    pc = 0;
    acc = 0;
    state = EmulatorState::Stopped;
}

void Emulator::run() {
    if (state == EmulatorState::Exited) {
        return;
    }

    state = EmulatorState::Running;

    while (true) {

        if (pc < 0 || pc >= (int)program.size()) {
            state = EmulatorState::Exited;
            break;
        }

        
        if (breakpoints.count(pc)) {
            state = EmulatorState::Stopped;
            break;
        }


        executeCurrent();


        if (state == EmulatorState::Exited) {
            break;
        }
    }
}


void Emulator::step() {
    if (state == EmulatorState::Exited) {
        return; 
    }

    state = EmulatorState::Running;

    // Защита от выхода за пределы программы
    if (pc < 0 || pc >= (int)program.size()) {
        state = EmulatorState::Exited;
        return;
    }

    executeCurrent();

    
    if (state == EmulatorState::Exited) {
        return;
    }

    
    if (pc < 0 || pc >= (int)program.size()) {
        state = EmulatorState::Exited;
    } else {

        state = EmulatorState::Stopped;
    }
}


void Emulator::stop() {
    if (state == EmulatorState::Running)
        state = EmulatorState::Stopped;
}

void Emulator::addBreakpoint(int p) {
    breakpoints.insert(p);
}

void Emulator::removeBreakpoint(int p) {
    breakpoints.erase(p);
}

void Emulator::clearBreakpoints() {
    breakpoints.clear();
}

void Emulator::executeCurrent() {
    const Instruction& instr = program[pc];
    if (instr.op == "ADD") {
        acc += instr.arg;
        pc++;
    } else if (instr.op == "SUB") {
        acc -= instr.arg;
        pc++;
    } else if (instr.op == "JMP") {
        pc = instr.arg;
    } else if (instr.op == "HLT") {
        state = EmulatorState::Exited;
    } else {
        state = EmulatorState::Exited;
    }
}
