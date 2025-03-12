#ifndef RANDOM_GENERATOR_HPP
#define RANDOM_GENERATOR_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unordered_map>
#include <cmath>  // For floor(), exp(), etc.
#include <algorithm> // For reverse(), random_shuffle
#include "caesar_cipher.hpp"

using namespace std;

// Euler's constant (e) for index calculation
const double EULER_CONSTANT = 2.718281828459045;

// Helper function to trim leading and trailing whitespaces
string trim(const string &str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == string::npos) return "";  // If no non-whitespace characters, return empty string
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, last - first + 1);
}

// Function to clean words (remove punctuation)
string cleanWordRandom(string word) {
    word = trim(word);  // Trim the word before processing
    if (word.empty()) return "";  // If word is empty after trimming, return empty string

    // Remove punctuation and replace with spaces
    for (char &c : word) {
        if (ispunct(c)) {
            c = ' ';  // Replace punctuation with a space
        }
    }
    return word;
}

// Function to generate a random word from the list of words
string generateRandomWord(vector<string> &words) {
    if (words.empty()) return "";

    // Get a random index
    int randomIndex = rand() % words.size();
    string word = words[randomIndex];

    // Salt the word: Apply random shuffling and character replacement
    random_shuffle(word.begin(), word.end());  // Shuffle characters in the word
    
    // Ensure the word length is between 5 and 7
    if (word.length() < 5) {
        // If the word is too short, add random characters to reach at least length 5
        while (word.length() < 5) {
            word += 'a' + rand() % 26;  // Append a random lowercase letter
        }
    } else if (word.length() > 7) {
        // If the word is too long, trim it to length 7
        word = word.substr(0, 7);
    }

    // Randomly replace characters in the word (you can customize this logic)
    for (char &c : word) {
        if (rand() % 2) {  // 50% chance to replace the character
            c = 'a' + rand() % 26;  // Replace with a random lowercase letter
        }
    }

    return word;
}

// Function to generate salted word index using Euler's constant
int complexEquationUsingE(int i, int totalWords) {
    return static_cast<int>(floor(EULER_CONSTANT * i)) % totalWords;
}

// Function to process words from file and store them in a hashmap
unordered_map<string, int> processWords(const string &filename) {
    srand(static_cast<unsigned int>(time(0))); // Initialize random seed

    ifstream file(filename);
    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        return {};
    }

    vector<string> words;
    string word;
    int CAESAR_SHIFT = 7;

    while (file >> word) {
        // Encrypt the word using Caesar Cipher and add it to the hashmap
        string encryptedWord = caesarCipherEncrypt(word, CAESAR_SHIFT);
        words.push_back(encryptedWord);  // Store the encrypted word
    }
    file.close();

    unordered_map<string, int> wordFreqMap;
    for (const string &encryptedWord : words) {
        // Increment the frequency of the encrypted word
        wordFreqMap[encryptedWord] += 1;  // or wordFreqMap[encryptedWord]++;
    }

    return wordFreqMap;
}

#endif
