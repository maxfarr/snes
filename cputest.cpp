#include "common.h"
#include "cpu.hpp"
#include "flat_memory.hpp"
#include "deps/json/json.hpp"

#include <memory>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using json = nlohmann::json;

#define BYTE_STRING(b) std::format("{:02X}", b)

static constexpr std::pair<const char*, uint32_t CPUState::*> FIELDS[] = {
    {"pc", &CPUState::pc}, {"s", &CPUState::s},     {"p", &CPUState::p},
    {"a",  &CPUState::a},  {"x", &CPUState::x},     {"y", &CPUState::y},
    {"dbr",&CPUState::dbr},{"d", &CPUState::d},     {"pbr",&CPUState::pbr},
    {"e",  &CPUState::e},
};

CPUState cpuStateFromJson(const json& j) {
    CPUState s{};
    for (auto [name, field] : FIELDS) s.*field = j[name].get<uint32_t>();
    return s;
}

int main() {
    std::unique_ptr<FLAT_MEMORY> mem = std::make_unique<FLAT_MEMORY>();
    std::unique_ptr<SNES_CPU> cpu = std::make_unique<SNES_CPU>(mem.get());

    std::vector<fs::path> paths;

    for (uint16_t i = 0; i < 256; ++i) {
        if (i == 0x54 || i == 0x44) {
            // todo: implement these tests
            continue;
        }

        byte op = static_cast<uint8_t>(i);

        paths.push_back("65816_tests/v1/" + std::format("{:02x}", op) + ".e.json");
        paths.push_back("65816_tests/v1/" + std::format("{:02x}", op) + ".n.json");
    }

    std::vector<std::string> failed_test_names;

    size_t num_tests_executed = 0;
    size_t num_tests_failed = 0;
    for (const auto& path : paths) {
        std::ifstream in(path);
        json tests_json = json::parse(in);

        for (const json& test : tests_json) {
            num_tests_executed++;
            mem->clear();
            const json& init = test["initial"];
            std::string test_name = test["name"];

            for (const auto& entry : init["ram"]) {
                threebyte addr = entry[0];
                byte value = entry[1];
                mem->write(addr, value);
            }

            cpu->e = init["e"].get<int>() != 0;
            cpu->PC = init["pc"];
            cpu->S = init["s"];
            cpu->setP(init["p"]);
            cpu->C = init["a"];
            cpu->X = init["x"];
            cpu->Y = init["y"];
            cpu->DBR = init["dbr"];
            cpu->D = init["d"];
            cpu->K = init["pbr"];
            if (cpu->e) cpu->setE(true);

            try {
                cpu->executeNextCommand();

                CPUState expected = cpuStateFromJson(test["final"]);
                CPUState actual   = cpu->getState();
                bool ok = (actual == expected);

                std::vector<std::string> diffs;
                if (!ok)
                    for (auto [field_name, field] : FIELDS)
                        if (actual.*field != expected.*field)
                            diffs.push_back(std::format("test: {} | field: {} | expected {:04X} got {:04X}",
                                                        test_name, field_name, expected.*field, actual.*field));

                for (const auto& entry : test["final"]["ram"]) {
                    threebyte addr = entry[0];
                    byte want = entry[1];
                    byte got = mem->read(addr);
                    if (got != want) {
                        ok = false;
                        diffs.push_back(std::format("ram[{:06X}]: expected {:02X} got {:02X}", addr, want, got));
                    }
                }

                if(!ok) {
                    num_tests_failed++;
                    std::cout << "test failed: " << test_name << std::endl;
                    std::cout << "diffs:" << std::endl;
                    for (const std::string& diff : diffs) {
                        std::cout << diff << std::endl;
                    }
                }
            } catch (const std::exception& e) {
                num_tests_failed++;
                std::cout << "test name " << test_name << " threw: " << e.what() << std::endl;
            }
        }
    }
    std::cout << "===========≈≈≈ RESULTS ≈≈≈===========" << std::endl;
    std::cout << "= total tests: " << num_tests_executed << std::endl;
    std::cout << "= tests failed: " << num_tests_failed << std::endl;
    std::cout << "= tests passed: " << num_tests_executed - num_tests_failed << std::endl;
    std::cout << "=====================================" << std::endl;
}