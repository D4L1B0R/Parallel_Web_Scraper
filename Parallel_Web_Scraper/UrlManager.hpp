// Project: Parallel Web Scraper
// Name of an author: Nikolić Dalibor SV13-2023
// Date and time of the last changes: 16.09.2025. 10:10

#pragma once
#include "Downloader.hpp"
#include <string>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_unordered_set.h>
#include <unordered_set>

class UrlManager {
    tbb::concurrent_vector<std::string> urls;
    tbb::concurrent_unordered_set<std::string> visited;
public:
    void addUrl(const std::string& url);
    size_t loadFromFile(const std::string& path);
    void loadFromConsole();
    std::vector<std::string> getUrlsSnapshot() const;
    bool markVisitedIfNew(const std::string& url);
    size_t uniqueCount() const;
    // crawl index page (synchronously, returns discovered urls)
    int crawlIndex(Downloader& downloader, const std::string& baseUrl, int maxPages);
};