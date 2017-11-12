#ifndef _LRUCACHE
#define _LRUCACHE

#include <unordered_map>
#include <list>
#include <utility>

template <typename Key, typename Value>
class LRUCache {
public:
    typedef std::pair<Key, Value> pair_t;
    typedef typename std::list<pair_t>::iterator list_it;

    LRUCache(int size) { cacheSize = size; };
    void write(Key key, Value value);
    Value read(Key key);

private:
    int cacheSize;
    std::list<pair_t> cacheList;
    std::unordered_map<Key, list_it> cacheMap;
};

template <typename Key, typename Value>
void LRUCache<Key, Value>::write(Key key, Value value) {
    auto i = cacheMap.find(key);
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

template <typename Key, typename Value>
Value LRUCache<Key, Value>::read(Key key) {
    auto i = cacheMap.find(key);
    if (i == cacheMap.end()) {
        throw std::invalid_argument("no such key in cache"); // !!! handle this exception in diskmngr
    } else {
        cacheList.splice(cacheList.cbegin(), cacheList, i->second);
    }
    return i->second->second;
}

#endif