#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "avl_tree.hpp"
#include "huffman.hpp"
#include "decrypt.hpp"

using namespace std;
const int SHIFT = 4;

// Convert string to lowercase
string toLowerCase(const string &s)
{
    string result = s;
    for (char c : s)
    {
        result += tolower(c);
    }
    return result;
}

// Clean word by removing punctuation
string cleanWord(const string &word)
{
    string result;
    for (char c : word)
    {
        if (isalnum(c))
            result += c;
    }
    return result;
}

// Generate dummy words (salt)
string generateSalt()
{
    const string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    string salt = "";
    for (int i = 0; i < ((rand() % 10) + 3); ++i)
    {
        salt += chars[rand() % chars.length()];
    }
    return salt;
}

// Insert real + salted words
void insertSaltedWords(const string &inputFile, const string &outputFile)
{
    ifstream inFile(inputFile);
    ofstream outFile(outputFile);

    if (!inFile || !outFile)
    {
        cerr << "Error opening file!" << endl;
        return;
    }

    string word;
    int count = 0;
    srand(time(0)); // Seed for random dummy word generation

    while (inFile >> word)
    {
        count++;
        outFile << word << " ";
        if (count % 5 == 0)
        {
            outFile << generateSalt() << " ";
        }
    }
    inFile.close();
    outFile.close();
}

// Caesar cipher (optional, not used in full encryption here)
string caesarEncrypt(const string &text, int shift)
{
    string encryptedText;
    for (char ch : text)
    {
        if (ch >= 'a' && ch <= 'z')
            encryptedText += 'a' + (ch - 'a' + shift) % 26;
        else if (ch >= 'A' && ch <= 'Z')
            encryptedText += 'A' + (ch - 'A' + shift) % 26;
        else if (ch >= '0' && ch <= '9')
            encryptedText += '0' + (ch - '0' + shift) % 10;
        else
            encryptedText += ch;
    }
    return encryptedText;
}

// Replace all words with Huffman codes
void replaceWithHuffmanCodes(const string &inputFile, const string &outputFile, unordered_map<string, string> &huffmanCodes)
{
    ifstream inFile(inputFile);
    ofstream outFile(outputFile);

    if (!inFile || !outFile)
    {
        cerr << "Error opening file!" << endl;
        return;
    }

    string word;
    while (inFile >> word)
    {
        if (huffmanCodes.find(word) != huffmanCodes.end())
        {
            outFile << huffmanCodes[word] << " ";
        }
    }

    inFile.close();
    outFile.close();
}

int main()
{
    int choice;

    HuffmanCoding huffman;
    do
    {
        cout << "\n=== Secure Text Processor ===\n";
        cout << "1. Full File Encryption\n";
        cout << "2. Full File Decryption\n";
        cout << "3. Encrypt Specific Words or Sentences\n";
        cout << "4. Decrypt Specific Words or Sentences\n";
        cout << "5. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        if (choice == 1)
        {
            string filename;
            cout << "Enter input file to encrypt: ";
            cin >> filename;

            ifstream check(filename);
            if (!check.is_open())
            {
                cerr << "File not found!\n";
                continue;
            }
            check.close();

            insertSaltedWords(filename, "saltedFile.txt");

            ifstream inputFile("saltedFile.txt");
            unordered_map<string, int> wordFrequency;
            string word, line;

            while (getline(inputFile, line))
            {
                istringstream stream(line);
                while (stream >> word)
                {
                    wordFrequency[word]++;
                }
            }
            inputFile.close();

            AVLTree avlTree;
            for (const auto &pair : wordFrequency)
            {
                avlTree.insert(pair.first, pair.second);
            }

            huffman.buildFromAVL(avlTree);
            unordered_map<string, string> huffmanCodes = huffman.getCodes();

            huffman.saveHuffmanCodes(huffmanCodes, "huffman_codes.txt");

            replaceWithHuffmanCodes("saltedFile.txt", "encodedFile.txt", huffmanCodes);
            cout << "✅ Encryption complete. Output: encodedFile.txt\n";
        }
        else if (choice == 2)
        {
            string fileName;
            cout << "Enter output filename for decrypted text: ";
            cin >> fileName;

            HuffmanCoding huffman; // ✅ Create HuffmanCoding object
            unordered_map<string, string> huffmanCodes = huffman.loadHuffmanCodes("huffman_codes.txt"); // ✅ Call member function

            if (huffmanCodes.empty())
            {
                cerr << "Decryption aborted: Huffman codes not found.\n";
                continue;
            }

            huffman.printCodes();
            Decryption d(huffmanCodes);
            d.decryptFile(fileName, "decryptedFile.txt"); // ✅ Decrypt!
            cout << "Decryption complete. Output: decryptedFile.txt" << "\n";
        }
        else if (choice == 3)
        {
            cout << "🔒 Encrypting specific words not implemented yet.\n";
        }
        else if (choice == 4)
        {
            cout << "🔓 Decrypting specific words not implemented yet.\n";
        }

    } while (choice != 5);

    cout << "Goodbye!\n";
    return 0;
}
