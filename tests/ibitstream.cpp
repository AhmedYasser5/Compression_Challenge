#include <gtest/gtest.h>

#include <compare>
#include <cstdint>
#include <fstream>
#include <string>

#include "bitstream.hpp"

using namespace std;

class ibitstreamTesting : public testing::TestWithParam<string> {
   public:
    ~ibitstreamTesting() override {}

    void SetUp() override {
        ofstream out(filename);
        out << GetParam() << endl;
        out.close();
        input.open(filename);
    }

   public:
    static const char* filename;
    ibitstream input;
};

const char* ibitstreamTesting::filename = "ibitstream.test.in";

TEST_P(ibitstreamTesting, BitByBitOperator) {
    for (auto letter : GetParam()) {
        auto byte = static_cast<uint8_t>(letter);
        for (int _ = 0; _ < 8; _++, byte <<= 1) {
            const bool bit = byte >> 7;
            bool read_bit;
            input >> read_bit;
            EXPECT_EQ(read_bit, bit);
        }
    }
}

TEST_P(ibitstreamTesting, BitByBit) {
    for (auto letter : GetParam()) {
        auto byte = static_cast<uint8_t>(letter);
        for (int _ = 0; _ < 8; _++, byte <<= 1) {
            const bool bit = byte >> 7;
            EXPECT_EQ(input.read(), bit);
        }
    }
}

TEST_P(ibitstreamTesting, ByteByByte) {
    for (auto letter : GetParam()) {
        const auto byte = static_cast<uint8_t>(letter);
        EXPECT_EQ(input.read_byte(), byte);
    }
}

TEST_P(ibitstreamTesting, Mixed) {
    decltype(GetParam().cbegin()) it;
    uint8_t shifts;
    auto read = [&it, &shifts]() -> bool {
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
                ASSERT_EQ(input.read(), read());
                continue;
            }
            uint8_t byte = 0;
            for (int _ = 0; _ < 8; _++) {
                byte <<= 1;
                byte |= read();
            }
            ASSERT_EQ(input.read_byte(), byte);
        }
        if (mask == 0) {
            break;
        }
        input.close();
        input.open(filename);
    }
}

INSTANTIATE_TEST_SUITE_P(
    ibitstreamSuite, ibitstreamTesting,
    testing::Values("Hello, World!"s, "Ahmed Yasser"s, ""s,
                    "The big brown fox jumps over the lazy dog"s));
