#ifndef _DISKMNGR
#define _DISKMNGR

#include <string>
#include <fstream>
#include <sys/stat.h>
#include "lrucache.h"

#define DBFILE "/Users/john/Code/isp/bplus/db"

using namespace std;


class DiskManager {
private:
    LRUCache<int, char *> * cache;
    int pageSize;
    int cacheSize;
    fstream file;
    void writeToCache(char * p, int offset);
    char * lookInCache(int offset);
    int getFileSize();
public:
    DiskManager(int pageSize, int cacheSize);
    ~DiskManager();

    void * readFromDisk(int offset);
    int getNewOffset();
    void writeToDisk(void * page, int offset);
};

//---------------------------------
// Disk Manager Constructor and Destructor
//---------------------------------

DiskManager::DiskManager(int pageSize, int cacheSize) {
    this->pageSize = pageSize;
    this->cacheSize = cacheSize;

    cache = new LRUCache<int, char *>(cacheSize);

    file.open(DBFILE, ios::out | ios::binary | ios::trunc);
    file.seekp(0, ios::beg);
    file.write((char *)&pageSize, (int)sizeof(pageSize));
    file.close();

    file.open(DBFILE, ios::in | ios::out | ios::binary);
}

DiskManager::~DiskManager() {
    delete cache;

    file.close();
}

//---------------------------------
//  Write to Disk
//---------------------------------

// seekg() for read, seekp() for write

int DiskManager::getFileSize() {
    file.seekg(0, ios::end);
    return (int)file.tellg();
}

int DiskManager::getNewOffset() {
    return getFileSize();
}

void DiskManager::writeToDisk(void * page, int offset) {
    if (offset > getFileSize()) {
        printf("offset %d\n", offset);
        fflush(stdout);
        throw invalid_argument("too big offset");
    }
    file.seekp(offset, ios::beg);
    file.write((char *)page, pageSize);

    char * p = new char[pageSize];
    memcpy(p, page, (size_t)pageSize);
    writeToCache(p, offset);
}

//---------------------------------
// Read from Disk
//---------------------------------

// seekg() for read, seekp() for write

void * DiskManager::readFromDisk(int offset) {
    if (pageSize + offset > getFileSize()) {
        throw invalid_argument("too big offset");
    }

    char * po = lookInCache(offset);
    if (po != NULL) {
        return (void *)po;
    }

    char * p = new char[pageSize];
    file.seekg(offset, ios::beg);
    file.read(p, pageSize);

    writeToCache(p, offset);

    return p;
}

//---------------------------------
// Cache Handling
//---------------------------------

void DiskManager::writeToCache(char * p, int offset) {
    cache->write(offset, p);
}

char * DiskManager::lookInCache(int offset) {
    try {
        char * p = cache->read(offset);
        return p;
    } catch (std::invalid_argument) {
        return NULL;
    }
}

#endif