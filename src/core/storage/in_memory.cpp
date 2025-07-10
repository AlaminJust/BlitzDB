#include "storage/in_memory.h"

namespace blitzdb {

    void InMemoryStorage::set(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        data_[key] = value;
    }

    std::string InMemoryStorage::get(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = data_.find(key);
        return (it != data_.end()) ? it->second : "";
    }

    bool InMemoryStorage::del(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        return data_.erase(key) > 0;
    }

} // namespace blitzdb