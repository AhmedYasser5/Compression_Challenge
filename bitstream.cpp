#include "bitstream.hpp"

using namespace std;

ibitstream& ibitstream::operator>>(bool& bit) {
    bit = read();
    return *this;
}

void ibitstream::refill() {
    if (shifts != 0) {
        return;
    }
    input.read(reinterpret_cast<char*>(&byte), 1);
    shifts = 8;
}

bool ibitstream::read() {
    refill();
    --shifts;
    bool ret = byte >> 7;
    byte <<= 1;
    return ret;
}

uint8_t ibitstream::read_byte() {
    refill();
    uint8_t ret_byte = byte;
    const auto remaining = 8 - shifts;
    shifts = 0;
    refill();
    ret_byte |= byte >> (8 - remaining);
    byte <<= remaining;
    shifts -= remaining;
    return ret_byte;
}

void ibitstream::align() {
    if (shifts == 8) {
        return;
    }
    shifts = 0;
}

void obitstream::flush() {
    if (shifts != 0) {
        return;
    }
    output.write(reinterpret_cast<const char*>(&byte), 1);
    shifts = 8;
}

void obitstream::close() {
    align();
    flush();
    output.close();
}

obitstream& obitstream::operator<<(bool bit) { return write(bit); }

obitstream& obitstream::write(bool bit) {
    flush();
    --shifts;
    byte <<= 1;
    byte |= bit;
    return *this;
}

obitstream& obitstream::write_byte(uint8_t byte) {
    flush();
    const auto remaining = 8 - shifts;
    this->byte <<= shifts;
    this->byte |= byte >> remaining;
    shifts = 0;
    flush();
    this->byte = byte;
    shifts -= remaining;
    return *this;
}

void obitstream::align() {
    if (shifts == 8) {
        return;
    }
    byte <<= shifts;
    shifts = 0;
}
