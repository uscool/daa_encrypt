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

// Global instances
unordered_map<string, string> globalHuffmanCodes;
RSA globalRSA;  // Global RSA instance

class Stack
{
private:
    vector<string> data;

public:
    void push(const string &item)
    {
        data.push_back(item);
    }

    string pop()
    {
        if (data.empty())
        {
            throw runtime_error("Stack is empty");
        }
        string item = data.back();
        data.pop_back();
        return item;
    }

    bool isEmpty() const
    {
        return data.empty();
    }

    size_t size() const
    {
        return data.size();
    }

    void clear()
    {
        data.clear();
    }
};
Stack fileStack;

void displayMenu()
{
    cout << "\n=== File Encryption/Decryption Menu ===" << endl;
    cout << "1. Encrypt File" << endl;
    cout << "2. Decrypt File" << endl;
    cout << "3. Exit" << endl;
    cout << "Enter your choice (1-3): ";
}
void displayEncryptionOptions()
{
    cout << "\n=== Encryption Options ===" << endl;
    cout << "1. Combined Encryption (Huffman + RSA + Caesar)" << endl;
    cout << "2. Huffman + Caesar Encryption" << endl;
    cout << "Enter your choice (1-2): ";
}
void displayDecryptionOptions()
{
    cout << "\n=== Decryption Options ===" << endl;
    cout << "1. Combined Decryption (Caesar + RSA + Huffman)" << endl;
    cout << "2. Caesar + Huffman Decryption" << endl;
    cout << "Enter your choice (1-2): ";
}

void replaceWithHuffmanCodes(const string& inputFile, const string& outputFile, unordered_map<string, string>& huffmanCodes) {
    ifstream inFile(inputFile);
    ofstream outFile(outputFile);

    if (!inFile || !outFile) {
        cerr << "Error opening file!" << endl;
        return;
    }

    // Read the entire content
    string content((istreambuf_iterator<char>(inFile)),
                  istreambuf_iterator<char>());
    inFile.close();

    // Process content character by character
    bool firstWord = true;
    string currentToken;
    bool inBrackets = false;

    for (char c : content) {
        if (c == '[') {
            inBrackets = true;
            currentToken.clear();
        }
        else if (c == ']') {
            inBrackets = false;
            if (!currentToken.empty()) {
                if (!firstWord) {
                    outFile << " ";
                }
                // Remove any leading/trailing spaces from the token
                size_t start = currentToken.find_first_not_of(" ");
                size_t end = currentToken.find_last_not_of(" ");
                if (start != string::npos && end != string::npos) {
                    string trimmedToken = currentToken.substr(start, end - start + 1);
                    // Check if this token has a Huffman code
                    if (huffmanCodes.find(trimmedToken) != huffmanCodes.end()) {
                        outFile << huffmanCodes[trimmedToken];
                    } else {
                        outFile << trimmedToken;
                    }
                }
                firstWord = false;
            }
        }
        else if (inBrackets) {
            currentToken += c;
        }
        else if (c == ' ') {
            if (!firstWord) {
                outFile << " ";
            }
            outFile << " ";
            firstWord = false;
        }
    }

    outFile.close();
}

void printHuffmanCodes() {
    cout << "\n=== Current Huffman Codes Hashmap ===" << endl;
    cout << "Total number of codes: " << globalHuffmanCodes.size() << endl;
    cout << "----------------------------------------" << endl;

    if (globalHuffmanCodes.empty()) {
        cout << "Hashmap is empty!" << endl;
    } else {
        for (const auto& pair : globalHuffmanCodes) {
            cout << "Token: '" << pair.first << "' -> Code: " << pair.second << endl;
        }
    }
    cout << "========================================" << endl;
}

void saveHuffmanCodesToFile(const unordered_map<string, string>& codes, const string& filename = "huffman_hashmap.txt") {
    ofstream file(filename);
    if (!file) {
        cerr << "Error saving Huffman codes!" << endl;
        return;
    }

    for (const auto& pair : codes) {
        // Save in format: token:code
        file << pair.first << ":" << pair.second << endl;
    }
    file.close();
    cout << "Huffman codes saved to " << filename << endl;
}

void loadHuffmanCodesFromFile(unordered_map<string, string>& codes, const string& filename = "huffman_hashmap.txt") {
    ifstream file(filename);
    if (!file) {
        cerr << "Error: huffman_hashmap.txt not found. Please encrypt a file first." << endl;
        return;
    }

    codes.clear();
    string line;
    while (getline(file, line)) {
        size_t colon_pos = line.find(':');
        if (colon_pos != string::npos) {
            string token = line.substr(0, colon_pos);
            string code = line.substr(colon_pos + 1);
            codes[token] = code;
        }
    }
    file.close();
    cout << "Huffman codes loaded from " << filename << endl;
}

void combinedEncryptFile(const string &filename)
{
    cout << "\n=== Starting Encryption Process ===" << endl;

    // Initialize RSA keys if not already initialized
    globalRSA.initializeKeys();

    // Step 1: Read the input file
    ifstream inputFile(filename);
    if (!inputFile)
    {
        cerr << "Error opening input file!" << endl;
        return;
    }

    // Step 2: Split content into words and apply RSA to each word

    string curr_word;
    vector<string> words;
    while (inputFile >> curr_word)
    {
        words.push_back(globalRSA.encryptString(curr_word));
    }

    // Step 3: Build frequency map
    unordered_map<string, int> frequencyMap;
    for (const string &word : words)
    {
        frequencyMap[word]++; // Include spaces in frequency map
    }

    // Step 4: Generate Huffman codes
    AVLTree avlTree;
    for (const auto &pair : frequencyMap)
    {
        avlTree.insert(pair.first, pair.second);
    }

    HuffmanCoding huffman;
    huffman.buildFromAVL(avlTree);
    globalHuffmanCodes = huffman.getCodes();

    // Save Huffman codes to file
    saveHuffmanCodesToFile(globalHuffmanCodes);

    // Print all Huffman codes
    printHuffmanCodes();

    // Step 5: Write RSA encrypted words to file
    ofstream rsaFile("rsa_encoded.txt");
    for (size_t i = 0; i < words.size(); ++i)
    {
        rsaFile << "[";
        rsaFile << words[i];
        if (i != words.size() - 1)
        {
            rsaFile << " ";
        }
        rsaFile << "]";
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
    for (char c : huffmanContent)
    {
        if (isdigit(c))
        {
            int digit = c - '0';
            digit = (digit + 4) % 10;
            caesarEncrypted += to_string(digit);
        }
        else
        {
            caesarEncrypted += c;
        }
    }

    ofstream finalFile("combined_encrypted.txt");
    finalFile << caesarEncrypted;
    finalFile.close();
    fileStack.push("combined_encrypted.txt");
    cout << "Stored in encrypted file named 'combined_encrypted.txt'" << endl;
}

void huffmanCaesarEncryptFile(const string &filename) {
    cout << "\n=== Starting Huffman + Caesar Encryption Process ===" << endl;

    // Step 1: Read the input file
    ifstream inputFile(filename);
    if (!inputFile) {
        cerr << "Error opening input file!" << endl;
        return;
    }

    // Read the entire content
    string content((istreambuf_iterator<char>(inputFile)),
                  istreambuf_iterator<char>());
    inputFile.close();

    // Step 2: Build frequency map
    unordered_map<string, int> frequencyMap;
    stringstream ss(content);
    string word;
    while (ss >> word) {
        frequencyMap[word]++;
    }

    // Step 3: Generate Huffman codes
    AVLTree avlTree;
    for (const auto &pair : frequencyMap) {
        avlTree.insert(pair.first, pair.second);
    }

    HuffmanCoding huffman;
    huffman.buildFromAVL(avlTree);
    globalHuffmanCodes = huffman.getCodes();

    // Save Huffman codes to file
    saveHuffmanCodesToFile(globalHuffmanCodes);

    // Print all Huffman codes
    printHuffmanCodes();

    // Step 4: Apply Huffman encoding
    ofstream huffmanFile("huffman_encoded.txt");
    stringstream contentStream(content);
    string currentWord;
    bool firstWord = true;

    while (contentStream >> currentWord) {
        if (!firstWord) {
            huffmanFile << " ";
        }
        if (globalHuffmanCodes.find(currentWord) != globalHuffmanCodes.end()) {
            huffmanFile << globalHuffmanCodes[currentWord];
        } else {
            huffmanFile << currentWord;
        }
        firstWord = false;
    }
    huffmanFile.close();
    fileStack.push("huffman_encoded.txt");

    // Step 5: Apply Caesar cipher
    ifstream huffmanInput("huffman_encoded.txt");
    string huffmanContent((istreambuf_iterator<char>(huffmanInput)),
                         istreambuf_iterator<char>());
    huffmanInput.close();

    string caesarEncrypted;
    for (char c : huffmanContent) {
        if (isdigit(c)) {
            int digit = c - '0';
            digit = (digit + SHIFT) % 10;
            caesarEncrypted += to_string(digit);
        } else {
            caesarEncrypted += c;
        }
    }

    ofstream finalFile("huffman_caesar_encrypted.txt");
    finalFile << caesarEncrypted;
    finalFile.close();
    fileStack.push("huffman_caesar_encrypted.txt");

    cout << "Encryption complete. Output saved to 'huffman_caesar_encrypted.txt'" << endl;
}

void decryption_process(){
    cout << "\n=== Starting Decryption Process ===" << endl;

    // Load Huffman codes from file
    loadHuffmanCodesFromFile(globalHuffmanCodes);

    // Print loaded codes for verification
    cout << "\n=== Loaded Huffman Codes ===" << endl;
    for (const auto& pair : globalHuffmanCodes) {
        cout << "Token: '" << pair.first << "' -> Code: " << pair.second << endl;
    }
    cout << "===========================" << endl;

    // Create decryptor instance (it will use the global RSA instance)
    Decryptor decryptor;

    // Step 1: Reverse Caesar cipher
    cout << "\n=== Step 1: Reversing Caesar Cipher ===" << endl;
    decryptor.reverseCaesarToFile();

    // Step 2: Decode Huffman codes
    cout << "\n=== Step 2: Decoding Huffman Codes ===" << endl;
    decryptor.decodeHuffmanToFile(globalHuffmanCodes);

    // Step 3: Reverse RSA encryption
    cout << "\n=== Step 3: Reversing RSA Encryption ===" << endl;
    decryptor.reverseRSAToFile();

    cout << "\n=== Decryption Process Complete ===" << endl;
    cout << "Final decrypted output saved to: decrypted_output.txt" << endl;
}

void huffmanCaesarDecryptFile() {
    cout << "\n=== Starting Huffman + Caesar Decryption Process ===" << endl;

    // Load Huffman codes from file
    loadHuffmanCodesFromFile(globalHuffmanCodes);

    // Print loaded codes for verification
    cout << "\n=== Loaded Huffman Codes ===" << endl;
    for (const auto& pair : globalHuffmanCodes) {
        cout << "Token: '" << pair.first << "' -> Code: " << pair.second << endl;
    }
    cout << "===========================" << endl;

    // Step 1: Reverse Caesar cipher
    cout << "\n=== Step 1: Reversing Caesar Cipher ===" << endl;
    ifstream encryptedFile("huffman_caesar_encrypted.txt");
    if (!encryptedFile) {
        cerr << "Error: Encrypted file not found!" << endl;
        return;
    }

    string encryptedContent((istreambuf_iterator<char>(encryptedFile)),
                          istreambuf_iterator<char>());
    encryptedFile.close();

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

    ofstream caesarReversed("caesar_reversed.txt");
    caesarReversed << reversedCaesar;
    caesarReversed.close();
    fileStack.push("caesar_reversed.txt");

    // Step 2: Decode Huffman codes
    cout << "\n=== Step 2: Decoding Huffman Codes ===" << endl;
    ifstream huffmanFile("caesar_reversed.txt");
    string huffmanContent((istreambuf_iterator<char>(huffmanFile)),
                         istreambuf_iterator<char>());
    huffmanFile.close();

    // Create a reverse map for Huffman codes
    unordered_map<string, string> reverseHuffmanCodes;
    for (const auto& pair : globalHuffmanCodes) {
        reverseHuffmanCodes[pair.second] = pair.first;
    }

    // Decode the content
    stringstream decodedContent;
    string currentCode;
    for (char c : huffmanContent) {
        if (c == ' ') {
            if (reverseHuffmanCodes.find(currentCode) != reverseHuffmanCodes.end()) {
                decodedContent << reverseHuffmanCodes[currentCode] << " ";
            } else {
                decodedContent << currentCode << " ";
            }
            currentCode.clear();
        } else {
            currentCode += c;
        }
    }

    // Handle the last code
    if (!currentCode.empty()) {
        if (reverseHuffmanCodes.find(currentCode) != reverseHuffmanCodes.end()) {
            decodedContent << reverseHuffmanCodes[currentCode];
        } else {
            decodedContent << currentCode;
        }
    }

    ofstream finalOutput("huffman_caesar_decrypted.txt");
    finalOutput << decodedContent.str();
    finalOutput.close();

    cout << "\n=== Decryption Process Complete ===" << endl;
    cout << "Final decrypted output saved to: huffman_caesar_decrypted.txt" << endl;
}

int main()
{
    string filename;
    int choice, subChoice;

    while (true)
    {
        displayMenu();
        cin >> choice;

        switch (choice)
        {
        case 1: // Encrypt
            cout << "Enter filename to encrypt: ";
            cin >> filename;
            displayEncryptionOptions();
            cin >> subChoice;
            if (subChoice == 1) {
                combinedEncryptFile(filename);
            } else if (subChoice == 2) {
                huffmanCaesarEncryptFile(filename);
            }
            break;
        case 2: // Decrypt
            cout << "Decrypting encrypted file\n";
            displayDecryptionOptions();
            cin >> subChoice;
            if (subChoice == 1) {
                decryption_process();
            } else if (subChoice == 2) {
                huffmanCaesarDecryptFile();
            }
            break;
        case 3: // Exit
            cout << "Exiting program..." << endl;
            return 0;
        default:
            cout << "Invalid choice. Please try again." << endl;
        }
    }

    return 0;
}
