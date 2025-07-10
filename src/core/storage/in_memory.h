#pragma once

#include <unordered_map>
#include <string>
#include <mutex>

namespace blitzdb {

    class InMemoryStorage {
    public:
        void set(const std::string& key, const std::string& value);
        std::string get(const std::string& key);
        bool del(const std::string& key);

    private:
        std::unordered_map<std::string, std::string> data_;
        std::mutex mutex_; // For thread safety
    };

} // namespace blitzdb