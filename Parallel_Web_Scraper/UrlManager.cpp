// Project: Parallel Web Scraper
// Name of an author: Nikolić Dalibor SV13-2023
// Date and time of the last changes: 16.09.2025. 09:09

#include "UrlManager.hpp"
#include <fstream>
#include <iostream>
#include <regex>

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

std::vector<std::string> UrlManager::crawlIndex(const std::string& indexHtml, const std::string& baseUrl) const {
    // baseUrl like "https://books.toscrape.com/catalogue/"
    std::vector<std::string> out;
    // find links to /catalogue/page-N.html or similar
    std::string htmlOne = indexHtml;
    std::replace(htmlOne.begin(), htmlOne.end(), '\n', ' ');
    std::regex linkRe("(?:href|HREF)\\s*=\\s*\"(/catalogue/page-[0-9]+\\.html)\"");
        std::sregex_iterator it(htmlOne.begin(), htmlOne.end(), linkRe), end;
    std::unordered_set<std::string> uniq;
    for (; it != end; ++it) {
        std::string rel = (*it)[1].str(); // like /catalogue/page-2.html
        // build absolute
        std::string abs = "https://books.toscrape.com" + rel;
        if (uniq.insert(abs).second) out.push_back(abs);
    }
    // sort to deterministic order
    std::sort(out.begin(), out.end());
    return out;
}