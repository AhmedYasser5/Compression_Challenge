#pragma once

#include <cstdint>
#include <fstream>
#include <string>

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
    std::uint8_t read_byte();
    operator bool() const { return (bool)input; }
    bool operator!() const { return !input; }
    void align();

   private:
    void refill();

   private:
    std::ifstream input;
    std::uint8_t byte;
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
    obitstream& write_byte(std::uint8_t byte);
    void align();

   private:
    void flush();

   private:
    std::ofstream output;
    std::uint8_t byte;
    std::uint8_t shifts;
};
