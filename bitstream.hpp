#pragma once

#include <cstdint>
#include <fstream>
#include <string>

namespace BitStream {
using character_type = char;

class ibitstream {
   public:
    ibitstream() = default;
    ibitstream(const std::string& filename)
        : input(filename, std::ios::binary), shifts(0) {}
    ~ibitstream() = default;
    void open(const std::string& filename) {
        input.open(filename, std::ios::binary);
        shifts = 0;
    }
    void close() { input.close(); }
    ibitstream(const ibitstream&) = delete;
    ibitstream(ibitstream&&) = default;
    ibitstream& operator>>(bool& bit);
    bool read();
    character_type read_unit();
    operator bool() const { return (bool)input; }
    bool operator!() const { return !input; }
    void align();

   private:
    void refill();

   private:
    std::basic_ifstream<character_type> input;
    std::uint8_t unit;
    std::uint8_t shifts;
};

class obitstream {
   public:
    obitstream() = default;
    obitstream(const std::string& filename)
        : output(filename, std::ios::binary), shifts(8) {}
    ~obitstream() { close(); }
    void open(const std::string& filename) {
        output.open(filename, std::ios::binary);
        shifts = 8;
    }
    void close();
    obitstream(const obitstream&) = delete;
    obitstream(obitstream&&) = default;
    obitstream& operator<<(bool bit);
    obitstream& write(bool bit);
    obitstream& write_unit(std::uint8_t unit);
    void align();

   private:
    void flush();

   private:
    std::basic_ofstream<character_type> output;
    std::uint8_t unit;
    std::uint8_t shifts;
};
}  // namespace BitStream
