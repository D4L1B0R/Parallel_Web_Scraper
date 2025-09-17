// Project: Parallel Web Scraper
// Name of an author: Nikolić Dalibor SV13-2023
// Date and time of the last changes: 16.09.2025. 11:09

#include "UrlManager.hpp"
#include <tbb/concurrent_vector.h>
#include <tbb/parallel_for.h>
#include <fstream>
#include <iostream>
#include <regex>
#include <mutex>

std::mutex mu;

void UrlManager::addUrl(const std::string& url) {
    // mark visited & push only if not existed
    std::lock_guard<std::mutex> lock(mu);
    static const std::regex urlRe(R"(^https?://[^\s/$.?#].[^\s]*$)");
    if (!std::regex_match(url, urlRe)) {
        std::cerr << "[UrlManager] Invalid URL: " << url << "\n";
        return;
    }
    if (visited.insert(url).second) {
        urls.push_back(url);
    }
}

size_t UrlManager::loadFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) return 0;
    size_t count = 0;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        addUrl(line);
        ++count;
    }
    return count;
}

void UrlManager::loadFromConsole() {
    std::string line;
    std::cout << "Enter URLs (one per line). Empty line to finish:\n";
    while (true) {
        std::getline(std::cin, line);
        if (line.empty()) break;
        addUrl(line);
    }
}

std::vector<std::string> UrlManager::getUrlsSnapshot() const {
    // copy to std::vector for iteration
    std::vector<std::string> snap;
    snap.reserve(urls.size());
    for (const auto& u : urls) snap.push_back(u);
    return snap;
}

bool UrlManager::markVisitedIfNew(const std::string& url) {
    return visited.insert(url).second;
}

size_t UrlManager::uniqueCount() const {
    return visited.size();
}

std::vector<std::string> UrlManager::crawlIndex(Downloader& downloader, const std::string& baseUrl, int maxPages) const {
    // baseUrl: "https://books.toscrape.com/catalogue/"
    tbb::concurrent_vector<std::string> results;

    // first starts from page-1
    results.push_back(baseUrl + "page-1.html");

    // parallel searching for next page, starting from page-1
    tbb::parallel_for(2, maxPages + 1, [&](int i) {
        std::string url = baseUrl + "page-" + std::to_string(i) + ".html";
        std::string html = downloader.downloadPage(url);
        if (!html.empty()) {
            results.push_back(url);
        }
        });

    std::vector<std::string> out(results.begin(), results.end());
    std::sort(out.begin(), out.end());
    return out;
}