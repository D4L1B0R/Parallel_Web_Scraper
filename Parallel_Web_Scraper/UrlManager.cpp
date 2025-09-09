#include "UrlManager.hpp"
#include <fstream>
#include <iostream>

void UrlManager::addUrl(const std::string& url) {
    // mark visited & push only if not existed
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