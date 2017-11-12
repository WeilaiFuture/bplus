#include <iostream>
#include "bplus.h"

#define PAGESIZE 4096
#define CACHESIZE 100

using namespace std;

int main() {
    BPlusTree<int, int> * t = new BPlusTree<int, int>(PAGESIZE, CACHESIZE);

    int n = 1e5;

    for (int i = 0; i < n; i++) {
        t->insert(i, i);
    }

    delete t;

//    t->printTree();


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