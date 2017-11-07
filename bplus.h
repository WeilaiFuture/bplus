#ifndef _BPLUS
#define _BPLUS

#include <iostream>
#include <fstream>
#include <stack>
#include "diskmngr.h"

using namespace std;

#define TMPL template <typename Key, typename Data>

TMPL class BPlusTree {
private:
    class Node {
    public:
        Key * keys;
        Data * data;
        Node ** children;
        Node * next;
        Node * prev;
        bool isLeaf;
        int numberKeys;
        explicit Node(int t);
        ~Node();
    };
    int t; // branching factor
//    int pageSize;
    Node * root;
    stack <Node *> nodeStack;


    void destroyTree(Node * n);
    Node * findLeaf(Node * n, Key key);
    void insertInLeaf(Node * n, Key key, Data data);
    Key splitLeaf(Node * n, Key key, Data data);
    void createNewRoot(Key divider, Node * left, Node * right);
    void insertInInternal(Node * n, Key key, Node * left, Node * right);
    Key splitInternal(Node * n, Key key, Node * left, Node * right);
    void printNode(Node * n);
public:
    explicit BPlusTree(int t);
    ~BPlusTree();
    void insert(Key key, Data data);
    void printTree();
};

//---------------------------------
// Tree Constructor and Destructor
//---------------------------------


TMPL BPlusTree<Key, Data>::BPlusTree(int t) {
    this->t = t;
    this->root = new Node(t);
}

TMPL BPlusTree<Key, Data>::~BPlusTree() {
    destroyTree(root);
}

TMPL void BPlusTree<Key, Data>::destroyTree(Node * n) {
    if (n == NULL) {
        return;
    }
    if (!n->isLeaf) {
        for (int i = 0; i <= n->numberKeys; i++) {
            destroyTree(n->children[i]);
        }
    }
    delete n;
}

//---------------------------------
// Node Constructor and Destructor
//---------------------------------

TMPL BPlusTree<Key, Data>::Node::Node(int t) {
    isLeaf = true;
    keys = new Key[2 * t - 1];
    data = new Data[2 * t - 1];
    children = new Node * [2 * t];
    next = NULL;
    prev = NULL;
    numberKeys = 0;
}

TMPL BPlusTree<Key, Data>::Node::~Node() {
    delete keys;
    delete data;
    delete children;
}

//---------------------------------
// Insertion
//---------------------------------

// After findLeaf() stack contains full path from root to target leaf's parent

TMPL typename BPlusTree<Key, Data>::Node * BPlusTree<Key, Data>::findLeaf(Node * n, Key key) {
    if (n->isLeaf)
        return n;

    nodeStack.push(n);

    int i = 0;
    while (i < n->numberKeys && n->keys[i] < key) i++;
    return findLeaf(n->children[i], key);
}


TMPL void BPlusTree<Key, Data>::insert(Key key, Data data) {
    Node * leaf = findLeaf(root, key);
    insertInLeaf(leaf, key, data);

    while (!nodeStack.empty()) {
        nodeStack.pop();
    }
}

TMPL void BPlusTree<Key, Data>::insertInLeaf(Node * n, Key key, Data data) {
    int i = 0;
    if (n->numberKeys < 2 * t - 1) {
        // Leaf in not full
        while (i < n->numberKeys && n->keys[i] < key) i++;
        for (int j = n->numberKeys - 1; j >= i; j--) {
            n->keys[j + 1] = n->keys[j];
            n->data[j + 1] = n->data[j];
        }
        n->keys[i] = key;
        n->data[i] = data;
        n->numberKeys++;
    } else {
        // Leaf is full
        Key divider = splitLeaf(n, key, data);
        if (nodeStack.empty()) {
            createNewRoot(divider, n, n->next);
        } else {
            Node * parent = nodeStack.top();
            nodeStack.pop();
            insertInInternal(parent, divider, n, n->next);
        }
    }
}

TMPL Key BPlusTree<Key, Data>::splitLeaf(Node * n, Key key, Data data) {
    Node * right = new Node(t);
    int i = 0;
    while (i < n->numberKeys && n->keys[i] < key) i++;
    if (i <= t - 1) {
        // insert in left
        for (int j = t - 1; j < n->numberKeys; j++) {
            right->keys[j - t + 1] = n->keys[j];
            right->data[j - t + 1] = n->data[j];
        }
        right->next = n->next;
        right->prev = n;
        n->next = right;
        n->numberKeys = t - 1;
        right->numberKeys = t;

        insertInLeaf(n, key, data);
    } else {
        // insert in right
        for (int j = t; j < n->numberKeys; j++) {
            right->keys[j - t] = n->keys[j];
            right->data[j - t] = n->data[j];
        }
        right->next = n->next;
        right->prev = n;
        n->next = right;
        n->numberKeys = t;
        right->numberKeys = t - 1;

        insertInLeaf(right, key, data);
    }
    return n->keys[n->numberKeys - 1];
}

TMPL void BPlusTree<Key, Data>::createNewRoot(Key divider, Node * left, Node * right) {
    Node * n = new Node(t);
    n->isLeaf = false;
    n->numberKeys = 1;
    n->keys[0] = divider;
    n->children[0] = left;
    n->children[1] = right;

    root = n;
}


TMPL void BPlusTree<Key, Data>::insertInInternal(Node * n, Key key, Node * left, Node * right) {
    int i = 0;
    while (i < n->numberKeys && n->keys[i] < key) i++;

    if (n->numberKeys < 2 * t - 1) {
        n->children[n->numberKeys + 1] = n->children[n->numberKeys];
        for (int j = n->numberKeys - 1; j >= i; j--) {
            n->children[j + 1] = n->children[j];
            n->keys[j + 1] = n->keys[j];
        }
        n->keys[i] = key;
        n->children[i] = left;
        n->children[i + 1] = right;
        n->numberKeys++;
    } else {
        Key divider = splitInternal(n, key, left, right);
        if (nodeStack.empty()) {
            createNewRoot(divider, n, n->next);
        } else {
            Node * parent = nodeStack.top();
            nodeStack.pop();
            insertInInternal(parent, divider, n, n->next);
        }
    }
}

TMPL Key BPlusTree<Key, Data>::splitInternal(Node * n, Key key, Node * left, Node * right) {
    Node * rightInternal = new Node(t);
    int i = 0;
    while (i < n->numberKeys && n->keys[i] < key) i++;

    for (int j = t; j < n->numberKeys; j++) {
        rightInternal->keys[j - t] = n->keys[j];
        rightInternal->children[j - t] = n->children[j];
    }
    rightInternal->children[t - 1] = n->children[n->numberKeys];
    rightInternal->next = n->next;
    rightInternal->prev = n;
    n->next = rightInternal;
    n->numberKeys = t;
    rightInternal->numberKeys = t - 1;

    // Insertion
    Node * target = n;
    if (i > t - 1) {
        target = rightInternal;
        i = i - t;
    }

    target->children[target->numberKeys] = target->children[target->numberKeys - 1];
    for (int j = target->numberKeys - 1; j >= i; j--) {
        target->children[j + 1] = target->children[j];
        target->keys[j + 1] = target->keys[j];
    }
    target->keys[i] = key;
    target->children[i] = left;
    target->children[i + 1] = right;
    target->numberKeys++;

    // Pass divider
    Key divider = n->keys[n->numberKeys - 1];
    n->numberKeys--;
    return divider;
}

//---------------------------------
// Printing tree
//---------------------------------

TMPL void BPlusTree<Key, Data>::printTree() {
    printNode(root);
}

TMPL void BPlusTree<Key, Data>::printNode(Node * n) {
    cout << "Node: " << n << endl;
    cout << "Keys: ";
    for (int i = 0; i < n->numberKeys; i++) {
        cout << n->keys[i] << " ";
    }
    cout << endl;
    if (!n->isLeaf) {
        for (int i = 0; i <= n->numberKeys; i++) {
            printNode(n->children[i]);
        }
    }
}

//---------------------------------

#endif