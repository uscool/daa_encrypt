#ifndef AVL_TREE_HPP
#define AVL_TREE_HPP

#include <string>
#include <algorithm>

struct AVLNode {
    std::string word;
    int count;
    AVLNode* left;
    AVLNode* right;
    int height;

    AVLNode(const std::string& w, int c) : 
        word(w), count(c), left(nullptr), right(nullptr), height(1) {}
};

class AVLTree {
private:
    AVLNode* root;

    int getHeight(AVLNode* node) {
        if (node == nullptr) return 0;
        return node->height;
    }

    int getBalance(AVLNode* node) {
        if (node == nullptr) return 0;
        return getHeight(node->left) - getHeight(node->right);
    }

    AVLNode* rightRotate(AVLNode* y) {
        AVLNode* x = y->left;
        AVLNode* T2 = x->right;

        x->right = y;
        y->left = T2;

        y->height = std::max(getHeight(y->left), getHeight(y->right)) + 1;
        x->height = std::max(getHeight(x->left), getHeight(x->right)) + 1;

        return x;
    }

    AVLNode* leftRotate(AVLNode* x) {
        AVLNode* y = x->right;
        AVLNode* T2 = y->left;

        y->left = x;
        x->right = T2;

        x->height = std::max(getHeight(x->left), getHeight(x->right)) + 1;
        y->height = std::max(getHeight(y->left), getHeight(y->right)) + 1;

        return y;
    }

    AVLNode* insert(AVLNode* node, const std::string& word, int count) {
        if (node == nullptr)
            return new AVLNode(word, count);

        if (count < node->count)
            node->left = insert(node->left, word, count);
        else if (count > node->count)
            node->right = insert(node->right, word, count);
        else {
            if (word < node->word)
                node->left = insert(node->left, word, count);
            else if (word > node->word)
                node->right = insert(node->right, word, count);
            else
                return node;
        }

        node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));

        int balance = getBalance(node);

        // Left Left Case
        if (balance > 1 && count < node->left->count)
            return rightRotate(node);

        // Right Right Case
        if (balance < -1 && count > node->right->count)
            return leftRotate(node);

        // Left Right Case
        if (balance > 1 && count > node->left->count) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }

        // Right Left Case
        if (balance < -1 && count < node->right->count) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }

        return node;
    }

    void inorderTraversal(AVLNode* node) const {
        if (node != nullptr) {
            inorderTraversal(node->left);
            std::cout << node->word << ": " << node->count << std::endl;
            inorderTraversal(node->right);
        }
    }

    void cleanup(AVLNode* node) {
        if (node != nullptr) {
            cleanup(node->left);
            cleanup(node->right);
            delete node;
        }
    }

public:
    AVLTree() : root(nullptr) {}

    ~AVLTree() {
        cleanup(root);
    }

    void insert(const std::string& word, int count) {
        root = insert(root, word, count);
    }

    void printInorder() const {
        inorderTraversal(root);
    }

    AVLNode* getRoot() const {
        return root;
    }
};

#endif 