#ifndef RSA_HPP
#define RSA_HPP

#include <string>
#include <vector>
#include <random>
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <bitset>
#include <iostream>

class RSA {
private:
    long long p, q, n, phi, e, d;
    const int KEY_SIZE = 16; // Using small key size for demonstration

    // Generate a random prime number
    long long generatePrime() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<long long> dis(100, 1000);
        
        while (true) {
            long long num = dis(gen);
            if (isPrime(num)) {
                return num;
            }
        }
    }

    // Check if a number is prime
    bool isPrime(long long n) {
        if (n <= 1) return false;
        if (n <= 3) return true;
        if (n % 2 == 0 || n % 3 == 0) return false;

        for (long long i = 5; i * i <= n; i += 6) {
            if (n % i == 0 || n % (i + 2) == 0) return false;
        }
        return true;
    }

    // Extended Euclidean Algorithm
    long long modInverse(long long a, long long m) {
        long long m0 = m, t, q;
        long long x0 = 0, x1 = 1;

        if (m == 1) return 0;

        while (a > 1) {
            q = a / m;
            t = m;
            m = a % m;
            a = t;
            t = x0;
            x0 = x1 - q * x0;
            x1 = t;
        }

        if (x1 < 0) x1 += m0;
        return x1;
    }

    // Modular exponentiation
    long long modPow(long long base, long long exp, long long mod) {
        long long result = 1;
        base = base % mod;
        while (exp > 0) {
            if (exp & 1) result = (result * base) % mod;
            base = (base * base) % mod;
            exp >>= 1;
        }
        return result;
    }

    // Convert binary string to long long
    long long binaryToLong(const std::string& binary) {
        long long result = 0;
        for (char bit : binary) {
            result = (result << 1) | (bit - '0');
        }
        return result;
    }

    // Convert long long to binary string
    std::string longToBinary(long long num, int bits) {
        std::string result;
        for (int i = bits - 1; i >= 0; --i) {
            result += ((num >> i) & 1) ? '1' : '0';
        }
        return result;
    }

public:
    RSA() {
        // Generate two prime numbers
        p = generatePrime();
        q = generatePrime();
        
        // Calculate n and phi
        n = p * q;
        phi = (p - 1) * (q - 1);
        
        // Choose e (public key)
        e = 65537; // Common value for e
        
        // Calculate d (private key)
        d = modInverse(e, phi);
    }

    // Get public key
    std::pair<long long, long long> getPublicKey() const {
        return {e, n};
    }

    // Get private key
    std::pair<long long, long long> getPrivateKey() const {
        return {d, n};
    }

    // Encrypt a single number
    long long encrypt(long long message) {
        return modPow(message, e, n);
    }

    // Decrypt a single number
    long long decrypt(long long ciphertext) {
        return modPow(ciphertext, d, n);
    }

    // Encrypt a string while preserving spaces
    std::string encryptString(const std::string& message) {
        std::string result;
        bool firstChar = true;
        
        for (char c : message) {
            if (!firstChar) {
                result += " ";
            }
            long long m = static_cast<long long>(c);
            long long encrypted = encrypt(m);
            result += std::to_string(encrypted);
            firstChar = false;
        }
        
        return result;
    }

    // Decrypt a string while preserving spaces
    std::string decryptString(const std::string& encrypted) {
        std::string result;
        std::stringstream ss(encrypted);
        std::string token;
        
        while (ss >> token) {
            try {
                long long num = std::stoll(token);
                long long decrypted = decrypt(num);
                result += static_cast<char>(decrypted);
            } catch (const std::exception& e) {
                std::cerr << "Error decrypting token: " << token << std::endl;
            }
        }
        
        return result;
    }
};

#endif 
