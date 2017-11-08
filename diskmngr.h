#ifndef _DISKMNGR
#define _DISKMNGR

#include <string>
#include <fstream>
#include <sys/stat.h>

#define DBFILE "/Users/john/Code/isp/bplus/db"

using namespace std;


class DiskManager {
private:
    int * cacheOffset;
    char ** cachePointer;
    int cacheCursor;
    int pageSize;
    int cacheSize;
    fstream file;
    void writeToCache(char * p, int offset);
    int lookInCache(int offset);
public:
    DiskManager(int pageSize, int cacheSize);
    ~DiskManager();

    void * readFromDisk(int offset);
    int writeToDisk(void * page);
    void writeToDisk(void * page, int offset);
    int getFileSize();
};

//---------------------------------
// Disk Manager Constructor and Destructor
//---------------------------------

DiskManager::DiskManager(int pageSize, int cacheSize) {
    this->pageSize = pageSize;
    this->cacheSize = cacheSize;
    cacheOffset = new int[cacheSize];
    cachePointer = new char * [cacheSize];
    cacheCursor = 0;
    for (int i = 0; i < cacheSize; i++) {
        cacheOffset[i] = -1;
        cachePointer[i] = NULL;
    }

    file.open(DBFILE, ios::out | ios::binary | ios::trunc);
    file.seekp(0, ios::beg);
    file.write((char *)&pageSize, (int)sizeof(pageSize));
    file.close();

    file.open(DBFILE, ios::in | ios::out | ios::binary);
}

DiskManager::~DiskManager() {
    for (int i = 0; i < cacheSize; i++) {
        delete cachePointer[i];
    }
    delete cachePointer;
    delete cacheOffset;

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

int DiskManager::writeToDisk(void * page) {
    int offset = getFileSize();
    writeToDisk(page, offset);
    return offset;
}

void DiskManager::writeToDisk(void * page, int offset) {
    if (offset > getFileSize()) {
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

    int os = lookInCache(offset);
    if (os != -1) {
        return (void *)cachePointer[os];
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
    for (int i = 0; i < cacheSize; i++) {
        if (cacheOffset[i] == offset) {
            delete cachePointer[i];
            cachePointer[i] = p;
            return;
        }
    }

    if (cachePointer[cacheCursor] != NULL) {
        delete cachePointer[cacheCursor];
    }
    cachePointer[cacheCursor] = p;
    cacheOffset[cacheCursor] = offset;
    cacheCursor = (cacheCursor + 1) % cacheSize;
}

int DiskManager::lookInCache(int offset) {
    for (int i = 0; i < cacheSize; i++) {
        if (cacheOffset[i] == offset) {
            return i;
        }
    }
    return -1;
}

#endif