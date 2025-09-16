// Project: Parallel Web Scraper
// Name of an author: Nikolić Dalibor SV13-2023
// Date and time of the last changes: 16.09.2025. 10:10

#pragma once
#include <string>
#include <vector>
#include <unordered_set>

class UrlManager {
public:
    void addUrl(const std::string& url);
    size_t loadFromFile(const std::string& path);
    void loadFromConsole();
    std::vector<std::string> getUrlsSnapshot() const;
    bool markVisitedIfNew(const std::string& url);
    size_t uniqueCount() const;
    // new:
    // crawl index page (synchronously, returns discovered urls)
    std::vector<std::string> crawlIndex(const std::string& indexHtml, const std::string& baseUrl) const;

private:
    std::vector<std::string> urls;
    std::unordered_set<std::string> visited;
};