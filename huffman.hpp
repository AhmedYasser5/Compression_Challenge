#pragma once

#include <algorithm>
#include <compare>
#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <stack>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "bitstream.hpp"

namespace Huffman {

template <typename T>
    requires(sizeof(char) == sizeof(T))
struct HuffmanTree {
    std::unique_ptr<HuffmanTree> left;
    std::unique_ptr<HuffmanTree> right;
    T letter;

    HuffmanTree() : left(nullptr), right(nullptr) {}
    HuffmanTree(HuffmanTree&& left, HuffmanTree&& right)
        : left(std::make_unique<HuffmanTree>(std::move(left))),
          right(std::make_unique<HuffmanTree>(std::move(right))) {}
    HuffmanTree(std::unique_ptr<HuffmanTree> left,
                std::unique_ptr<HuffmanTree> right)
        : left(left.release()), right(right.release()) {}
    HuffmanTree(T letter) : left(nullptr), right(nullptr), letter(letter) {}
    HuffmanTree(T letter, HuffmanTree&& left, HuffmanTree&& right)
        : left(std::make_unique(std::move(left))),
          right(std::make_unique(std::move(right))),
          letter(letter) {}
    HuffmanTree(T letter, std::unique_ptr<HuffmanTree> left,
                std::unique_ptr<HuffmanTree> right)
        : left(left.release()), right(right.release()), letter(letter) {}

    ~HuffmanTree() = default;

    HuffmanTree(const HuffmanTree&) = delete;
    HuffmanTree(HuffmanTree&&) = default;

    HuffmanTree& operator=(const HuffmanTree&) = delete;
    HuffmanTree& operator=(HuffmanTree&&) = default;

    std::weak_ordering operator<=>(const HuffmanTree<T>& other) const {
        return letter <=> other.letter;
    }
};

template <typename T, typename U>
    requires std::is_integral_v<U> && std::is_unsigned_v<U>
HuffmanTree<T> generate_mapping(std::unordered_map<T, U>&& count_table) {
    using Node = std::pair<U, HuffmanTree<T>>;
    std::vector<Node> trees;
    trees.reserve(count_table.size());
    for (auto&& [letter, count] : count_table) {
        trees.emplace_back(count, HuffmanTree(letter));
    }
    std::make_heap(trees.begin(), trees.end(), std::greater<Node>());
    while (trees.size() > 1) {
        std::pop_heap(trees.begin(), trees.end(), std::greater<Node>());
        auto smallest = std::move(trees.back());
        trees.pop_back();
        std::pop_heap(trees.begin(), trees.end(), std::greater<Node>());
        auto larger = std::move(trees.back());
        trees.pop_back();
        // Tree will be heavier (has more nodes) to the right
        trees.emplace_back(
            smallest.first + larger.first,
            HuffmanTree(std::move(smallest.second), std::move(larger.second)));
        std::push_heap(trees.begin(), trees.end(), std::greater<Node>());
    }
    return std::move(trees.back().second);
}

template <typename T>
void generate_inverse_mapping(const HuffmanTree<T>& tree,
                              std::unordered_map<T, std::vector<bool>>& table,
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

template <typename T>
std::unordered_map<T, std::vector<bool>> generate_inverse_mapping(
    const HuffmanTree<T>& tree) {
    std::unordered_map<T, std::vector<bool>> table;
    std::vector<bool> encoding;
    generate_inverse_mapping(tree, table, encoding);
    return table;
}

// TODO: works with any character encoding (not just ASCII)
template <typename T>
void serialize_tree(obitstream& output, const HuffmanTree<T>& tree) {
    if (tree.left == nullptr) {
        output.write(1);
        output.write_byte(static_cast<uint8_t>(tree.letter));
        return;
    }
    output.write(0);
    serialize_tree(output, *tree.left);
    serialize_tree(output, *tree.right);
}

template <typename T>
void serialize_text(std::istream& input, obitstream& output,
                    const std::unordered_map<T, std::vector<bool>>& encode) {
    for (T letter; input.get(letter);) {
        for (auto bit : encode.at(letter)) {
            output.write(bit);
        }
    }
}

void check_invalid_file(ibitstream&);

// TODO: works with any character encoding (not just ASCII)
template <typename T>
HuffmanTree<T> deserialize_tree(ibitstream& input) {
    bool bit;
    input >> bit;
    check_invalid_file(input);
    HuffmanTree<T> node;
    if (bit) {
        node.letter = input.read_byte();
        check_invalid_file(input);
    } else {
        node.left =
            std::make_unique<HuffmanTree<T>>(deserialize_tree<T>(input));
        node.right =
            std::make_unique<HuffmanTree<T>>(deserialize_tree<T>(input));
    }
    return node;
}

template <typename T>
void deserialize_text(ibitstream& input, std::ostream& output,
                      const HuffmanTree<T>& decode) {
    while (true) {
        HuffmanTree<T> const* node = &decode;
        bool bit;
        while (node->left != nullptr && (input >> bit)) {
            node = bit ? node->right.get() : node->left.get();
        }
        if (node->left != nullptr) {
            break;
        }
        output << node->letter;
    }
}

}  // namespace Huffman
