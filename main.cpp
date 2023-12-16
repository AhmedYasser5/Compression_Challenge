#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "huffman.hpp"

using namespace std;

unordered_map<char, uint64_t> generate_count_table(istream& buffer) {
    unordered_map<char, uint64_t> count;
    char letter;
    while (buffer.get(letter)) {
        ++count[letter];
    }
    ++count[EOF];
    return count;
}

void compress(const char* filename) {
    ifstream input(filename);
    if (!input) {
        throw ios::failure("No such file to compress!");
    }
    auto count_table = generate_count_table(input);
    auto tree = Huffman::generate_mapping(count_table);
    auto encode = Huffman::generate_inverse_mapping(tree);
    obitstream output(filename + ".huf"s);
    Huffman::serialize_tree(output, tree);

    input.clear();
    input.seekg(ios::beg);
    Huffman::serialize_text(input, output, encode);
}

void decompress(const char* filename) {
    ibitstream input(filename);
    if (!input) {
        throw ios::failure("No such file to decompress!");
    }
    auto tree = Huffman::deserialize_tree<char>(input);
    ofstream output(filename + ".fuh"s);
    Huffman::deserialize_text(input, output, tree);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        throw invalid_argument("Usage: " + string(argv[0]) +
                               " [c|d] <filename>");
    }
    if (argv[1][0] == 'c') {
        compress(argv[2]);
    } else if (argv[1][0] == 'd') {
        decompress(argv[2]);
    } else {
        throw invalid_argument("First argument should be 'c' or 'd'");
    }
    return 0;
}
