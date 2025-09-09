#pragma once
#include <string>

struct AnalysisResult {
    int fiveStarBooks = 0;
    double totalPrice = 0.0;
    int bookCount = 0;

    // additional metrics
    int priceOver50 = 0;
    int containsPoem = 0;
    double maxPrice = 0.0;
    std::string maxPriceTitle;

    // helper
    void mergeFrom(const AnalysisResult& other);
};

class Analyzer {
public:
    Analyzer() = default;
    // analyze html content and return AnalysisResult
    AnalysisResult analyzePage(const std::string& html);
};

std::string decodeHtmlEntities(const std::string& text);