#ifndef DECRYPT_HPP
#define DECRYPT_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include "rsa.hpp"
#include "huffman.hpp"
#include "avl_tree.hpp"

using namespace std;

// Forward declarations
extern RSA globalRSA;
extern unordered_map<string, string> globalHuffmanCodes;

class Decryptor {
private:
    const int SHIFT = 4;  // Same shift as in encryption

    // Reverse Caesar cipher for digits
    string reverseCaesar(const string& text) {
        cout << "\n=== Step 1: Reversing Caesar Cipher ===" << endl;
        cout << "Input: " << text << endl;
        
        string result;
        for (char c : text) {
            if (isdigit(c)) {
                int digit = c - '0';
                digit = (digit - SHIFT + 10) % 10;  // Reverse the shift
                result += to_string(digit);
            } else {
                result += c;
            }
        }
        
        cout << "Output: " << result << endl;
        cout << "=====================================" << endl;
        return result;
    }

    // Convert Huffman codes back to original tokens
    string decodeHuffman(const string& encoded, const unordered_map<string, string>& huffmanCodes) {
        cout << "\n=== Step 2: Decoding Huffman Codes ===" << endl;
        cout << "Input: " << encoded << endl;
        
        // Create reverse mapping (code -> token)
        unordered_map<string, string> reverseCodes;
        for (const auto& pair : huffmanCodes) {
            reverseCodes[pair.second] = pair.first;
        }

        string result;
        string currentCode;
        
        for (char c : encoded) {
            currentCode += c;
            if (reverseCodes.find(currentCode) != reverseCodes.end()) {
                result += reverseCodes[currentCode];
                cout << "Decoded '" << currentCode << "' to '" << reverseCodes[currentCode] << "'" << endl;
                currentCode.clear();
            }
        }
        
        cout << "Final Output: " << result << endl;
        cout << "=====================================" << endl;
        return result;
    }

public:
    Decryptor() = default;

    // Method to reverse Caesar cipher and save to file
    void reverseCaesarToFile(const string& inputFile = "combined_encrypted.txt", 
                           const string& outputFile = "reverse_caesar.txt") {
        cout << "\n=== Reversing Caesar Cipher ===" << endl;
        cout << "Reading from: " << inputFile << endl;
        
        // Read the encrypted file
        ifstream encryptedFile(inputFile);
        if (!encryptedFile.is_open()) {
            throw runtime_error("Failed to open encrypted file: " + inputFile);
        }

        string encryptedContent((istreambuf_iterator<char>(encryptedFile)),
                              istreambuf_iterator<char>());
        encryptedFile.close();

        cout << "Original Content: " << encryptedContent << endl;

        // Reverse the Caesar cipher
        string reversedContent = reverseCaesar(encryptedContent);

        // Write to output file
        ofstream output(outputFile);
        if (!output.is_open()) {
            throw runtime_error("Failed to create output file: " + outputFile);
        }

        output << reversedContent;
        output.close();

        cout << "Caesar cipher reversed and saved to: " << outputFile << endl;
        cout << "=====================================" << endl;
    }

    // Decrypt a file and return the decrypted content
    string decryptFile(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            throw runtime_error("Failed to open file: " + filename);
        }

        stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        return globalRSA.decryptString(buffer.str());
    }

    // Decrypt a string directly
    string decryptString(const string& encrypted) {
        return globalRSA.decryptString(encrypted);
    }

    // Get the private key for decryption
    pair<long long, long long> getPrivateKey() const {
        return globalRSA.getPrivateKey();
    }

    // Combined decryption process (Caesar + RSA + Huffman)
    string combinedDecryptFile(const string& filename, const unordered_map<string, string>& huffmanCodes) {
        cout << "\n=== Starting Decryption Process ===" << endl;
        
        // Step 1: Read the encrypted file
        ifstream encryptedFile(filename);
        if (!encryptedFile.is_open()) {
            throw runtime_error("Failed to open encrypted file: " + filename);
        }

        string encryptedContent((istreambuf_iterator<char>(encryptedFile)),
                              istreambuf_iterator<char>());
        encryptedFile.close();

        cout << "Original Encrypted Content: " << encryptedContent << endl;

        // Step 2: Reverse Caesar cipher
        string afterCaesar = reverseCaesar(encryptedContent);

        // Step 3: Decode Huffman codes
        string afterHuffman = decodeHuffman(afterCaesar, huffmanCodes);

        // Step 4: Split into RSA-encrypted words and decrypt each
        cout << "\n=== Step 3: RSA Decryption ===" << endl;
        stringstream ss(afterHuffman);
        string word;
        string result;
        bool firstWord = true;

        while (ss >> word) {
            if (!firstWord) {
                result += " ";
            }
            // Remove brackets if present
            if (word.front() == '[') word = word.substr(1);
            if (word.back() == ']') word.pop_back();
            
            string decryptedWord = globalRSA.decryptString(word);
            cout << "Decrypted '" << word << "' to '" << decryptedWord << "'" << endl;
            result += decryptedWord;
            firstWord = false;
        }

        cout << "\nFinal Decrypted Text: " << result << endl;
        cout << "=====================================" << endl;
        return result;
    }

    // Method to decode Huffman codes and save to file
    void decodeHuffmanToFile(const unordered_map<string, string>& huffmanCodes,
                           const string& inputFile = "reverse_caesar.txt", 
                           const string& outputFile = "reverse_huffman.txt") {
        cout << "\n=== Decoding Huffman Codes ===" << endl;
        
        // Create and print reverse mapping (code -> token)
        cout << "\n=== Huffman Codes Reverse Mapping ===" << endl;
        unordered_map<string, string> reverseCodes;
        for (const auto& pair : huffmanCodes) {
            reverseCodes[pair.second] = pair.first;  // code -> token
            cout << "Code: '" << pair.second << "' -> Token: '" << pair.first << "'" << endl;
        }
        cout << "===================================" << endl;
        
        cout << "Reading from: " << inputFile << endl;
        
        // Read the input file
        ifstream input(inputFile);
        if (!input.is_open()) {
            throw runtime_error("Failed to open input file: " + inputFile);
        }

        // Write to output file
        ofstream output(outputFile);
        if (!output.is_open()) {
            throw runtime_error("Failed to create output file: " + outputFile);
        }

        string word;
        while (input >> word) {
            // Look up the word in reverseCodes (word is a code)
            if (reverseCodes.find(word) != reverseCodes.end()) {
                output << "[" << reverseCodes[word] << "]";
            } else {
                output << "[" << word << "]";  // If no match found, keep original
            }
            output << " ";  // Add space between tokens
        }

        input.close();
        output.close();

        cout << "Huffman codes decoded and saved to: " << outputFile << endl;
        cout << "=====================================" << endl;
    }

    // Method to reverse RSA encryption and save to file
    void reverseRSAToFile(const string& inputFile = "reverse_huffman.txt", 
                         const string& outputFile = "decrypted_output.txt") {
        cout << "\n=== Reversing RSA Encryption ===" << endl;
        cout << "Reading from: " << inputFile << endl;
        
        // Read the input file
        ifstream input(inputFile);
        if (!input.is_open()) {
            throw runtime_error("Failed to open input file: " + inputFile);
        }

        // Write to output file
        ofstream output(outputFile);
        if (!output.is_open()) {
            throw runtime_error("Failed to create output file: " + outputFile);
        }

        string content((istreambuf_iterator<char>(input)),
                      istreambuf_iterator<char>());
        input.close();

        bool firstWord = true;
        string currentEncrypted;
        bool inBrackets = false;

        for (char c : content) {
            if (c == '[') {
                inBrackets = true;
                currentEncrypted.clear();
            } else if (c == ']') {
                inBrackets = false;
                if (!currentEncrypted.empty()) {
                    cout << "Processing encrypted content: " << currentEncrypted << endl;
                    
                    // Decrypt the entire content as a single word using global RSA
                    string decryptedWord = globalRSA.decryptString(currentEncrypted);
                    cout << "Decrypted to: " << decryptedWord << endl;
                    
                    // Add space between words (except before first word)
                    if (!firstWord) {
                        output << " ";
                    }
                    output << decryptedWord;
                    firstWord = false;
                }
            } else if (inBrackets) {
                currentEncrypted += c;
            }
        }

        output.close();

        cout << "RSA encryption reversed and saved to: " << outputFile << endl;
        cout << "=====================================" << endl;
    }

    void huffmanCaesarDecryptToFile(const unordered_map<string, string>& huffmanCodes) {
        cout << "\n=== Starting Huffman + Caesar Decryption ===" << endl;
        
        // Step 1: Reverse Caesar cipher
        ifstream encryptedFile("combined_encrypted.txt");
        if (!encryptedFile) {
            cerr << "Error opening encrypted file!" << endl;
            return;
        }

        string encryptedContent((istreambuf_iterator<char>(encryptedFile)),
                              istreambuf_iterator<char>());
        encryptedFile.close();
        cout << "Read encrypted content: " << encryptedContent << endl;

        string reversedCaesar;
        for (char c : encryptedContent) {
            if (isdigit(c)) {
                int digit = c - '0';
                digit = (digit - SHIFT + 10) % 10;  // Reverse the shift
                reversedCaesar += to_string(digit);
            } else {
                reversedCaesar += c;
            }
        }
        cout << "Reversed Caesar cipher: " << reversedCaesar << endl;

        ofstream caesarReversedFile("reverse_caesar.txt");
        caesarReversedFile << reversedCaesar;
        caesarReversedFile.close();
        cout << "Caesar cipher reversed and saved to 'reverse_caesar.txt'" << endl;

        // Step 2: Decode Huffman codes
        ifstream huffmanFile("reverse_caesar.txt");
        ofstream decodedFile("decrypted_output.txt");
        
        if (!huffmanFile || !decodedFile) {
            cerr << "Error opening files for Huffman decoding!" << endl;
            return;
        }

        cout << "Huffman codes available: " << huffmanCodes.size() << endl;
        for (const auto& pair : huffmanCodes) {
            cout << "Token: '" << pair.first << "' -> Code: " << pair.second << endl;
        }

        string line;
        while (getline(huffmanFile, line)) {
            cout << "Processing line: " << line << endl;
            string currentCode;
            for (char c : line) {
                if (c == ' ') {
                    // Try to decode the current code
                    bool found = false;
                    for (const auto& pair : huffmanCodes) {
                        if (pair.second == currentCode) {
                            decodedFile << pair.first << " ";
                            cout << "Decoded '" << currentCode << "' to '" << pair.first << "'" << endl;
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        decodedFile << currentCode << " ";
                        cout << "Could not decode '" << currentCode << "', writing as is" << endl;
                    }
                    currentCode.clear();
                } else {
                    currentCode += c;
                }
            }
            // Handle the last code in the line
            if (!currentCode.empty()) {
                bool found = false;
                for (const auto& pair : huffmanCodes) {
                    if (pair.second == currentCode) {
                        decodedFile << pair.first;
                        cout << "Decoded '" << currentCode << "' to '" << pair.first << "'" << endl;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    decodedFile << currentCode;
                    cout << "Could not decode '" << currentCode << "', writing as is" << endl;
                }
            }
            decodedFile << endl;
        }

        huffmanFile.close();
        decodedFile.close();
        cout << "Huffman codes decoded and saved to 'decrypted_output.txt'" << endl;
    }
};

#endif // DECRYPT_HPP 