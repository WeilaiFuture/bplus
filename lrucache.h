#ifndef _LRUCACHE
#define _LRUCACHE

#include <unordered_map>
#include <list>
#include <utility>

#define TMPL template <typename Key, typename Value>

TMPL class LRUCache {
public:
    typedef std::pair<Key, Value> pair_t;
    typedef std::list<pair_t>::iterator list_it;

    LRUCache(int size) { cacheSize = size; };
    void write(Key key, Value value);
    Value read(Key key);

private:
    int cacheSize;
    std::list<pair_t> cacheList;
    std::unordered_map<Key, list_it> cacheMap;
};

TMPL void LRUCache::write(Key key, Value value) {
    std::unordered_map::iterator i = cacheMap.find(key);
    if (i == cacheMap.end()) {
        // no such element in map
        if (cacheMap.size() + 1 > cacheSize) {
            // pop back last element from list and delete the pointer stored in it

            auto last = cacheList.crbegin();
            Value v = last->second;
            delete v; // specific for bplus tree implementation
            cacheMap.erase(last->first);
            cacheList.pop_back();
        }
        cacheList.push_front(std::make_pair(key, value));
        cacheMap[key] = cacheList.begin();
    } else {
        Value v = i->second->second;
        delete v;
        i->second->second = value;
        cacheList.splice(cacheList.cbegin(), cacheList, i->second);
    }
}

TMPL Value LRUCache::read(Key key) {
    std::unordered_map::iterator i = cacheMap.find(key);
    if (i == cacheMap.end()) {
        throw std::invalid_argument("no such key in cache"); // !!! handle this exception in diskmngr
    } else {
        cacheList.splice(cacheList.cbegin(), cacheList, i->second);
    }
    return i->second->second;
}

#endif