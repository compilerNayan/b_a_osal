#ifndef CACHE_H
#define CACHE_H

#include "StandardDefines.h"
#include <unordered_map>
#include <esp_timer.h>

template <typename K, typename V>
class Cache {
    Private struct Entry {
        V value;
        ULongLong expiry; // microseconds since boot
    };
    
    Private StdUnorderedMap<K, Entry> cache_;
    Private ULongLong defaultTtlMs_;
    
    Private ULongLong Now() Const {
        return static_cast<ULongLong>(esp_timer_get_time()); // µs
    }
    
    Public Explicit Cache(ULongLong defaultTtlMs = 180000) 
        : defaultTtlMs_(defaultTtlMs) {}
    
    Public Void Put(const K& key, const V& value, ULongLong ttlMs) {
        ULongLong expiry = Now() + (ttlMs * 1000ULL);
        cache_[key] = { value, expiry };
    }
    
    Public Void Put(const K& key, const V& value) {
        ULongLong expiry = Now() + (defaultTtlMs_ * 1000ULL);
        cache_[key] = { value, expiry };
    }
    
    Public Optional<V> Get(const K& key) {
        auto it = cache_.find(key);
        if (it == cache_.end()) return std::nullopt;
    
        if (Now() > it->second.expiry) {
            cache_.erase(it);
            return std::nullopt;
        }
        return it->second.value;
    }
    
    Public Bool Contains(const K& key) {
        auto it = cache_.find(key);
        if (it == cache_.end()) return false;
        if (Now() > it->second.expiry) {
            cache_.erase(it);
            return false;
        }
        return true;
    }
    
    Public Void Remove(const K& key) {
        cache_.erase(key);
    }
    
    Public Void Clear() {
        cache_.clear();
    }
    
    Public Size Size() {
        for (auto it = cache_.begin(); it != cache_.end();) {
            if (Now() > it->second.expiry) {
                it = cache_.erase(it);
            } else {
                ++it;
            }
        }
        return cache_.size();
    }
    
    Public Void SetDefaultTtl(ULongLong ttlMs) {
        defaultTtlMs_ = ttlMs;
    }
    
    Public ULongLong GetDefaultTtl() Const {
        return defaultTtlMs_;
    }
};

#endif // CACHE_H
