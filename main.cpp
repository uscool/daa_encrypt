#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include "avl_tree.hpp"
#include "huffman.hpp"
#include "random_generator.hpp"

using namespace std;

// Convert string to lowercase
string toLowerCase(const string &s) {
    string result = s;
    for (char &c : result) {
        c = tolower(c);
    }
    return result;
}

// Clean word by removing punctuation
string cleanWord(const string &word) {
    string result;
    for (char c : word) {
        if (isalnum(c)) {
            result += c;
        }
    }
    return result;
}

int main() {
    string filename;
    cout << "Enter the input file name: ";
    cin >> filename;

    ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return 1;
    }

    unordered_map<string, int> wordFrequency;
    string line, word;

    while (getline(inputFile, line)) {
        istringstream stream(line);
        while (stream >> word) {
            word = cleanWord(toLowerCase(word));
            if (!word.empty()) {
                wordFrequency[word]++;
            }
        }
    }
    inputFile.close();

    // Generate salted words using RandomGenerator
    unordered_map<string, int> saltedWords = processWords(filename);

    // Add salted words to the original word frequency map
    for (const auto &pair : saltedWords) {
        wordFrequency[pair.first] += pair.second;  // Increment the frequency of salted words
    }

    // Now process the words using AVL tree and Huffman coding as before
    AVLTree avlTree;
    for (const auto &pair : wordFrequency) {
        avlTree.insert(pair.first, pair.second);
    }

    cout << "\nWord frequencies in AVL tree (inorder traversal):\n";
    avlTree.printInorder();

    HuffmanCoding huffman;
    huffman.buildFromAVL(avlTree);

    cout << "\nHuffman codes for each word:\n";
    huffman.printCodes();

    return 0;
}
