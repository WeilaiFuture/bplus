#include <iostream>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include "bplus.h"

#define PAGESIZE 4096
#define CACHESIZE 128

using namespace std;

int main(int argc, char * argv[]) {
    BPlusTree<int, int> * t = new BPlusTree<int, int>(PAGESIZE, CACHESIZE);

    chrono::time_point<chrono::system_clock> start, end;
    srand(time(NULL));

    int n = 1e6;
    int a = 0;

    if (argc >= 2) {
        n = atoi(argv[1]);
    }

    start = chrono::system_clock::now();
    for (int i = 0; i < n; i++) {
        a = rand();
        t->insert(a, 0);
    }
    end = chrono::system_clock::now();

    cout << chrono::duration_cast<chrono::seconds>(end - start).count();

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