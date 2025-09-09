#pragma once
#include <string>
#include <vector>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_unordered_set.h>

class UrlManager {
public:
    UrlManager() = default;
    // Add a single URL (thread-safe)
    void addUrl(const std::string& url);
    // load urls from a newline-separated file; returns how many added
    size_t loadFromFile(const std::string& path);
    // load urls interactively from console until empty line
    void loadFromConsole();
    // snapshot of URLs to process
    std::vector<std::string> getUrlsSnapshot() const;
    // visited check (thread-safe)
    bool markVisitedIfNew(const std::string& url);

    size_t uniqueCount() const;

private:
    tbb::concurrent_vector<std::string> urls;
    tbb::concurrent_unordered_set<std::string> visited;
};