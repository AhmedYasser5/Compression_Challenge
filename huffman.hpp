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

using character_type = BitStream::character_type;

struct HuffmanTree {
    std::unique_ptr<HuffmanTree> left;
    std::unique_ptr<HuffmanTree> right;
    character_type letter;

    HuffmanTree() : left(nullptr), right(nullptr) {}
    HuffmanTree(character_type letter)
        : left(nullptr), right(nullptr), letter(letter) {}

    HuffmanTree(HuffmanTree&& left, HuffmanTree&& right)
        : left(std::make_unique<HuffmanTree>(std::move(left))),
          right(std::make_unique<HuffmanTree>(std::move(right))) {}
    HuffmanTree(std::unique_ptr<HuffmanTree> left,
                std::unique_ptr<HuffmanTree> right)
        : left(left.release()), right(right.release()) {}

    HuffmanTree(character_type letter, HuffmanTree&& left, HuffmanTree&& right)
        : left(std::make_unique<HuffmanTree>(std::move(left))),
          right(std::make_unique<HuffmanTree>(std::move(right))),
          letter(letter) {}
    HuffmanTree(character_type letter, std::unique_ptr<HuffmanTree> left,
                std::unique_ptr<HuffmanTree> right)
        : left(left.release()), right(right.release()), letter(letter) {}

    ~HuffmanTree() = default;

    HuffmanTree(const HuffmanTree&) = delete;
    HuffmanTree(HuffmanTree&&) = default;

    HuffmanTree& operator=(const HuffmanTree&) = delete;
    HuffmanTree& operator=(HuffmanTree&&) = default;

    std::weak_ordering operator<=>(const HuffmanTree& other) const {
        return letter <=> other.letter;
    }
};

template <typename U>
    requires std::is_integral_v<U> && std::is_unsigned_v<U>
HuffmanTree generate_mapping(
    const std::unordered_map<character_type, U>& count_table) {
    using Node = std::pair<U, HuffmanTree>;
    std::vector<Node> trees;
    trees.reserve(count_table.size());
    for (auto& [letter, count] : count_table) {
        trees.emplace_back(count, HuffmanTree(letter));
    }
    std::make_heap(trees.begin(), trees.end(), std::greater<Node>());

    while (trees.size() > 1) {
        std::pop_heap(trees.begin(), trees.end(), std::greater<Node>());
        auto smallest = std::move(trees.back());
        trees.pop_back();

        std::pop_heap(trees.begin(), trees.end(), std::greater<Node>());
        auto larger = std::move(trees.back());

        // Tree will be heavier (has more nodes) to the right
        trees.back() = std::make_pair(
            smallest.first + larger.first,
            HuffmanTree(std::move(smallest.second), std::move(larger.second)));
        std::push_heap(trees.begin(), trees.end(), std::greater<Node>());
    }
    return std::move(trees.back().second);
}

std::unordered_map<Huffman::character_type, std::vector<bool>>
generate_inverse_mapping(const HuffmanTree& tree);

void serialize_tree(BitStream::obitstream& output, const HuffmanTree& tree);

void serialize_text(
    std::basic_istream<character_type>& input, BitStream::obitstream& output,
    const std::unordered_map<character_type, std::vector<bool>>& encode);

HuffmanTree deserialize_tree(BitStream::ibitstream& input);

void deserialize_text(BitStream::ibitstream& input,
                      std::basic_ostream<character_type>& output,
                      const HuffmanTree& decode);
}  // namespace Huffman
