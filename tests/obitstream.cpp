#include <gtest/gtest.h>

#include <compare>
#include <cstdint>
#include <fstream>
#include <string>

#include "bitstream.hpp"

using namespace std;

using character_type = BitStream::character_type;

class obitstreamTesting : public testing::TestWithParam<string> {
   public:
    ~obitstreamTesting() override {}

    void SetUp() override {
        param.reserve(GetParam().size());
        for (auto letter : GetParam()) {
            param += *(character_type*)(&letter);
        }
        output.open(filename);
    }

    void Validate() {
        output.close();
        basic_ifstream<character_type> input(filename, ios::binary);
        character_type read_byte;
        for (auto letter : GetParam()) {
            const auto byte = letter;
            input.read(&read_byte, 1);
            EXPECT_EQ(byte, read_byte);
        }
        EXPECT_EQ(input.peek(), EOF);
    }

    const string& GetParam() { return param; }

    void TearDown() override { Validate(); }

   public:
    static const char* filename;
    BitStream::obitstream output;
    string param;
};

const char* obitstreamTesting::filename = "obitstream.test.in";

TEST_P(obitstreamTesting, BitByBitOperator) {
    for (auto letter : GetParam()) {
        auto byte = letter;
        for (int _ = 0; _ < 8; _++, byte <<= 1) {
            const bool bit = byte >> 7;
            output << bit;
        }
    }
}

TEST_P(obitstreamTesting, BitByBit) {
    for (auto letter : GetParam()) {
        auto byte = letter;
        for (int _ = 0; _ < 8; _++, byte <<= 1) {
            const bool bit = byte >> 7;
            output.write(bit);
        }
    }
}

TEST_P(obitstreamTesting, ByteByByte) {
    for (auto letter : GetParam()) {
        const auto byte = letter;
        output.write_unit(byte);
    }
}

TEST_P(obitstreamTesting, Mixed) {
    decltype(GetParam().cbegin()) it;
    uint8_t shifts;
    auto write = [this, &it, &shifts]() -> bool {
        if (it == GetParam().cend()) {
            return false;
        }
        const auto ret = *it & (1 << (7 - shifts));
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
            character_type unit = 0;
            for (int _ = 0; _ < 8; _++) {
                unit <<= 1;
                unit |= write();
            }
            output.write_unit(unit);
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
    testing::Values("Hello, World!", "Ahmed Yasser", "",
                    "The big brown fox jumps over the lazy dog"));
