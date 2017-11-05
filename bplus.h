#ifndef _BPLUS
#define _BPLUS

#include <iostream>
#include <fstream>

#define TMPL template <typename Key, typename Data>

TMPL class BPlusTree;

TMPL class BPlusTree {
private:
    class Node {
    public:
        Key * keys;

    };
    int t;
public:

};


#endif