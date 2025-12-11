#include <iostream>
#include <vector>
#include "emulator.h"

int main() {
    Emulator emu;

    // Тестовая программа:
    // 0: acc += 1
    // 1: acc += 2
    // 2: acc -= 1
    // 3: HLT
    std::vector<Instruction> prog = {
        {"ADD", 1},
        {"ADD", 2},
        {"SUB", 1},
        {"HLT", 0}
    };

    emu.loadProgram(prog);

    std::cout << "Initial state: pc=" << emu.getPC()
              << " acc=" << emu.getAcc() << "\n";

    while (emu.getState() != EmulatorState::Exited) {
        emu.step();
        std::cout << "After step: pc=" << emu.getPC()
                  << " acc=" << emu.getAcc()
                  << " state=" << (int)emu.getState() << "\n";
    }

    std::cout << "Program finished. Final acc=" << emu.getAcc() << "\n";
    return 0;
}
