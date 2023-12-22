#include "huffman.hpp"

static void check_invalid_file(BitStream::ibitstream& input) {
    if (input) {
        return;
    }
    throw std::ios::failure("Not a huf-compressed file!");
}

static void generate_inverse_mapping(
    const Huffman::HuffmanTree& tree,
    std::unordered_map<Huffman::character_type, std::vector<bool>>& table,
    std::vector<bool>& encoding) {
    if (tree.left == nullptr) {
        table.insert_or_assign(tree.letter, encoding);
        return;
    }
    encoding.push_back(0);
    generate_inverse_mapping(*tree.left, table, encoding);
    encoding.back() = 1;
    generate_inverse_mapping(*tree.right, table, encoding);
    encoding.pop_back();
}

std::unordered_map<Huffman::character_type, std::vector<bool>>
Huffman::generate_inverse_mapping(const HuffmanTree& tree) {
    std::unordered_map<character_type, std::vector<bool>> table;
    std::vector<bool> encoding;
    ::generate_inverse_mapping(tree, table, encoding);
    return table;
}

void Huffman::serialize_tree(BitStream::obitstream& output,
                             const HuffmanTree& tree) {
    if (tree.left == nullptr) {
        output.write(1);
        output.write_unit(static_cast<uint8_t>(tree.letter));
        return;
    }
    output.write(0);
    serialize_tree(output, *tree.left);
    serialize_tree(output, *tree.right);
}

static void serialize_letter(
    BitStream::obitstream& output,
    const std::unordered_map<Huffman::character_type, std::vector<bool>>&
        encode,
    Huffman::character_type letter) {
    for (auto bit : encode.at(letter)) {
        output.write(bit);
    }
}

void Huffman::serialize_text(
    std::basic_istream<character_type>& input, BitStream::obitstream& output,
    const std::unordered_map<character_type, std::vector<bool>>& encode) {
    for (character_type letter; input.get(letter);) {
        ::serialize_letter(output, encode, letter);
    }
    ::serialize_letter(output, encode, EOF);
}

Huffman::HuffmanTree Huffman::deserialize_tree(BitStream::ibitstream& input) {
    bool bit;
    input >> bit;
    ::check_invalid_file(input);
    HuffmanTree node;
    if (bit) {
        node.letter = input.read_unit();
        ::check_invalid_file(input);
    } else {
        node.left = std::make_unique<HuffmanTree>(deserialize_tree(input));
        node.right = std::make_unique<HuffmanTree>(deserialize_tree(input));
    }
    return node;
}

void Huffman::deserialize_text(BitStream::ibitstream& input,
                               std::basic_ostream<character_type>& output,
                               const HuffmanTree& decode) {
    while (true) {
        HuffmanTree const* node = &decode;
        bool bit;
        while (node->left != nullptr && (input >> bit)) {
            node = bit ? node->right.get() : node->left.get();
        }
        if (node->left != nullptr) {
            ::check_invalid_file(input);
        }
        if (node->letter == static_cast<character_type>(EOF)) {
            break;
        }
        output << node->letter;
    }
}
