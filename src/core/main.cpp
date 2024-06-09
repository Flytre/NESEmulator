#include <iostream>
#include "cpu.cpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include "instructions.h"
#include <filesystem>


using json = nlohmann::json;

void print_ram(const json &ram) {
    // Create a vector of pairs to store the addresses and values
    vector<pair<int, int>> sorted_ram;
    for (auto &entry: ram) {
        auto addr = entry[0].get<int>();
        auto val = entry[1].get<int>();
        sorted_ram.emplace_back(addr, val);
    }

    // Sort the vector by address
    sort(sorted_ram.begin(), sorted_ram.end());

    // Print the sorted RAM values
    for (const auto &entry: sorted_ram) {
        auto addr = entry.first;
        auto val = entry.second;
        cerr << "RAM Address: 0x" << hex << addr << " (" << dec << addr << ")"
             << ", Value: 0x" << hex << val << " (" << dec << val << ")" << endl;
    }
}

//see: https://github.com/SingleStepTests/ProcessorTests/tree/main/nes6502
void test_json(string file_str) {
    std::ifstream file(file_str);
    json jsonObject;
    file >> jsonObject;
    string PAUSE_ON = "08 60 be";
    for (auto &obj: jsonObject) {
        auto initial = obj["initial"];
        Cpu6502 cpu;
        cpu.power();
        string name = obj["name"];
        if (name == PAUSE_ON) {
            cout << "BREAKPOINT" << endl;
        }
        cpu.reg().setPC(Addr(initial["pc"].get<int>()));
        cpu.reg().setS(Val(initial["s"].get<int>()));
        cpu.reg().setA(Val(initial["a"].get<int>()));
        cpu.reg().setX(Val(initial["x"].get<int>()));
        cpu.reg().setY(Val(initial["y"].get<int>()));
        cpu.reg().setP(Val(initial["p"].get<int>()));
        auto ram = initial["ram"];
        for (auto &entry: ram) {
            auto addr = entry[0].get<int>();
            auto val = entry[1].get<int>();
            cpu.mem().write_byte(addr, val);
        }
        auto instr = cpu.cpu_state().get_byte(cpu.cpu_state().reg().getPC()).val;
        cpu.cpu_state().reg().incrPC();
        if (name == PAUSE_ON) {
            cout << "BREAKPOINT" << endl;
        }
        auto cycles = instructions[instr]->act(cpu.cpu_state());
        auto final = obj["final"];

        try {
            if (cpu.reg().getPC().addr != final["pc"].get<int>()) {
                throw runtime_error("PC mismatch");
            }
            if (cpu.reg().getS().val != final["s"].get<int>()) {
                throw runtime_error("S mismatch");
            }
            if (cpu.reg().getA().val != final["a"].get<int>()) {
                throw runtime_error("A mismatch");
            }
            if (cpu.reg().getX().val != final["x"].get<int>()) {
                throw runtime_error("X mismatch");
            }
            if (cpu.reg().getY().val != final["y"].get<int>()) {
                throw runtime_error("Y mismatch");
            }
            if (cpu.reg().getP().val != final["p"].get<int>()) {
                throw runtime_error("P mismatch");
            }
            auto final_ram = final["ram"];
            for (auto &entry: final_ram) {
                auto addr = entry[0].get<int>();
                auto val = entry[1].get<int>();
                if (cpu.mem().read_byte(addr) != val) {
                    throw runtime_error("RAM mismatch at address " + to_string(addr));
                }
            }
        } catch (const std::exception &e) {
            cerr << "Test failed for " << name << endl;
            cerr << "Reason: " << e.what() << endl;
            cerr << "Initial state: " << initial.dump() << endl;
            cerr << "Initial ram: " << endl;
            print_ram(initial["ram"]);
            cerr << "--------------------------------" << endl;
            cerr << "--Expected Output--" << endl;
            cerr << "Final state: " << final.dump() << endl;
            cerr << "Final ram: " << endl;
            print_ram(final["ram"]);
            cerr << "--------------------------------" << endl;
            cerr << "--User Output--" << endl;
            cerr << "CPU Registers:" << endl;
            cerr << "PC: " << (int) cpu.reg().getPC().addr << endl;
            cerr << "S: " << (int) cpu.reg().getS().val << endl;
            cerr << "A: " << (int) cpu.reg().getA().val << endl;
            cerr << "X: " << (int) cpu.reg().getX().val << endl;
            cerr << "Y: " << (int) cpu.reg().getY().val << endl;
            cerr << "P: " << (int) cpu.reg().getP().val << endl;
            assert(false); // Force a failure in the catch block
        }
    }
    cout << "PASSED" << endl;
}

void test_sample() {
    Cpu6502 cpu;
    std::ifstream file("../tests/sample.nes"); // Open the file
    if (!file.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return;
    }
    cpu.load_rom(file);
    cpu.power();
    for (int i = 0; i < 4; i++)
        cpu.posedge_clock();
}

int main() {
    namespace fs = std::filesystem;
    std::string test_dir = "../tests/v1/";

    // Iterate over all files in the directory
    for (const auto& entry : fs::directory_iterator(test_dir)) {
        // Check if the file has a .json extension
        if (entry.path().extension() == ".json") {
            std::string filename = entry.path().string();
            test_json(filename);
        }
    }

}
