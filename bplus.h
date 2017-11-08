#ifndef _BPLUS
#define _BPLUS

#include <iostream>
#include <fstream>
#include <stack>
#include <algorithm>
#include "diskmngr.h"

#define TMPL template <typename Key, typename Data>

/*
Node structure on disk:
 offset (int)       |
 numberKeys (int)   |
 isLeaf (bool)      |   HEADER
 next (int)         |
 prev (int)         |
 keys (2t-1 Key)    |
 children (2t  int) or data (2t-1  data)
*/


using namespace std;

TMPL class BPlusTree {
private:
    class Node {
    public:
        int offset;
        Key * keys;
        Data * data;
        int * children;
        int next;
        int prev;
        bool isLeaf;
        int numberKeys;
        explicit Node(int t);
        ~Node();
    };
    int t; // branching factor
    int pageSize;
    int headerSize;
    Node * root;
    stack <Node *> nodeStack;
    DiskManager * dm;


    Node * findLeaf(Node * n, Key key);
    void insertInLeaf(Node * n, Key key, Data data);
    Key splitLeaf(Node * n, Key key, Data data);
    void createNewRoot(Key divider, int left, int right);
    void insertInInternal(Node * n, Key key, int left, int right);
    Key splitInternal(Node * n, Key key, int left, int right);
    void printNode(Node * n);
    void loadNode(Node * n, int offset);
    void saveNode(Node * n);
public:
    explicit BPlusTree(int pageSize, int cacheSize);
    ~BPlusTree();
    void insert(Key key, Data data);
    void printTree();
    Data search(Key key);
};

//---------------------------------
// Tree Constructor and Destructor
//---------------------------------


TMPL BPlusTree<Key, Data>::BPlusTree(int pageSize, int cacheSize) {
    dm = new DiskManager(pageSize, cacheSize);

    this->pageSize = pageSize;
    int d = max(sizeof(Data), sizeof(int));
    t = (pageSize - 4 * sizeof(int) - sizeof(bool) + sizeof(Key))/(2 * d + 2 * sizeof(Key));
    headerSize = sizeof(int) * 4 + sizeof(bool) + (2 * t - 1) * sizeof(Key);

    root = new Node(t);
    root->offset = dm->getNewOffset();
    saveNode(root);

    cout << "Branching factor: " << t << endl << endl;
}

TMPL BPlusTree<Key, Data>::~BPlusTree() {
    delete root;
    delete dm;
}

//---------------------------------
// Node Constructor and Destructor
//---------------------------------

TMPL BPlusTree<Key, Data>::Node::Node(int t) {
    offset = -1;
    isLeaf = true;
    keys = new Key[2 * t - 1];
    data = new Data[2 * t - 1];
    children = new int [2 * t];
    next = -1;
    prev = -1;
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

    Node * child = new Node(t);
    loadNode(child, n->children[i]);
    return findLeaf(child, key);
}


TMPL void BPlusTree<Key, Data>::insert(Key key, Data data) {
    Node * leaf = findLeaf(root, key);
    insertInLeaf(leaf, key, data);

    while (!nodeStack.empty() && nodeStack.top() != root) {
        delete nodeStack.top();
        nodeStack.pop();
    }
    if (!nodeStack.empty())
        nodeStack.pop();
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
        saveNode(n);
        if (n != root)
            delete n;
    } else {
        // Leaf is full
        int offset = n->offset;
        int nextOffset = dm->getNewOffset();
        Key divider = splitLeaf(n, key, data);
        if (nodeStack.empty()) {
            createNewRoot(divider, offset, nextOffset);
        } else {
            Node * parent = nodeStack.top();
            nodeStack.pop();
            insertInInternal(parent, divider, offset, nextOffset);
        }
    }
}

TMPL Key BPlusTree<Key, Data>::splitLeaf(Node * n, Key key, Data data) {
    Node * right = new Node(t);
    right->offset = dm->getNewOffset();

    int divider = 0;

    int i = 0;
    while (i < n->numberKeys && n->keys[i] < key) i++;

    if (i <= t - 1) {
        // insert in left
        for (int j = t - 1; j < n->numberKeys; j++) {
            right->keys[j - t + 1] = n->keys[j];
            right->data[j - t + 1] = n->data[j];
        }
        right->next = n->next;
        right->prev = n->offset;
        n->next = right->offset;
        n->numberKeys = t - 1;
        right->numberKeys = t;

        divider = n->keys[n->numberKeys - 1];
        if (i == t - 1)
            divider = key;

        insertInLeaf(n, key, data);
        saveNode(right);
        delete right;
    } else {
        // insert in right
        for (int j = t; j < n->numberKeys; j++) {
            right->keys[j - t] = n->keys[j];
            right->data[j - t] = n->data[j];
        }
        right->next = n->next;
        right->prev = n->offset;
        n->next = right->offset;
        n->numberKeys = t;
        right->numberKeys = t - 1;

        divider = n->keys[n->numberKeys - 1];

        insertInLeaf(right, key, data);
        saveNode(n);
    }
    return divider;
}

TMPL void BPlusTree<Key, Data>::createNewRoot(Key divider, int left, int right) {
    Node * n = new Node(t);
    n->offset = dm->getNewOffset();
    n->isLeaf = false;
    n->numberKeys = 1;
    n->keys[0] = divider;
    n->children[0] = left;
    n->children[1] = right;

    saveNode(n);

    root = n;
}


TMPL void BPlusTree<Key, Data>::insertInInternal(Node * n, Key key, int left, int right) {
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
        saveNode(n);
        if (n != root)
            delete n;
    } else {
        Key divider = splitInternal(n, key, left, right);
        if (nodeStack.empty()) {
            createNewRoot(divider, n->offset, n->next);
        } else {
            Node * parent = nodeStack.top();
            nodeStack.pop();
            insertInInternal(parent, divider, n->offset, n->next);
        }
        saveNode(n);
        delete n;
    }
}

TMPL Key BPlusTree<Key, Data>::splitInternal(Node * n, Key key, int left, int right) {
    Node * rightInternal = new Node(t);
    rightInternal->offset = dm->getNewOffset();
    int i = 0;
    while (i < n->numberKeys && n->keys[i] < key) i++;

    for (int j = t; j < n->numberKeys; j++) {
        rightInternal->keys[j - t] = n->keys[j];
        rightInternal->children[j - t] = n->children[j];
    }
    rightInternal->isLeaf = false;
    rightInternal->children[t - 1] = n->children[n->numberKeys];
    rightInternal->next = n->next;
    rightInternal->prev = n->offset;
    n->next = rightInternal->offset;
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
    saveNode(rightInternal);
    saveNode(n);
    delete rightInternal;
    return divider;
}

//---------------------------------
// Printing tree
//---------------------------------

TMPL void BPlusTree<Key, Data>::printTree() {
    printNode(root);
}

TMPL void BPlusTree<Key, Data>::printNode(Node * n) {
    cout << "Node's offset: " << n->offset << endl;
    cout << "Keys: ";
    for (int i = 0; i < n->numberKeys; i++) {
        cout << n->keys[i] << " ";
    }
    cout << endl;
    Node * child;
    int i = 0;
    if (!n->isLeaf) {
        for (i = 0; i <= n->numberKeys; i++) {
            child = new Node(t);
            loadNode(child, n->children[i]);
            printNode(child);
            delete child;
        }
    }
}

//---------------------------------
// Disk operations
//---------------------------------

// Considering that Node is already allocated

TMPL void BPlusTree<Key, Data>::loadNode(Node * n, int offset) {
    char * buff = new char[pageSize];
    memcpy(buff, dm->readFromDisk(offset), pageSize);

    int i = 0;

    memcpy(&n->offset, buff + i, sizeof(n->offset));
    i += sizeof(n->offset);

    memcpy(&n->numberKeys, buff + i, sizeof(n->numberKeys));
    i += sizeof(n->numberKeys);

    memcpy(&n->isLeaf, buff + i, sizeof(n->isLeaf));
    i += sizeof(n->isLeaf);

    memcpy(&n->next, buff + i, sizeof(n->next));
    i += sizeof(n->next);

    memcpy(&n->prev, buff + i, sizeof(n->prev));
    i += sizeof(n->prev);

    for (int j = 0; j < 2 * t - 1; j++) {
        memcpy(&n->keys[j], buff + i, sizeof(Key));
        i += sizeof(Key);
    }

    if (n->isLeaf) {
        for (int j = 0; j < 2 * t - 1; j++) {
            memcpy(&n->data[j], buff + i, sizeof(Data));
            i += sizeof(Data);
        }
    } else {
        for (int j = 0; j < 2 * t; j++) {
            memcpy(&n->children[j], buff + i, sizeof(n->children[0]));
            i += sizeof(n->children[0]);
        }
    }

    delete[] buff;
}

TMPL void BPlusTree<Key, Data>::saveNode(Node * n) {
    char * buff = new char[pageSize];
    if (n->offset == -1) {
        n->offset = dm->getNewOffset();
    }

    int i = 0;

    memcpy(buff + i, &n->offset, sizeof(n->offset));
    i += sizeof(n->offset);

    memcpy(buff + i, &n->numberKeys, sizeof(n->numberKeys));
    i += sizeof(n->numberKeys);

    memcpy(buff + i, &n->isLeaf, sizeof(n->isLeaf));
    i += sizeof(n->isLeaf);

    memcpy(buff + i, &n->next, sizeof(n->next));
    i += sizeof(n->next);

    memcpy(buff + i, &n->prev, sizeof(n->prev));
    i += sizeof(n->prev);

    for (int j = 0; j < 2 * t - 1; j++) {
        memcpy(buff + i, &n->keys[j], sizeof(Key));
        i += sizeof(Key);
    }

    if (n->isLeaf) {
        for (int j = 0; j < 2 * t - 1; j++) {
            memcpy(buff + i, &n->data[j], sizeof(Data));
            i += sizeof(Data);
        }
    } else {
        for (int j = 0; j < 2 * t; j++) {
            memcpy(buff + i, &n->children[j], sizeof(n->children[0]));
            i += sizeof(n->children[0]);
        }
    }

    dm->writeToDisk((void *)buff, n->offset);
    delete[] buff;
}


//---------------------------------
// Search
//---------------------------------

TMPL Data BPlusTree<Key, Data>::search(Key key) {
    int i = 0;

    Node * n = new Node(t);
    loadNode(n, root->offset);
    while (!n->isLeaf) {
        while (i < n->numberKeys && n->keys[i] < key) i++;
        loadNode(n, n->children[i]);
    }
    while (i < n->numberKeys && n->keys[i] < key) i++;
    if (n->keys[i] != key) {
        throw invalid_argument("key not present in tree");
    }
    return n->data[i];

}


#endif