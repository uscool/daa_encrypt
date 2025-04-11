#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include "avl_tree.hpp"
#include "huffman.hpp"
#include "decrypt.hpp"
#include "rsa.hpp"

using namespace std;
const int SHIFT = 4;

// Add this at the top with other global variables
unordered_map<string, string> globalHuffmanCodes;
RSA rsa; // Global RSA instance

// Stack implementation using vector
class Stack {
private:
    vector<string> data;

public:
    void push(const string& item) {
        data.push_back(item);
    }

    string pop() {
        if (data.empty()) {
            throw runtime_error("Stack is empty");
        }
        string item = data.back();
        data.pop_back();
        return item;
    }

    bool isEmpty() const {
        return data.empty();
    }

    size_t size() const {
        return data.size();
    }

    void clear() {
        data.clear();
    }
};

// Global stack for file operations
Stack fileStack;

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

    srand(10010292); // Seed for random dummy word generation

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

    // Read all words into a vector to preserve order
    vector<string> words;
    string word;
    while (inFile >> word) {
        words.push_back(word);
    }
    inFile.close();

    // Process words in order
    bool firstWord = true;
    string encodedText;
    for (const string& word : words) {
        if (!firstWord) {
            encodedText += " ";  // Add space between words
            outFile << " ";
        }
        if (huffmanCodes.find(word) != huffmanCodes.end()) {
            encodedText += huffmanCodes[word];
            outFile << huffmanCodes[word];
        } else {
            encodedText += word;  // Keep original if no code found
            outFile << word;
        }
        firstWord = false;
    }

    outFile.close();
}

void huffmanCaesarEncryptFile(const string& filename) {
    ifstream ogFile(filename);
    if (!ogFile.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    // Push original filename to stack
    fileStack.push(filename);

    // Salting
    insertSaltedWords(filename, "saltedFile.txt");
    fileStack.push("saltedFile.txt");

    ogFile.close();
    ifstream inputFile("saltedFile.txt");

    unordered_map<string, int> wordFrequency;
    string line, word;

    while (getline(inputFile, line)) {
        istringstream stream(line);
        while (stream >> word) {
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

    // Store Huffman codes globally
    globalHuffmanCodes = huffman.getCodes();

    // Caesar cipher 
    unordered_map<string, string> encryptedWordFrequency;

    for (const auto& pair : wordFrequency) {
        string encryptedKey = caesarEncrypt(pair.first, SHIFT);
        encryptedWordFrequency[encryptedKey] = pair.second;
    }

    cout << "Encrypted Word Frequency:" << endl;
    for (const auto &pair : encryptedWordFrequency) {
        cout << pair.first << ": " << pair.second << endl;
    }

    replaceWithHuffmanCodes("saltedFile.txt", "encodedFile.txt", globalHuffmanCodes);
    fileStack.push("encodedFile.txt");
    cout << "Encoded file created: encodedFile.txt" << endl;
}

void huffmanCaesarDecryptFile() {
    if (globalHuffmanCodes.empty()) {
        cout << "Error: No encryption has been performed yet. Please encrypt a file first." << endl;
        return;
    }

    if (fileStack.size() < 2) {
        cout << "Error: No files available for decryption." << endl;
        return;
    }

    // Get the encoded file from stack
    string encodedFile = fileStack.pop();
    string outputFile = "decryptedFile.txt";

    cout << "Starting Huffman decryption..." << endl;
    cout << "Input file: " << encodedFile << endl;

    Decryption d(globalHuffmanCodes);
    d.decryptFile(encodedFile, outputFile);
    fileStack.push(outputFile);

    // Verify the decrypted file exists and has content
    ifstream finalCheck(outputFile);
    string finalContent((istreambuf_iterator<char>(finalCheck)),
                       istreambuf_iterator<char>());
    finalCheck.close();

    cout << "Decrypted file length: " << finalContent.length() << endl;
    if (finalContent.empty()) {
        cout << "Warning: Decrypted file is empty!" << endl;
    } else {
        cout << "First 100 characters of decrypted content: " << finalContent.substr(0, 100) << endl;
    }

    cout << "File decrypted successfully. Output saved to: " << outputFile << endl;
}

void rsaEncryptFile(const string& filename) {
    ifstream inFile(filename);
    if (!inFile.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    string content((istreambuf_iterator<char>(inFile)),
                   istreambuf_iterator<char>());
    inFile.close();

    // Encrypt the content using RSA
    string encryptedContent = rsa.encryptString(content);

    // Save the encrypted content
    string outputFile = "rsa_encrypted.txt";
    ofstream outFile(outputFile);
    outFile << encryptedContent;
    outFile.close();

    fileStack.push(outputFile);
    cout << "RSA encrypted file created: " << outputFile << endl;
}

void rsaDecryptFile() {
    if (fileStack.isEmpty()) {
        cout << "Error: No files available for decryption." << endl;
        return;
    }

    string inputFile = fileStack.pop();
    ifstream inFile(inputFile);
    if (!inFile.is_open()) {
        cerr << "Error opening file: " << inputFile << endl;
        return;
    }

    string encryptedContent((istreambuf_iterator<char>(inFile)),
                          istreambuf_iterator<char>());
    inFile.close();

    // Decrypt the content using RSA
    string decryptedContent = rsa.decryptString(encryptedContent);

    // Save the decrypted content
    string outputFile = "rsa_decrypted.txt";
    ofstream outFile(outputFile);
    outFile << decryptedContent;
    outFile.close();

    fileStack.push(outputFile);
    cout << "RSA decrypted file created: " << outputFile << endl;
}

// Convert binary string to ASCII characters with word boundaries preserved
string binaryToAscii(const string& binary) {
    string ascii;
    stringstream ss(binary);
    string token;
    bool firstWord = true;

    // Debug: Print input binary
    cout << "\n=== Binary Input for ASCII Conversion ===" << endl;
    cout << binary << endl;

    while (ss >> token) {
        if (!firstWord) {
            ascii += " "; // Add space between words
        }

        // Convert each binary code to ASCII, preserving original length
        string asciiCode;
        for (size_t i = 0; i < token.length(); i += 7) {
            string byte = token.substr(i, min(7, (int)(token.length() - i)));
            if (byte.length() < 7) {
                byte = string(7 - byte.length(), '0') + byte;
            }
            char c = static_cast<char>(stoi(byte, nullptr, 2));
            asciiCode += c;
        }
        ascii += asciiCode;
        firstWord = false;
    }

    // Debug: Print ASCII output
    cout << "\n=== ASCII Output ===" << endl;
    cout << ascii << endl;

    return ascii;
}

// Convert ASCII characters to binary string with word boundaries preserved
string asciiToBinary(const string& ascii) {
    string binary;
    stringstream ss(ascii);
    string token;
    bool firstWord = true;

    // Debug: Print input ASCII
    cout << "\n=== ASCII Input for Binary Conversion ===" << endl;
    cout << ascii << endl;

    while (ss >> token) {
        if (!firstWord) {
            binary += " "; // Add space between words
        }

        // Convert each ASCII code back to binary, preserving original length
        string binaryCode;
        for (char c : token) {
            string byte = bitset<7>(c).to_string();
            // Remove leading zeros to get original length
            size_t firstOne = byte.find('1');
            if (firstOne != string::npos) {
                byte = byte.substr(firstOne);
            }
            binaryCode += byte;
        }
        binary += binaryCode;
        firstWord = false;
    }

    // Debug: Print binary output
    cout << "\n=== Binary Output ===" << endl;
    cout << binary << endl;

    return binary;
}

void combinedEncryptFile(const string& filename) {
    cout << "\n=== Starting Encryption Process ===" << endl;

    // Step 1: Read the input file
    ifstream inputFile(filename);
    if (!inputFile) {
        cerr << "Error opening input file!" << endl;
        return;
    }

    string content((istreambuf_iterator<char>(inputFile)),
                  istreambuf_iterator<char>());
    inputFile.close();

    // Step 2: Split content into words and apply RSA to each word
    vector<string> words;
    string currentWord;
    for (char c : content) {
        if (c == '+' && currentWord == "+") {
            currentWord += c;
            string encryptedWord = rsa.encryptString(currentWord);
            encryptedWord = "|" + encryptedWord + "|";
            words.push_back(encryptedWord);
            currentWord.clear();
        }
        else if (c == ',' || c == '.' || c == ';' || c == ':' || c == '!' || c == '?') {
            if (!currentWord.empty()) {
                string encryptedWord = rsa.encryptString(currentWord);
                encryptedWord = "|" + encryptedWord + "|";
                words.push_back(encryptedWord);
                currentWord.clear();
            }
            string punctuation(1, c);
            string encryptedPunctuation = rsa.encryptString(punctuation);
            encryptedPunctuation = "|" + encryptedPunctuation + "|";
            words.push_back(encryptedPunctuation);
        }
        else if (isalnum(c) || c == '\'') {
            currentWord += c;
        }
        else if (isspace(c)) {
            if (!currentWord.empty()) {
                string encryptedWord = rsa.encryptString(currentWord);
                encryptedWord = "|" + encryptedWord + "|";
                words.push_back(encryptedWord);
                currentWord.clear();
            }
            words.push_back(" ");
        }
        else {
            if (!currentWord.empty()) {
                string encryptedWord = rsa.encryptString(currentWord);
                encryptedWord = "|" + encryptedWord + "|";
                words.push_back(encryptedWord);
                currentWord.clear();
            }
            string specialChar(1, c);
            string encryptedChar = rsa.encryptString(specialChar);
            encryptedChar = "|" + encryptedChar + "|";
            words.push_back(encryptedChar);
        }
    }

    if (!currentWord.empty()) {
        string encryptedWord = rsa.encryptString(currentWord);
        encryptedWord = "|" + encryptedWord + "|";
        words.push_back(encryptedWord);
    }

    // Step 3: Build frequency map
    unordered_map<string, int> frequencyMap;
    for (const string& word : words) {
        if (word != " ") {
            frequencyMap[word]++;
        }
    }

    // Step 4: Generate Huffman codes
    AVLTree avlTree;
    for (const auto& pair : frequencyMap) {
        avlTree.insert(pair.first, pair.second);
    }

    HuffmanCoding huffman;
    huffman.buildFromAVL(avlTree);
    globalHuffmanCodes = huffman.getCodes();

    // Step 5: Write RSA encrypted words to file
    ofstream rsaFile("rsa_encoded.txt");
    for (const string& word : words) {
        if (word == " ") {
            rsaFile << " ";
        } else {
            rsaFile << word;
        }
    }
    rsaFile.close();
    fileStack.push("rsa_encoded.txt");

    // Step 6: Apply Huffman encoding
    replaceWithHuffmanCodes("rsa_encoded.txt", "huffman_encoded.txt", globalHuffmanCodes);
    fileStack.push("huffman_encoded.txt");

    // Step 7: Apply Caesar cipher
    ifstream huffmanFile("huffman_encoded.txt");
    string huffmanContent((istreambuf_iterator<char>(huffmanFile)),
                         istreambuf_iterator<char>());
    huffmanFile.close();

    string caesarEncrypted;
    for (char c : huffmanContent) {
        if (isdigit(c)) {
            int digit = c - '0';
            digit = (digit + 4) % 10;
            caesarEncrypted += to_string(digit);
        } else {
            caesarEncrypted += c;
        }
    }

    ofstream finalFile("combined_encrypted.txt");
    finalFile << caesarEncrypted;
    finalFile.close();
    fileStack.push("combined_encrypted.txt");
    cout << "Stored in encrypted file named 'combined_encrypted.txt'" << endl;
}

void removeSaltedWords(const string& inputFile, const string& outputFile) {
    ifstream inFile(inputFile);
    ofstream outFile(outputFile);

    if (!inFile || !outFile) {
        cerr << "Error opening file!" << endl;
        return;
    }

    string word;
    int count = 0;

    while (inFile >> word) {
        count++;
        if (count % 5 != 0) {
            outFile << word << " ";
        }
    }

    inFile.close();
    outFile.close();
}

void combinedDecryptFile() {
    cout << "\n=== Starting Decryption Process ===" << endl;

    // Step 1: Reverse Caesar cipher
    ifstream encryptedFile("combined_encrypted.txt");
    if (!encryptedFile) {
        cerr << "Error opening encrypted file!" << endl;
        return;
    }

    string encryptedContent((istreambuf_iterator<char>(encryptedFile)),
                          istreambuf_iterator<char>());
    encryptedFile.close();

    string caesarDecrypted;
    for (char c : encryptedContent) {
        if (isdigit(c)) {
            int digit = c - '0';
            digit = (digit - 4 + 10) % 10;
            caesarDecrypted += to_string(digit);
        } else {
            caesarDecrypted += c;
        }
    }

    ofstream caesarFile("caesar_decrypted.txt");
    caesarFile << caesarDecrypted;
    caesarFile.close();
    fileStack.push("caesar_decrypted.txt");

    // Step 2: Apply Huffman decoding
    unordered_map<string, string> codeToWord;
    for (const auto& pair : globalHuffmanCodes) {
        codeToWord[pair.second] = pair.first;
    }

    vector<string> binaryCodes;
    string currentCode;
    bool inToken = false;
    string currentToken;

    for (char c : caesarDecrypted) {
        if (c == '|') {
            if (inToken) {
                if (!currentToken.empty()) {
                    binaryCodes.push_back(currentToken);
                    currentToken.clear();
                }
                inToken = false;
            } else {
                inToken = true;
            }
        } else if (c == ' ' && !inToken) {
            if (!currentCode.empty()) {
                binaryCodes.push_back(currentCode);
                currentCode.clear();
            }
        } else if (inToken) {
            currentToken += c;
        } else {
            currentCode += c;
        }
    }
    if (!currentCode.empty()) {
        binaryCodes.push_back(currentCode);
    }
    if (!currentToken.empty()) {
        binaryCodes.push_back(currentToken);
    }

    ofstream huffmanFile("huffman_decrypted.txt");
    for (const string& code : binaryCodes) {
        if (codeToWord.find(code) != codeToWord.end()) {
            string rsaWord = codeToWord[code];
            huffmanFile << rsaWord << " ";
        } else {
            huffmanFile << "|" << code << "| ";
        }
    }
    huffmanFile.close();
    fileStack.push("huffman_decrypted.txt");

    // Step 3: Apply RSA decryption
    ifstream huffmanDecryptedFile("huffman_decrypted.txt");
    string huffmanContent((istreambuf_iterator<char>(huffmanDecryptedFile)),
                         istreambuf_iterator<char>());
    huffmanDecryptedFile.close();

    vector<string> rsaWords;
    string currentRsaWord;
    inToken = false;

    for (char c : huffmanContent) {
        if (c == '|') {
            if (inToken) {
                if (!currentRsaWord.empty()) {
                    rsaWords.push_back(currentRsaWord);
                    currentRsaWord.clear();
                }
                inToken = false;
            } else {
                inToken = true;
            }
        } else if (c == ' ' && !inToken) {
            if (!currentRsaWord.empty()) {
                rsaWords.push_back(currentRsaWord);
                currentRsaWord.clear();
            }
            rsaWords.push_back(" ");
        } else if (inToken) {
            currentRsaWord += c;
        }
    }
    if (!currentRsaWord.empty()) {
        rsaWords.push_back(currentRsaWord);
    }

    ofstream rsaFile("rsa_decrypted.txt");
    for (const string& word : rsaWords) {
        if (word == " ") {
            rsaFile << " ";
        } else {
            string decryptedWord = rsa.decryptString(word);
            rsaFile << decryptedWord;
        }
    }
    rsaFile.close();
    fileStack.push("rsa_decrypted.txt");

    // Step 4: Remove salt
    removeSaltedWords("rsa_decrypted.txt", "final_decrypted.txt");
    fileStack.push("final_decrypted.txt");
    cout << "Stored in decrypted file named 'final_decrypted.txt'" << endl;
}

void displayMenu() {
    cout << "\n=== File Encryption/Decryption Menu ===" << endl;
    cout << "1. Encrypt File" << endl;
    cout << "2. Decrypt File" << endl;
    cout << "3. Exit" << endl;
    cout << "Enter your choice (1-3): ";
}

void displayEncryptionOptions() {
    cout << "\n=== Encryption Options ===" << endl;
    cout << "1. Combined Encryption (Huffman + RSA + Caesar)" << endl;
    cout << "2. Huffman + Caesar Encryption" << endl;
    cout << "Enter your choice (1-2): ";
}

void displayDecryptionOptions() {
    cout << "\n=== Decryption Options ===" << endl;
    cout << "1. Combined Decryption (Caesar + RSA + Huffman)" << endl;
    cout << "2. Caesar + Huffman Decryption" << endl;
    cout << "Enter your choice (1-2): ";
}

int main() {
    string filename;
    int choice, subChoice;

    while (true) {
        displayMenu();
        cin >> choice;

        switch (choice) {
            case 1: // Encrypt
                cout << "Enter filename to encrypt: ";
                cin >> filename;
                displayEncryptionOptions();
                cin >> subChoice;

                switch (subChoice) {
                    case 1:
                        combinedEncryptFile(filename);
                        break;
                    case 2:
                        huffmanCaesarEncryptFile(filename);
                        break;
                    default:
                        cout << "Invalid choice!" << endl;
                }
                break;

            case 2: // Decrypt
                displayDecryptionOptions();
                cin >> subChoice;

                switch (subChoice) {
                    case 1:
                        combinedDecryptFile();
                        break;
                    case 2:
                        huffmanCaesarDecryptFile();
                        break;
                    default:
                        cout << "Invalid choice!" << endl;
                }
                break;

            case 3: // Exit
                cout << "Exiting program..." << endl;
                return 0;

            default:
                cout << "Invalid choice! Please try again." << endl;
        }
    }

    return 0;
}
