// RateTree.h  -- MODULE 3: Search & Trees
// Owner: Brijesh Gaur
//
// Topics covered:
//   DSA : Binary Search Tree, AVL self-balancing tree (rotations), recursion,
//         in-order traversal, range search, Huffman coding (priority queue +
//         binary tree), pointers
//   OOP : class templates, operator overloading (<, ==, <<), constructors,
//         destructors, exception handling
//   STL : vector, map, priority_queue, unordered_map
#ifndef VELORENT_RATETREE_H
#define VELORENT_RATETREE_H

#include <algorithm>
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include "Exceptions.h"

// ---------------------------------------------------------------------------
// One entry of the price index: (daily rate, VIN). The AVL tree sorts these.
// Operators are overloaded so the generic tree can compare them with  <
// ---------------------------------------------------------------------------
struct RateEntry {
    double rate;
    std::string vin;

    bool operator<(const RateEntry& o) const {
        return rate < o.rate || (rate == o.rate && vin < o.vin);
    }
    bool operator>(const RateEntry& o) const { return o < *this; }
    bool operator==(const RateEntry& o) const { return rate == o.rate && vin == o.vin; }
};

inline std::ostream& operator<<(std::ostream& os, const RateEntry& e) {
    return os << "Rs." << e.rate << " (" << e.vin << ")";
}

// ---------------------------------------------------------------------------
// AVL tree (class template): a BST that rebalances itself with rotations so
// its height stays O(log n). Insert / search / range query = O(log n).
// Works for ANY type T that has operator<.
// ---------------------------------------------------------------------------
template <typename T>
class AVLTree {
    struct Node {
        T key;
        Node* left;
        Node* right;
        int height;
        explicit Node(const T& k) : key(k), left(nullptr), right(nullptr), height(1) {}
    };

    Node* root_ = nullptr;
    std::size_t size_ = 0;

    static int h(const Node* n) { return n ? n->height : 0; }
    static void update(Node* n) { n->height = 1 + std::max(h(n->left), h(n->right)); }
    static int balanceFactor(const Node* n) { return n ? h(n->left) - h(n->right) : 0; }

    // Right rotation (left rotation is the mirror image):
    //   before: y has left child x;  x has subtrees A (left) and B (right); y has right subtree C
    //   after : x becomes the root;  x.left = A;  x.right = y;  y.left = B;  y.right = C
    static Node* rotateRight(Node* y) {
        Node* x = y->left;
        y->left = x->right;
        x->right = y;
        update(y);
        update(x);
        return x;
    }

    static Node* rotateLeft(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        y->left = x;
        update(x);
        update(y);
        return y;
    }

    Node* insert(Node* n, const T& key, bool& inserted) {
        if (!n) {
            inserted = true;
            return new Node(key);
        }
        if (key < n->key)
            n->left = insert(n->left, key, inserted);
        else if (n->key < key)
            n->right = insert(n->right, key, inserted);
        else
            return n;                                   // duplicate: ignore

        update(n);
        int bf = balanceFactor(n);
        if (bf > 1 && key < n->left->key) return rotateRight(n);          // Left-Left
        if (bf < -1 && n->right->key < key) return rotateLeft(n);         // Right-Right
        if (bf > 1 && n->left->key < key) {                               // Left-Right
            n->left = rotateLeft(n->left);
            return rotateRight(n);
        }
        if (bf < -1 && key < n->right->key) {                             // Right-Left
            n->right = rotateRight(n->right);
            return rotateLeft(n);
        }
        return n;
    }

    static void inorder(const Node* n, std::vector<T>& out) {
        if (!n) return;
        inorder(n->left, out);
        out.push_back(n->key);
        inorder(n->right, out);
    }

    // Collects every key k with lo <= k <= hi, skipping subtrees that cannot match.
    static void range(const Node* n, const T& lo, const T& hi, std::vector<T>& out) {
        if (!n) return;
        if (lo < n->key) range(n->left, lo, hi, out);
        if (!(n->key < lo) && !(hi < n->key)) out.push_back(n->key);
        if (n->key < hi) range(n->right, lo, hi, out);
    }

    static void destroy(Node* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

    static void printTree(const Node* n, int depth, std::ostream& os) {
        if (!n) return;
        printTree(n->right, depth + 1, os);
        os << std::string(static_cast<std::size_t>(depth) * 6, ' ') << n->key << "\n";
        printTree(n->left, depth + 1, os);
    }

public:
    AVLTree() = default;
    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;
    ~AVLTree() { destroy(root_); }

    bool insert(const T& key) {
        bool inserted = false;
        root_ = insert(root_, key, inserted);
        if (inserted) ++size_;
        return inserted;
    }

    bool contains(const T& key) const {
        const Node* n = root_;
        while (n) {
            if (key < n->key) n = n->left;
            else if (n->key < key) n = n->right;
            else return true;
        }
        return false;
    }

    const T& findMin() const {
        if (!root_) throw NotFoundException("Tree is empty");
        const Node* n = root_;
        while (n->left) n = n->left;
        return n->key;
    }

    const T& findMax() const {
        if (!root_) throw NotFoundException("Tree is empty");
        const Node* n = root_;
        while (n->right) n = n->right;
        return n->key;
    }

    std::vector<T> inorder() const {
        std::vector<T> out;
        inorder(root_, out);
        return out;
    }

    std::vector<T> rangeQuery(const T& lo, const T& hi) const {
        std::vector<T> out;
        range(root_, lo, hi, out);
        return out;
    }

    int height() const { return h(root_); }
    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    void print(std::ostream& os = std::cout) const { printTree(root_, 0, os); }
};

// Convenience: all vehicles with  minRate <= rate <= maxRate
inline std::vector<RateEntry> rangeByRate(const AVLTree<RateEntry>& tree, double minRate,
                                          double maxRate) {
    if (minRate > maxRate) throw InvalidInputException("minRate cannot exceed maxRate");
    // "\x7f" is bigger than any normal VIN, so it acts as +infinity for the VIN tie-break.
    return tree.rangeQuery(RateEntry{minRate, ""}, RateEntry{maxRate, std::string(1, '\x7f')});
}

// ---------------------------------------------------------------------------
// Huffman coding: frequent characters get SHORT bit codes, rare ones LONG
// codes, so receipts / logs take fewer bits. Built with a min-heap.
// (Here the "bit string" is kept as text of '0'/'1' to keep it easy to show.)
// ---------------------------------------------------------------------------
class HuffmanCoder {
    struct HNode {
    char ch;
    int freq;
    int order;
    HNode* left;
    HNode* right;

    HNode(char c, int f, int o, HNode* l, HNode* r)
        : ch(c), freq(f), order(o), left(l), right(r) {}
    };
    
    struct Cmp {                       // min-heap on frequency
        bool operator()(const HNode* a, const HNode* b) const {
            if (a->freq != b->freq) return a->freq > b->freq;
            return a->order > b->order;
        }
    };

    HNode* root_ = nullptr;
    std::unordered_map<char, std::string> codes_;

    static void destroy(HNode* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

    void assign(const HNode* n, const std::string& prefix) {
        if (!n) return;
        if (!n->left && !n->right) {                    // leaf = a real character
            codes_[n->ch] = prefix.empty() ? "0" : prefix;
            return;
        }
        assign(n->left, prefix + "0");
        assign(n->right, prefix + "1");
    }

public:
    HuffmanCoder() = default;
    HuffmanCoder(const HuffmanCoder&) = delete;
    HuffmanCoder& operator=(const HuffmanCoder&) = delete;
    ~HuffmanCoder() { destroy(root_); }

    void build(const std::string& text) {
        if (text.empty()) throw InvalidInputException("Cannot build Huffman tree for empty text");
        destroy(root_);
        root_ = nullptr;
        codes_.clear();

        std::map<char, int> freq;                       // ordered map -> deterministic
        for (char c : text) ++freq[c];

        std::priority_queue<HNode*, std::vector<HNode*>, Cmp> pq;
        int order = 0;
        for (const auto& entry : freq) {
            char ch = entry.first;
            int f = entry.second;
            pq.push(new HNode(ch, f, order++, nullptr, nullptr));
        }

        while (pq.size() > 1) {                         // merge two rarest nodes
            HNode* a = pq.top(); pq.pop();
            HNode* b = pq.top(); pq.pop();
            pq.push(new HNode{'\0', a->freq + b->freq, order++, a, b});
        }
        root_ = pq.top();
        assign(root_, "");
    }

    std::string encode(const std::string& text) const {
        if (!root_) throw InvalidInputException("Call build() first");
        std::string bits;
        for (char c : text) {
            auto it = codes_.find(c);
            if (it == codes_.end()) throw InvalidInputException("Character not in Huffman tree");
            bits += it->second;
        }
        return bits;
    }

    std::string decode(const std::string& bits) const {
        if (!root_) throw InvalidInputException("Call build() first");
        std::string out;
        if (!root_->left && !root_->right) {            // only one distinct character
            out.assign(bits.size(), root_->ch);
            return out;
        }
        const HNode* cur = root_;
        for (char b : bits) {
            if (b != '0' && b != '1') throw InvalidInputException("Bits must be 0 or 1");
            cur = (b == '0') ? cur->left : cur->right;
            if (!cur->left && !cur->right) {            // reached a leaf
                out += cur->ch;
                cur = root_;
            }
        }
        return out;
    }

    const std::unordered_map<char, std::string>& codes() const { return codes_; }

    void printCodes(std::ostream& os = std::cout) const {
        std::vector<std::pair<char, std::string>> v(codes_.begin(), codes_.end());
        std::sort(v.begin(), v.end(), [](const auto& a, const auto& b) {
            return a.second.size() != b.second.size() ? a.second.size() < b.second.size()
                                                      : a.first < b.first;
        });
        for (const auto& entry : v) {
            char c = entry.first;
            const std::string& code = entry.second;
            os << "  '" << (c == ' ' ? std::string("space") : (c == '\n' ? std::string("\\n") : std::string(1, c)))
               << "' -> " << code << "\n";
        }
    }

    // compressed bits / original bits (8 bits per char); smaller is better
    double compressionRatio(const std::string& text) const {
        return static_cast<double>(encode(text).size()) / (8.0 * static_cast<double>(text.size()));
    }
};

#endif
