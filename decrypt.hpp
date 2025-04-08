#ifndef DECRYPTION_HPP
#define DECRYPTION_HPP

#include <unordered_map>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>

using namespace std;

class Decryption {
private:
    unordered_map<string, string> huffmanToWord;

public:
    Decryption(const unordered_map<string, string>& huffmanCodes) {
        for (const auto& pair : huffmanCodes) {
            huffmanToWord[pair.second] = pair.first;
        }
    }

    string decode(const string& encodedText) {
        string decodedText;
        bool firstWord = true;
        
        // Split the encoded text by spaces to preserve word order
        stringstream ss(encodedText);
        string token;
        
        while (ss >> token) {
            // Process each token (Huffman code) separately
            if (huffmanToWord.find(token) != huffmanToWord.end()) {
                if (!firstWord) {
                    decodedText += " ";
                }
                decodedText += huffmanToWord[token];
                firstWord = false;
            } else {
                // If the token is not found in the Huffman codes, try to split it
                // This handles cases where spaces are missing between codes
                string currentCode;
                for (char bit : token) {
                    currentCode += bit;
                    if (huffmanToWord.find(currentCode) != huffmanToWord.end()) {
                        if (!firstWord) {
                            decodedText += " ";
                        }
                        decodedText += huffmanToWord[currentCode];
                        firstWord = false;
                        currentCode.clear();
                    }
                }
                
                // If there's a remaining code, try to match it
                if (!currentCode.empty()) {
                    // Try to find a matching code by adding bits from the next token
                    string nextToken;
                    if (ss >> nextToken) {
                        for (char bit : nextToken) {
                            currentCode += bit;
                            if (huffmanToWord.find(currentCode) != huffmanToWord.end()) {
                                if (!firstWord) {
                                    decodedText += " ";
                                }
                                decodedText += huffmanToWord[currentCode];
                                firstWord = false;
                                currentCode.clear();
                                break;
                            }
                        }
                    }
                }
            }
        }
        
        return decodedText;
    }

    void decryptFile(const string& encodedFile, const string& outputFile) {
        ifstream inFile(encodedFile);
        ofstream outFile(outputFile);
        if (!inFile || !outFile) {
            cerr << "Error opening files!" << endl;
            return;
        }

        // Read all words into a vector to preserve order
        vector<string> words;
        string word;
        while (inFile >> word) {
            words.push_back(word);
        }
        inFile.close();

        // Process words, skipping salt values
        string encodedText = "";
        for (size_t i = 0; i < words.size(); i++) {
            // Skip every 5th word (salt)
            if ((i + 1) % 6 != 0) {
                encodedText += words[i];
                if (i < words.size() - 1 && (i + 2) % 5 != 0) {
                    encodedText += " "; // Add space between non-salt words
                }
            }
        }

        // Debug output
        cout << "Encoded text before decoding: " << encodedText << endl;

        // Print each word separately for debugging
        cout << "Words before decoding: ";
        stringstream ss(encodedText);
        string token;
        while (ss >> token) {
            cout << token << " ";
        }
        cout << endl;

        string decryptedText = decode(encodedText);
        outFile << decryptedText;
        outFile.close();
        cout << "Decryption completed. Output saved to " << outputFile << endl;
    }
};

#endif
