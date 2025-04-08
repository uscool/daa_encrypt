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

    cout << "\nHuffman codes for each word:\n";
    huffman.printCodes();

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

void combinedEncryptFile(const string& filename) {
    ifstream ogFile(filename);
    if (!ogFile.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    // Push original filename to stack
    fileStack.push(filename);

    // Step 1: Salting
    insertSaltedWords(filename, "saltedFile.txt");
    fileStack.push("saltedFile.txt");
    
    ogFile.close();
    ifstream inputFile("saltedFile.txt");

    // Step 2: Build frequency map and Huffman codes
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

    cout << "\nHuffman codes for each word:\n";
    huffman.printCodes();

    // Store Huffman codes globally
    globalHuffmanCodes = huffman.getCodes();

    // Step 3: Apply Huffman encoding
    replaceWithHuffmanCodes("saltedFile.txt", "huffman_encoded.txt", globalHuffmanCodes);
    fileStack.push("huffman_encoded.txt");
    cout << "Huffman encoded file created: huffman_encoded.txt" << endl;

    // Step 4: Apply RSA encryption
    ifstream huffmanFile("huffman_encoded.txt");
    string huffmanContent((istreambuf_iterator<char>(huffmanFile)),
                         istreambuf_iterator<char>());
    huffmanFile.close();

    // Use the new RSA method for Huffman codes
    string rsaEncrypted = rsa.encryptHuffmanCodes(huffmanContent);
    ofstream rsaFile("rsa_encoded.txt");
    rsaFile << rsaEncrypted;
    rsaFile.close();
    fileStack.push("rsa_encoded.txt");
    cout << "RSA encrypted file created: rsa_encoded.txt" << endl;

    // Step 5: Apply Caesar cipher
    ifstream rsaEncryptedFile("rsa_encoded.txt");
    string rsaContent((istreambuf_iterator<char>(rsaEncryptedFile)),
                     istreambuf_iterator<char>());
    rsaEncryptedFile.close();

    string caesarEncrypted = caesarEncrypt(rsaContent, SHIFT);
    ofstream finalFile("combined_encrypted.txt");
    finalFile << caesarEncrypted;
    finalFile.close();
    fileStack.push("combined_encrypted.txt");
    cout << "Combined encrypted file created: combined_encrypted.txt" << endl;
}

void combinedDecryptFile() {
    if (globalHuffmanCodes.empty()) {
        cout << "Error: No encryption has been performed yet. Please encrypt a file first." << endl;
        return;
    }

    if (fileStack.size() < 4) {
        cout << "Error: No files available for decryption." << endl;
        return;
    }

    // Step 1: Reverse Caesar cipher
    string encryptedFile = fileStack.pop();
    ifstream inFile(encryptedFile);
    string encryptedContent((istreambuf_iterator<char>(inFile)),
                          istreambuf_iterator<char>());
    inFile.close();

    cout << "Step 1: Caesar Decryption" << endl;
    cout << "Input length: " << encryptedContent.length() << endl;

    // Reverse Caesar cipher (shift by -SHIFT)
    string caesarDecrypted;
    for (char ch : encryptedContent) {
        if (ch >= 'a' && ch <= 'z') 
            caesarDecrypted += 'a' + (ch - 'a' - SHIFT + 26) % 26;
        else if (ch >= 'A' && ch <= 'Z') 
            caesarDecrypted += 'A' + (ch - 'A' - SHIFT + 26) % 26;
        else if (ch >= '0' && ch <= '9') 
            caesarDecrypted += '0' + (ch - '0' - SHIFT + 10) % 10;
        else 
            caesarDecrypted += ch;
    }

    cout << "Caesar decrypted length: " << caesarDecrypted.length() << endl;

    ofstream caesarFile("caesar_decrypted.txt");
    caesarFile << caesarDecrypted;
    caesarFile.close();
    fileStack.push("caesar_decrypted.txt");

    // Step 2: Reverse RSA
    cout << "\nStep 2: RSA Decryption" << endl;
    // Use the new RSA method for Huffman codes
    string rsaDecrypted = rsa.decryptHuffmanCodes(caesarDecrypted);
    cout << "RSA decrypted length: " << rsaDecrypted.length() << endl;

    // Write the RSA decrypted content to a file
    ofstream rsaFile("rsa_decrypted.txt");
    rsaFile << rsaDecrypted;
    rsaFile.close();
    fileStack.push("rsa_decrypted.txt");

    // Step 3: Reverse Huffman
    cout << "\nStep 3: Huffman Decryption" << endl;
    Decryption d(globalHuffmanCodes);
    d.decryptFile("rsa_decrypted.txt", "combined_decrypted.txt");
    fileStack.push("combined_decrypted.txt");
    
    // Verify the decrypted file exists and has content
    ifstream finalCheck("combined_decrypted.txt");
    string finalContent((istreambuf_iterator<char>(finalCheck)),
                       istreambuf_iterator<char>());
    finalCheck.close();
    
    cout << "Final decrypted file length: " << finalContent.length() << endl;
    if (finalContent.empty()) {
        cout << "Warning: Final decrypted file is empty!" << endl;
    } else {
        cout << "First 100 characters of decrypted content: " << finalContent.substr(0, 100) << endl;
    }
    
    cout << "Combined decryption completed. Output saved to: combined_decrypted.txt" << endl;
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
