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
        // Debug: Print Huffman codes
        cout << "\n=== Decryption: Huffman Codes ===" << endl;
        for (const auto& pair : huffmanCodes) {
            cout << "Word: '" << pair.first << "' -> Code: " << pair.second << endl;
            huffmanToWord[pair.second] = pair.first;
        }
    }

    string decode(const string& encodedText) {
        string decodedText;
        bool firstWord = true;

        // Debug: Print input encoded text
        cout << "\n=== Decryption: Input Encoded Text ===" << endl;
        cout << encodedText << endl;

        // Split the encoded text by spaces to preserve word order
        stringstream ss(encodedText);
        string token;

        while (ss >> token) {
            // Debug: Print current token
            cout << "Processing token: " << token << endl;

            // Try to find the token in Huffman codes
            auto it = huffmanToWord.find(token);
            if (it != huffmanToWord.end()) {
                if (!firstWord) {
                    decodedText += " ";
                }
                decodedText += it->second;
                firstWord = false;

                // Debug: Print decoded word
                cout << "Decoded to: " << it->second << endl;
            } else {
                // Try to find a partial match
                bool found = false;
                for (const auto& pair : huffmanToWord) {
                    if (token.find(pair.first) == 0) {
                        if (!firstWord) {
                            decodedText += " ";
                        }
                        decodedText += pair.second;
                        firstWord = false;
                        found = true;

                        // Debug: Print decoded word
                        cout << "Partially matched: " << pair.first << " -> " << pair.second << endl;
                        break;
                    }
                }
                if (!found) {
                    cout << "Warning: Token not found in Huffman codes: " << token << endl;
                }
            }
        }

        // Debug: Print final decoded text
        cout << "\n=== Decryption: Final Decoded Text ===" << endl;
        cout << decodedText << endl;

        return decodedText;
    }

    void decryptFile(const string& encodedFile, const string& outputFile) {
        ifstream inFile(encodedFile);
        ofstream outFile(outputFile);
        if (!inFile || !outFile) {
            cerr << "Error opening files!" << endl;
            return;
        }

        // Read the entire file content
        string encodedText((istreambuf_iterator<char>(inFile)),
                          istreambuf_iterator<char>());
        inFile.close();

        // Debug: Print file content
        cout << "\n=== Decryption: File Content ===" << endl;
        cout << encodedText << endl;

        // Process the content
        string decodedText = decode(encodedText);
        outFile << decodedText << " ";
        outFile.close();
        cout << "Decryption completed. Output saved to " << outputFile << endl;
    }
};

#endif
