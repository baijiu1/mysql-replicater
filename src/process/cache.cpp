//
// Created by 白杰 on 2024/5/18.
//

#include "cache.h"
#include "parser.h"
#include <map>
using namespace std;
map<uint32_t, struct ComBinlogDumpFields*> Globalcache;
map<int, vector<struct table_map_event_field_info> > fieldInfoCache;

int addToCache(uint32_t key, struct ComBinlogDumpFields* value) {
    Globalcache[key] = value;
};

struct ComBinlogDumpFields* getFromCache(uint32_t key) {
    auto it = Globalcache.find(key);
    if (it != Globalcache.end()) {
        return it->second;
    }
    return nullptr;
}

int addFieldsToCache(uint32_t key, struct table_map_event_field_info value) {
//    fieldInfoCache[key] = value;
};