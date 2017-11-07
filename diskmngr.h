#ifndef _DISKMNGR
#define _DISKMNGR

#include <string>

#define DBFILE "./db"

using namespace std;

class DiskManager {
private:
    int * cash;
    int pageSize;
    int cashSize;
    string fileName;
public:
    DiskManager(int pageSize, int cashSize);
    ~DiskManager();

    void * readFromDisk(int offset);
    void writeToDisk(void * node);
    void writeToDisk(void * node, int offset);
};

DiskManager::DiskManager(int pageSize, int cashSize) {
    this->pageSize = pageSize;
    this->cashSize = cashSize;
    cash = new int[cashSize];
}

DiskManager::~DiskManager() {
    delete cash;
}

#endif