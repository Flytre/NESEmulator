#include <iostream>
#include "cpu.cpp"
#include <fstream>
int main() {
    Cpu6502 cpu;
    std::ifstream file("/Users/flytre/CLionProjects/NESEmulator/nestest.nes"); // Open the file
    if (!file.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return 1;
    }
    cpu.load_rom(file);
    cout << "DONE" << endl;
}
