// Project: Parallel Web Scraper
// Name of an author: Nikolić Dalibor SV13-2023
// Date and time of the last changes: 16.09.2025. 10:09

#pragma once
#include <string>
#include <vector>
#include "Common.hpp"

struct AnalysisResult {
    int fiveStarBooks = 0;
    double totalPrice = 0.0;
    int bookCount = 0;
    int priceOver50 = 0;
    int containsPoem = 0;
    double maxPrice = 0.0;
    std::string maxPriceTitle;
    void mergeFrom(const AnalysisResult& other);
};

class Analyzer {
public:
    Analyzer() = default;
    // parse page and return list of BookRecord plus aggregated AnalysisResult
    std::pair<std::vector<BookRecord>, AnalysisResult> parsePageRecords(const std::string& html);
};

std::string decodeHtmlEntities(const std::string& text);