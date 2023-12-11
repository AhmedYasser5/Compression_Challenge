#include <gtest/gtest.h>

#include <compare>
#include <cstdint>
#include <fstream>
#include <string>

#include "bitstream.hpp"

using namespace std;

class obitstreamTesting : public testing::TestWithParam<string> {
   public:
    ~obitstreamTesting() override {}

    void SetUp() override { output.open(filename); }

    void Validate() {
        output.close();
        ifstream input(filename, ios::binary);
        char read_byte;
        for (auto letter : GetParam()) {
            const auto byte = static_cast<uint8_t>(letter);
            input.read(&read_byte, 1);
            EXPECT_EQ(byte, static_cast<uint8_t>(read_byte));
        }
        EXPECT_EQ(input.peek(), EOF);
    }

    void TearDown() override { Validate(); }

   public:
    static const char* filename;
    obitstream output;
};

const char* obitstreamTesting::filename = "obitstream.test.in";

TEST_P(obitstreamTesting, BitByBitOperator) {
    for (auto letter : GetParam()) {
        auto byte = static_cast<uint8_t>(letter);
        for (int _ = 0; _ < 8; _++, byte <<= 1) {
            const bool bit = byte >> 7;
            output << bit;
        }
    }
}

TEST_P(obitstreamTesting, BitByBit) {
    for (auto letter : GetParam()) {
        auto byte = static_cast<uint8_t>(letter);
        for (int _ = 0; _ < 8; _++, byte <<= 1) {
            const bool bit = byte >> 7;
            output.write(bit);
        }
    }
}

TEST_P(obitstreamTesting, ByteByByte) {
    for (auto letter : GetParam()) {
        const auto byte = static_cast<uint8_t>(letter);
        output.write_byte(byte);
    }
}

TEST_P(obitstreamTesting, Mixed) {
    decltype(GetParam().cbegin()) it;
    uint8_t shifts;
    auto write = [&it, &shifts]() -> bool {
        if (it == GetParam().cend()) {
            return false;
        }
        const auto ret = static_cast<uint8_t>(*it) & (1 << (7 - shifts));
        if (++shifts == 8) {
            shifts = 0;
            ++it;
        }
        return ret;
    };
    auto pretty_binary = [](auto mask) -> string {
        string bin;
        for (size_t _ = 0; _ < sizeof(mask); _++, mask >>= 1) {
            bin += bool(mask & 1) + '0';
        }
        return bin;
    };
    for (uint8_t mask = 1;; mask++) {
        SCOPED_TRACE("At mask = "s + pretty_binary(mask));
        it = GetParam().cbegin();
        shifts = 0;
        while (it != GetParam().cend() || shifts != 0) {
            SCOPED_TRACE("At it = "s + to_string(it - GetParam().cbegin()) +
                         ", shifts = "s + to_string(shifts));
            if (8 - shifts + (next(it) != GetParam().cend()) * 8 < 8 ||
                (mask & (1 << shifts))) {
                output.write(write());
                continue;
            }
            uint8_t byte = 0;
            for (int _ = 0; _ < 8; _++) {
                byte <<= 1;
                byte |= write();
            }
            output.write_byte(byte);
        }
        Validate();
        if (mask == 0) {
            break;
        }
        output.open(filename);
    }
}

INSTANTIATE_TEST_SUITE_P(
    obitstreamSuite, obitstreamTesting,
    testing::Values("Hello, World!"s, "Ahmed Yasser"s, ""s,
                    "The big brown fox jumps over the lazy dog"s));
