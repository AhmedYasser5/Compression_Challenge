#include "huffman.hpp"

#include <gtest/gtest.h>

#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

using character_type = Huffman::character_type;
using count_type = std::size_t;
using HufTree = Huffman::HuffmanTree;

class HuffmanTesting : public testing::TestWithParam<string> {
   public:
    ~HuffmanTesting() override {}
};

template <typename T>
static string to_string(const vector<T>& e);

template <typename T>
static string to_string(const T& e);

template <typename T, typename U>
static string to_string(const pair<T, U>& e) {
    return '(' + to_string(e.first) + ", " + to_string(e.second) + ')';
}

template <typename T>
static string to_string(const vector<T>& e) {
    if (e.empty()) {
        return "[]";
    }
    string str = "[";
    for (const auto& o : e) {
        str += to_string(o);
        str += ", ";
    }
    str.pop_back();
    str.back() = ']';
    return str;
}

template <>
string to_string(const vector<bool>& e) {
    if (e.empty()) {
        return "";
    }
    string str;
    for (const auto& o : e) {
        str += o ? '1' : '0';
    }
    return str;
}

template <typename T>
static string to_string(const T& e) {
    if (e.empty()) {
        return "{}";
    }
    string str = "{";
    for (const auto& o : e) {
        str += to_string(o);
        str += ", ";
    }
    str.pop_back();
    str.back() = '}';
    return str;
}

template <>
string to_string(const character_type& e) {
    return "'" + string(1, e) + "'";
}

template <>
string to_string(const string& e) {
    return "\"" + e + "\"";
}

template <>
string to_string(const bool& e) {
    return e ? "true" : "false";
}

static unordered_map<character_type, count_type> get_count(
    const string& param) {
    unordered_map<character_type, count_type> count;
    for (auto letter : param) {
        ++count[letter];
    }
    ++count[EOF];
    return count;
}

#define ADD_TEST(name, ...) static void test_##name(__VA_ARGS__)

ADD_TEST(generate_mapping, const string& param, HufTree& tree) {
    const auto count_table = get_count(param);
    tree = Huffman::generate_mapping(count_table);
    map<count_type, unordered_set<character_type>, greater<count_type>>
        frequency_table;
    for (auto [letter, count] : count_table) {
        frequency_table[count].insert(letter);
    }

    queue<const HufTree*> bfs;
    bfs.push(&tree);
    while (!bfs.empty()) {
        unordered_multiset<character_type> to_erase;
        for (auto _ = bfs.size(); _ > 0; _--) {
            auto node = bfs.front();
            bfs.pop();
            if (node->left == nullptr) {
                to_erase.insert(node->letter);
                continue;
            }
            bfs.push(node->left.get());
            bfs.push(node->right.get());
        }
        while (!to_erase.empty()) {
            SCOPED_TRACE("Frequency table: " + to_string(frequency_table));
            SCOPED_TRACE("Will be erased: " + to_string(to_erase));
            bool erased = false;
            for (auto it = to_erase.begin(); it != to_erase.end();) {
                ASSERT_FALSE(frequency_table.empty());
                auto& nexts = frequency_table.begin()->second;
                auto next = nexts.find(*it);
                if (next == nexts.end()) {
                    ++it;
                    continue;
                }
                nexts.erase(next);
                if (nexts.empty()) {
                    frequency_table.erase(frequency_table.begin());
                }
                it = to_erase.erase(it);
                erased = true;
            }
            ASSERT_TRUE(erased);
        }
    }
    EXPECT_TRUE(frequency_table.empty());
}

ADD_TEST(generate_inverse_mapping, const HufTree& tree,
         unordered_map<character_type, vector<bool>>& imap) {
    imap = Huffman::generate_inverse_mapping(tree);

    queue<pair<const HufTree*, vector<bool>>> bfs;
    bfs.emplace(&tree, vector<bool>());
    while (!bfs.empty()) {
        auto [node, mapping] = std::move(bfs.front());
        bfs.pop();
        if (node->left == nullptr) {
            SCOPED_TRACE("Letter is: " + to_string(node->letter));
            auto it = imap.find(node->letter);
            ASSERT_NE(it, imap.end());
            EXPECT_EQ(mapping, it->second);
            continue;
        }
        mapping.emplace_back(0);
        bfs.emplace(node->left.get(), mapping);
        mapping.back() = 1;
        bfs.emplace(node->right.get(), std::move(mapping));
    }
}

ADD_TEST(tree_serialization, const HufTree& tree) {
    auto filename = "huffman.tree";
    {
        BitStream::obitstream output(filename);
        Huffman::serialize_tree(output, tree);
    }
    HufTree deserialized_tree;
    ASSERT_NO_THROW(deserialized_tree = ({
                        BitStream::ibitstream input(filename);
                        Huffman::deserialize_tree(input);
                    }));
    queue<pair<const HufTree*, const HufTree*>> bfs;
    bfs.emplace(&tree, &deserialized_tree);
    while (!bfs.empty()) {
        auto [node, deserialized_node] = bfs.front();
        bfs.pop();
        if (node->left == nullptr) {
            EXPECT_EQ(deserialized_node->left, nullptr);
            EXPECT_EQ(deserialized_node->right, nullptr);
            EXPECT_EQ(node->letter, deserialized_node->letter);
            continue;
        }
        EXPECT_NE(deserialized_node->left, nullptr);
        bfs.emplace(node->left.get(), deserialized_node->left.get());
        EXPECT_NE(deserialized_node->right, nullptr);
        bfs.emplace(node->right.get(), deserialized_node->right.get());
    }
}

ADD_TEST(text_serialization, const string& param,
         const unordered_map<character_type, vector<bool>>& encode,
         const HufTree& tree) {
    auto filename = "huffman.text";
    {
        basic_istringstream<character_type> input(param);
        BitStream::obitstream output(filename);
        Huffman::serialize_text(input, output, encode);
    }
    basic_ostringstream<character_type> output;
    {
        BitStream::ibitstream input(filename);
        ASSERT_NO_THROW(Huffman::deserialize_text(input, output, tree));
    }
    EXPECT_EQ(output.str(), param);
}

TEST_P(HuffmanTesting, integration_test) {
    HufTree tree;
    test_generate_mapping(GetParam(), tree);
    unordered_map<character_type, vector<bool>> encode;
    test_generate_inverse_mapping(tree, encode);
    test_tree_serialization(tree);
    test_text_serialization(GetParam(), encode, tree);
}

INSTANTIATE_TEST_SUITE_P(
    Huffman, HuffmanTesting,
    testing::Values(
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do "
        "eiusmod tempor incididunt ut labore et dolore magna aliqua. Aenean "
        "sed adipiscing diam donec adipiscing tristique risus nec. Bibendum "
        "neque egestas congue quisque. Purus faucibus ornare suspendisse sed "
        "nisi. Suspendisse potenti nullam ac tortor vitae purus faucibus "
        "ornare suspendisse. Ultricies leo integer malesuada nunc vel. "
        "Pellentesque elit eget gravida cum sociis. Adipiscing vitae proin "
        "sagittis nisl rhoncus. Lobortis feugiat vivamus at augue eget arcu "
        "dictum varius duis. Orci sagittis eu volutpat odio. Ac orci "
        "phasellus egestas tellus rutrum tellus pellentesque. Convallis "
        "convallis tellus id interdum velit laoreet id donec. Tempor nec "
        "feugiat nisl pretium fusce id velit.",
        "Proin fermentum leo vel orci porta. Diam maecenas sed enim ut sem "
        "viverra aliquet. Nulla aliquet porttitor lacus luctus accumsan. "
        "Placerat duis ultricies lacus sed turpis tincidunt. Arcu risus quis "
        "varius quam quisque. Bibendum ut tristique et egestas quis ipsum "
        "suspendisse. Senectus et netus et malesuada. Ultricies integer quis "
        "auctor elit sed vulputate mi sit. Feugiat scelerisque varius morbi "
        "enim nunc faucibus a pellentesque. Integer enim neque volutpat ac "
        "tincidunt. Nibh tortor id aliquet lectus proin. Sit amet mattis "
        "vulputate enim nulla. Amet nisl purus in mollis nunc. In metus "
        "vulputate eu scelerisque felis imperdiet proin fermentum.",
        "Egestas sed sed risus pretium quam. Sed risus ultricies tristique "
        "nulla aliquet enim. Vitae turpis massa sed elementum tempus "
        "egestas. Scelerisque mauris pellentesque pulvinar pellentesque "
        "habitant morbi. Adipiscing elit ut aliquam purus sit amet. Mauris a "
        "diam maecenas sed enim ut sem. Habitant morbi tristique senectus et "
        "netus et malesuada fames ac. Consequat id porta nibh venenatis cras "
        "sed felis eget. Eleifend donec pretium vulputate sapien. Est "
        "ultricies integer quis auctor elit sed vulputate. Adipiscing elit "
        "pellentesque habitant morbi tristique senectus. Interdum "
        "consectetur libero id faucibus. Sed felis eget velit aliquet. "
        "Pharetra massa massa ultricies mi quis hendrerit. Aliquet sagittis "
        "id consectetur purus ut faucibus pulvinar. Quam pellentesque nec "
        "nam aliquam sem. Sapien nec sagittis aliquam malesuada bibendum "
        "arcu vitae elementum. Elementum integer enim neque volutpat ac "
        "tincidunt vitae semper quis. Euismod in pellentesque massa placerat "
        "duis ultricies lacus sed turpis.",
        "Aliquet risus feugiat in ante metus dictum at tempor commodo. "
        "Volutpat maecenas volutpat blandit aliquam etiam erat. Interdum "
        "velit euismod in pellentesque massa. Metus vulputate eu scelerisque "
        "felis imperdiet proin fermentum. Enim eu turpis egestas pretium "
        "aenean pharetra. Vitae turpis massa sed elementum tempus egestas "
        "sed sed. A pellentesque sit amet porttitor eget dolor. Eu augue ut "
        "lectus arcu bibendum at varius vel. Justo nec ultrices dui sapien "
        "eget mi proin. Risus quis varius quam quisque id diam. Duis at "
        "consectetur lorem donec massa sapien. Eget magna fermentum iaculis "
        "eu non diam phasellus vestibulum lorem. Pretium nibh ipsum "
        "consequat nisl vel pretium lectus quam id.",
        "Purus viverra accumsan in nisl nisi scelerisque eu. Lacinia quis "
        "vel eros donec ac. Vitae tortor condimentum lacinia quis vel. "
        "Scelerisque eu ultrices vitae auctor eu augue ut. Nibh sed pulvinar "
        "proin gravida hendrerit. Pellentesque pulvinar pellentesque "
        "habitant morbi tristique senectus. Sit amet justo donec enim diam "
        "vulputate ut pharetra sit. Volutpat lacus laoreet non curabitur "
        "gravida arcu. Suspendisse in est ante in. Pellentesque eu tincidunt "
        "tortor aliquam nulla facilisi. Urna duis convallis convallis tellus "
        "id. Dignissim suspendisse in est ante in nibh. Ut sem viverra "
        "aliquet eget sit."));
