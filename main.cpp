#include <iostream>
#include <string>
#include "bplus.h"

using namespace std;

int main() {
    BPlusTree<int, int> * t = new BPlusTree<int, int>(2);
    int a[] = {5, 10, 15, 20, 0, 25, 30, 1};

    int n = sizeof(a) / sizeof(a[0]);
    for (int i = 0; i < n; i++) {
        t->insert(a[i], 10);
    }
    t->printTree();
    return 0;
}