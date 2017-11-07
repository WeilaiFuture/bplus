#ifndef _BPLUS
#define _BPLUS

#include <iostream>
#include <fstream>

#define TMPL template <typename Key, typename Data>

TMPL class BPlusTree {
private:
    class Node {
    public:
        Key * keys;
        Data * data;
        Node ** children;
        bool isLeaf;
        int numberKeys;
        Node(int t);
//        Node(Node * n);
        ~Node();
    };
    int t;
//    int pageSize;
    Node * root;
    void destroyTree(Node * n);
public:
    BPlusTree(int t);
    ~BPlusTree();
};

TMPL BPlusTree<Key, Data>::BPlusTree(int t) {
    this->t = t;
    root = new Node(t);
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

TMPL BPlusTree<Key, Data>::Node::Node(int t) {
    isLeaf = true;
    keys = new Key[2 * t - 1];
    data = new Data[2 * t - 1];
    children = new Node * [2 * t];
    numberKeys = 0;
}

TMPL BPlusTree<Key, Data>::Node::~Node() {
    delete keys;
    delete data;
    delete children;
}

//---------------------------------

#endif