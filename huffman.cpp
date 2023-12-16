#include "huffman.hpp"

void Huffman::detail::check_invalid_file(ibitstream& input) {
    if (input) {
        return;
    }
    throw std::ios_base::failure("Not a huf-compressed file!");
}
