#include <iostream>
#include <string>
#include <cctype>  // For isalpha(), islower(), isupper()

// Function to encrypt a word using Caesar Cipher
std::string caesarCipherEncrypt(const std::string &word, int shift) {
    std::string encryptedWord = word;

    // Iterate over each character in the word
    for (char &c : encryptedWord) {
        if (isalpha(c)) {  // Only process alphabetic characters
            char base = islower(c) ? 'a' : 'A';  // Determine whether the letter is lowercase or uppercase
            c = (c - base + shift) % 26 + base;  // Apply Caesar shift with wrapping
        }
        // Non-alphabetic characters remain unchanged
    }

    return encryptedWord;
}
