#include <iostream>
#include "bplus.h"

#define PAGESIZE 48
#define CACHESIZE 10

using namespace std;

int main() {
    BPlusTree<int, int> * t = new BPlusTree<int, int>(PAGESIZE, CACHESIZE);
    int a[] = {5, 10, 15, 20, 25, 0, 1, 30, 3, 4, 21, 22, 2};

    int n = sizeof(a) / sizeof(a[0]);
    for (int i = 0; i < n; i++) {
        t->insert(a[i], 10);
    }

    t->printTree();


//    DiskManager * dm = new DiskManager(4, 2);
//    char a[] = "priv";
//    dm->writeToDisk((void *)a);
//    dm->writeToDisk((void *)a);
//    cout << (char *)dm->readFromDisk(4);
//    char b[] = "neet";
//    dm->writeToDisk((void *)b, 4);
//    delete dm;
    return 0;
}