#ifndef HUFFMAN_HPP
#define HUFFMAN_HPP

#include <queue>
#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include "avl_tree.hpp"

using namespace std;

struct HuffmanNode {
    string word;
    int frequency;
    HuffmanNode *left, *right;

    HuffmanNode(const string &w, int freq) : word(w), frequency(freq), left(nullptr), right(nullptr) {}
};

struct CompareNodes {
    bool operator()(HuffmanNode *a, HuffmanNode *b) {
        return a->frequency > b->frequency;
    }
};

class HuffmanCoding {
private:
    HuffmanNode *root;
    unordered_map<string, string> huffmanCodes;

    void generateCodes(HuffmanNode *node, const string &code) {
        if (!node) return;
        if (!node->word.empty()) {
            huffmanCodes[node->word] = code;
        }
        generateCodes(node->left, code + "0");
        generateCodes(node->right, code + "1");
    }

    void cleanup(HuffmanNode *node) {
        if (node) {
            cleanup(node->left);
            cleanup(node->right);
            delete node;
        }
    }

    void collectNodes(AVLNode *avlNode, vector<HuffmanNode *> &nodes) {
        if (!avlNode) return;
        collectNodes(avlNode->left, nodes);
        nodes.push_back(new HuffmanNode(avlNode->word, avlNode->count));
        collectNodes(avlNode->right, nodes);
    }

public:
    HuffmanCoding() : root(nullptr) {}

    ~HuffmanCoding() {
        cleanup(root);
    }

    void buildFromAVL(AVLTree &avlTree) {
        vector<HuffmanNode *> nodes;
        collectNodes(avlTree.getRoot(), nodes);

        priority_queue<HuffmanNode *, vector<HuffmanNode *>, CompareNodes> pq;
        for (auto node : nodes) {
            pq.push(node);
        }

        while (pq.size() > 1) {
            HuffmanNode *left = pq.top(); pq.pop();
            HuffmanNode *right = pq.top(); pq.pop();
            HuffmanNode *internal = new HuffmanNode("", left->frequency + right->frequency);
            internal->left = left;
            internal->right = right;
            pq.push(internal);
        }

        root = pq.top();
        generateCodes(root, "");
    }

    const unordered_map<string, string> &getCodes() {
        return huffmanCodes;
    }

    void printCodes() const {
        for (const auto &pair : huffmanCodes) {
            cout << pair.first << ": " << pair.second << endl;
        }
    }

    void saveHuffmanCodes(const unordered_map<string, string>& codes, const string& filePath) {
        ofstream out(filePath);
        if (!out) {
            cerr << "Failed to save Huffman codes to file!\n";
            return;
        }
        for (const auto& pair : codes) {
            out << pair.first << " " << pair.second << "\n";
        }
        out.close();
    }

    unordered_map<string, string> loadHuffmanCodes(const string &filename) {
        unordered_map<string, string> codes;
        ifstream file(filename);
        if (!file) return codes;
    
        string word, code;
        while (file >> word >> code) {
            codes[word] = code;
        }
        for (auto& pair : codes) {
            cout << pair.first << ": " << pair.second << endl;
        }
        cout << endl;
        return codes;
    }
};

#endif