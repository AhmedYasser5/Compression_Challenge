#include "bitstream.hpp"

using namespace std;

BitStream::ibitstream& BitStream::ibitstream::operator>>(bool& bit) {
    bit = read();
    return *this;
}

void BitStream::ibitstream::refill() {
    if (shifts != 0) {
        return;
    }
    input.read(reinterpret_cast<character_type*>(&unit), 1);
    shifts = 8;
}

bool BitStream::ibitstream::read() {
    refill();
    --shifts;
    bool ret = unit >> 7;
    unit <<= 1;
    return ret;
}

BitStream::character_type BitStream::ibitstream::read_unit() {
    refill();
    auto ret_byte = unit;
    const auto remaining = 8 - shifts;
    shifts = 0;
    refill();
    ret_byte |= unit >> (8 - remaining);
    unit <<= remaining;
    shifts -= remaining;
    return ret_byte;
}

void BitStream::ibitstream::align() {
    if (shifts == 8) {
        return;
    }
    shifts = 0;
}

void BitStream::obitstream::flush() {
    if (shifts != 0) {
        return;
    }
    output.write(reinterpret_cast<character_type*>(&unit), 1);
    shifts = 8;
}

void BitStream::obitstream::close() {
    align();
    flush();
    output.close();
}

BitStream::obitstream& BitStream::obitstream::operator<<(bool bit) {
    return write(bit);
}

BitStream::obitstream& BitStream::obitstream::write(bool bit) {
    flush();
    --shifts;
    unit <<= 1;
    unit |= bit;
    return *this;
}

BitStream::obitstream& BitStream::obitstream::write_unit(std::uint8_t unit) {
    flush();
    const auto remaining = 8 - shifts;
    this->unit <<= shifts;
    this->unit |= unit >> remaining;
    shifts = 0;
    flush();
    this->unit = unit;
    shifts -= remaining;
    return *this;
}

void BitStream::obitstream::align() {
    if (shifts == 8) {
        return;
    }
    unit <<= shifts;
    shifts = 0;
}
