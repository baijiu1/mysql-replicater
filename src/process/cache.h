//
// Created by 白杰 on 2024/5/18.
//

#ifndef MYSQL_REPLICATER_CACHE_H
#define MYSQL_REPLICATER_CACHE_H

#include <map>
#include <string>
#include "parser.h"

using namespace std;


int addToCache(uint32_t key, struct ComBinlogDumpFields* value);
struct ComBinlogDumpFields* getFromCache(uint32_t key);
//
//class Cache {
//public:
//    static Cache& getInstance() {
//        static Cache instance;
//        return instance;
//    }
//
//    void addToCache(uint32_t key, struct ComBinlogDumpFields* value) {
//        cache[key] = value;
//    }
//
//    struct ComBinlogDumpFields* getFromCache(uint32_t key) {
//        auto it = cache.find(key);
//        if (it != cache.end()) {
//            return it->second;
//        }
//        return nullptr;
//    }
//
////private:
////    Cache() {}
////    ~Cache() {}
//
//    std::map<uint32_t, struct ComBinlogDumpFields*> cache;
//
//    // Delete copy constructor and assignment operator to prevent copies
////    Cache(const Cache&) = delete;
////    Cache& operator=(const Cache&) = delete;
//};


#endif //MYSQL_REPLICATER_CACHE_H
