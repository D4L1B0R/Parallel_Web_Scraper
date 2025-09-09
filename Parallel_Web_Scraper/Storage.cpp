#include "Storage.hpp"

void Storage::storeResult(const AnalysisResult& result) {
    results.push_back(result);
}

AnalysisResult Storage::getAggregatedResult() const {
    AnalysisResult total;

    std::lock_guard<std::mutex> lock(m);
    for (const auto& r : results) {
        total.mergeFrom(r);
    }
    return total;
}

void Storage::incrementPagesProcessed() {
    pages.fetch_add(1, std::memory_order_relaxed);
}

int Storage::pagesProcessed() const {
    return pages.load(std::memory_order_relaxed);
}