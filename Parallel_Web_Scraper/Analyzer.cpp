#include "Analyzer.hpp"
#include <regex>
#include <algorithm>
#include <unordered_map>
#include <sstream>
#include <iostream>

void AnalysisResult::mergeFrom(const AnalysisResult& other) {
    fiveStarBooks += other.fiveStarBooks;
    totalPrice += other.totalPrice;
    bookCount += other.bookCount;
    priceOver50 += other.priceOver50;
    containsPoem += other.containsPoem;
    if (other.maxPrice > maxPrice) {
        maxPrice = other.maxPrice;
        maxPriceTitle = other.maxPriceTitle;
    }
}

static int ratingStringToInt(const std::string& r) {
    if (r == "One") return 1;
    if (r == "Two") return 2;
    if (r == "Three") return 3;
    if (r == "Four") return 4;
    if (r == "Five") return 5;
    return 0;
}

static std::string decodeHtmlEntities(const std::string& text) {
    std::string result = text;
    static const std::unordered_map<std::string, char> entities = {
        {"&quot;", '"'}, {"&apos;", '\''}, {"&amp;", '&'},
        {"&lt;", '<'}, {"&gt;", '>'}, {"&nbsp;", ' '},
        {"&#39;", '\''}, {"&rsquo;", '\''}, {"&ldquo;", '"'},
        {"&rdquo;", '"'}, {"&hellip;", '…'}
    };
    for (auto it = entities.begin(); it != entities.end(); ++it) {
        const std::string& entity = it->first;
        char chr = it->second;
        size_t pos = 0;
        while ((pos = result.find(entity, pos)) != std::string::npos) {
            result.replace(pos, entity.size(), 1, chr);
            ++pos;
        }
    }
    std::regex decimalEntityRe("&#([0-9]+);");
    std::smatch m;
    while (std::regex_search(result, m, decimalEntityRe)) {
        int code = std::stoi(m[1].str());
        char c = (code >= 0 && code <= 127) ? static_cast<char>(code) : '?';
        result.replace(m.position(0), m.length(0), 1, c);
    }
    return result;
}

std::pair<std::vector<BookRecord>, AnalysisResult> Analyzer::parsePageRecords(const std::string& html) {
    AnalysisResult res;
    std::vector<BookRecord> records;

    std::cout << "[Analyzer] parsing HTML length=" << html.size() << "\n";

    std::string htmlOneLine = html;
    std::replace(htmlOneLine.begin(), htmlOneLine.end(), '\n', ' ');
    // dotall not available portably, we removed newlines above
    std::regex articleRe(R"(<article[^>]*class="[^"]*product_pod[^"]*"[^>]*>.*?</article>)", std::regex::icase);
    std::sregex_iterator it(htmlOneLine.begin(), htmlOneLine.end(), articleRe);
    std::sregex_iterator end;

    std::regex titleRe("title=\"([^\"]+)\"", std::regex::icase);
        std::regex priceRe(R"(£([0-9]+(?:\.[0-9]{2})))", std::regex::icase);
    std::regex ratingRe(R"(star-rating\s+([A-Za-z]+))", std::regex::icase);

    for (; it != end; ++it) {
        std::string block = it->str();
        std::smatch m;

        BookRecord br;
        br.title = "UNKNOWN";
        if (std::regex_search(block, m, titleRe)) {
            br.title = decodeHtmlEntities(m[1].str());
        }

        double price = 0.0;
        if (std::regex_search(block, m, priceRe)) {
            try { price = std::stod(m[1].str()); }
            catch (...) { price = 0.0; }
        }
        br.price = price;

        int rating = 0;
        if (std::regex_search(block, m, ratingRe)) {
            rating = ratingStringToInt(m[1].str());
        }
        br.rating = rating;

        auto blockLower = block;
        std::transform(blockLower.begin(), blockLower.end(), blockLower.begin(), ::tolower);
        bool hasPoem = (blockLower.find("poem") != std::string::npos);

        // aggregate
        res.bookCount += 1;
        res.totalPrice += price;
        if (rating == 5) res.fiveStarBooks += 1;
        if (price > 50.0) res.priceOver50 += 1;
        if (hasPoem) res.containsPoem += 1;
        if (price > res.maxPrice) {
            res.maxPrice = price;
            res.maxPriceTitle = br.title;
        }

        records.push_back(std::move(br));
        std::cout << "[Analyzer] Book: " << records.back().title << " £" << price << " rating=" << rating << "\n";
    }

    return { std::move(records), res };
}