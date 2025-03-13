#ifndef DECRYPTION_HPP
#define DECRYPTION_HPP

#include <unordered_map>
#include <string>
#include <fstream>
#include <iostream>

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
        string decodedText, currentCode;
        for (char bit : encodedText) {
            currentCode += bit;
            if (huffmanToWord.find(currentCode) != huffmanToWord.end()) {
                decodedText += huffmanToWord[currentCode] + " ";
                currentCode.clear();
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

        string encodedText = "", word;
        while (inFile >> word) {
            encodedText += word;
        }
        inFile.close();

        string decryptedText = decode(encodedText);
        outFile << decryptedText;
        outFile.close();
        cout << "Decryption completed. Output saved to " << outputFile << endl;
    }
};

#endif