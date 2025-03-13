#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include "avl_tree.hpp"
#include "huffman.hpp"
#include "decrypt.hpp"

using namespace std;
const int SHIFT = 4;

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

string generateSalt() {
    const string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    string salt = "";
    for (int i = 0; i < ((rand() % 10) + 3); ++i) { 
        salt += chars[rand() % chars.length()];
    }
    return salt;
}

void insertSaltedWords(const string& inputFile, const string& outputFile) {
    ifstream inFile(inputFile);
    ofstream outFile(outputFile);

    if (!inFile || !outFile) {
        cerr << "Error opening file!" << endl;
        return;
    }

    string word;
    int count = 0;

    srand(time(0)); // Seed for random dummy word generation

    while (inFile >> word) {
        count++;
        outFile << word << " ";
        if (count % 5 == 0) {
            outFile << generateSalt() << " ";
        }
    }
    inFile.close();
    outFile.close();
}

string caesarEncrypt(const string& text, int shift) {
    string encryptedText;
    for (char ch : text) {
        if (ch >= 'a' && ch <= 'z') 
            encryptedText += 'a' + (ch - 'a' + shift) % 26;
        else if (ch >= 'A' && ch <= 'Z') 
            encryptedText += 'A' + (ch - 'A' + shift) % 26;
        else if (ch >= '0' && ch <= '9') 
            encryptedText += '0' + (ch - '0' + shift) % 10;
        else 
            encryptedText += ch; // Keep other characters unchanged
    }
    return encryptedText;
}

void replaceWithHuffmanCodes(const string& inputFile, const string& outputFile, unordered_map<string, string>& huffmanCodes) {
    ifstream inFile(inputFile);
    ofstream outFile(outputFile);

    if (!inFile || !outFile) {
        cerr << "Error opening file!" << endl;
        return;
    }

    string word;
    while (inFile >> word) { 
        // string cleanedWord = cleanWord(toLowerCase(word));
        if (huffmanCodes.find(word) != huffmanCodes.end()) {
            outFile << huffmanCodes[word] << " "; // Replace with Huffman code
        }
    }

    inFile.close();
    outFile.close();
}

int main() {
    string filename;
    cout << "Enter the input file name: ";
    cin >> filename;

    ifstream ogFile(filename);
    if (!ogFile.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return 1;
    }

    // Salting
    insertSaltedWords(filename, "saltedFile.txt");
    
    ogFile.close();
    ifstream inputFile("saltedFile.txt");

    unordered_map<string, int> wordFrequency;
    string line, word;

    while (getline(inputFile, line)) {
        istringstream stream(line);
        while (stream >> word) {
            // word = cleanWord(toLowerCase(word));
            if (!word.empty()) {
                wordFrequency[word]++;
            }
        }
    }
    inputFile.close();

    AVLTree avlTree;
    for (const auto &pair : wordFrequency) {
        avlTree.insert(pair.first, pair.second);
    }

    HuffmanCoding huffman;
    huffman.buildFromAVL(avlTree);

    cout << "\nHuffman codes for each word:\n";
    huffman.printCodes();

    // Caesar cipher 
    unordered_map<string, string> encryptedWordFrequency;

    for (const auto& pair : wordFrequency) {
        string encryptedKey = caesarEncrypt(pair.first, SHIFT); // Encrypt word
        encryptedWordFrequency[encryptedKey] = pair.second; // Maintain frequency
    }

    cout << "Encrypted Word Frequency:" << endl;
    for (const auto &pair : encryptedWordFrequency) {
        cout << pair.first << ": " << pair.second << endl;
    }

    unordered_map<string, string> huffmanCodes = huffman.getCodes();
    replaceWithHuffmanCodes("saltedFile.txt", "encodedFile.txt", huffmanCodes);
    cout << "Encoded file created: encodedFile.txt" << endl;

    Decryption d(huffman.getCodes());
    d.decryptFile("encodedFile.txt", "decryptedFile.txt");

    return 0;
}


