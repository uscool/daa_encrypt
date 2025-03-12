#ifndef AVL_TREE_HPP
#define AVL_TREE_HPP

#include <string>
#include <algorithm>
#include <iostream>

using namespace std;

struct AVLNode {
    string word;
    int count;
    AVLNode *left, *right;
    int height;

    AVLNode(const string &w, int c) : word(w), count(c), left(nullptr), right(nullptr), height(1) {}
};

class AVLTree {
private:
    AVLNode *root;

    int getHeight(AVLNode *node) {
        return node ? node->height : 0;
    }

    int getBalance(AVLNode *node) {
        return node ? getHeight(node->left) - getHeight(node->right) : 0;
    }

    AVLNode* rightRotate(AVLNode *y) {
        AVLNode *x = y->left;
        AVLNode *T2 = x->right;

        x->right = y;
        y->left = T2;

        y->height = max(getHeight(y->left), getHeight(y->right)) + 1;
        x->height = max(getHeight(x->left), getHeight(x->right)) + 1;

        return x;
    }

    AVLNode* leftRotate(AVLNode *x) {
        AVLNode *y = x->right;
        AVLNode *T2 = y->left;

        y->left = x;
        x->right = T2;

        x->height = max(getHeight(x->left), getHeight(x->right)) + 1;
        y->height = max(getHeight(y->left), getHeight(y->right)) + 1;

        return y;
    }

    AVLNode* insert(AVLNode *node, const string &word, int count) {
        if (!node) return new AVLNode(word, count);

        if (count < node->count || (count == node->count && word < node->word))
            node->left = insert(node->left, word, count);
        else if (count > node->count || (count == node->count && word > node->word))
            node->right = insert(node->right, word, count);
        else
            return node;

        node->height = 1 + max(getHeight(node->left), getHeight(node->right));

        int balance = getBalance(node);

        if (balance > 1 && (count < node->left->count || (count == node->left->count && word < node->left->word)))
            return rightRotate(node);

        if (balance < -1 && (count > node->right->count || (count == node->right->count && word > node->right->word)))
            return leftRotate(node);

        if (balance > 1 && (count > node->left->count || (count == node->left->count && word > node->left->word))) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }

        if (balance < -1 && (count < node->right->count || (count == node->right->count && word < node->right->word))) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }

        return node;
    }

    void inorderTraversal(AVLNode *node) const {
        if (node) {
            inorderTraversal(node->left);
            cout << node->word << ": " << node->count << endl;
            inorderTraversal(node->right);
        }
    }

    void cleanup(AVLNode *node) {
        if (node) {
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

    void insert(const string &word, int count) {
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
