#include "Storage.hpp"

void Storage::storeResult(const AnalysisResult& result) {
    std::lock_guard<std::mutex> lock(m);
    results.push_back(result);
}

void Storage::storeRecords(const std::vector<BookRecord>& recs) {
    for (const auto& r : recs) records.push_back(r); // concurrent_vector allows push_back concurrently
}

AnalysisResult Storage::getAggregatedResult() const {
    AnalysisResult total;
    std::lock_guard<std::mutex> lock(m);
    for (const auto& r : results) total.mergeFrom(r);
    return total;
}

void Storage::incrementPagesProcessed() {
    pages.fetch_add(1, std::memory_order_relaxed);
}

int Storage::pagesProcessed() const {
    return pages.load(std::memory_order_relaxed);
}

void Storage::reset() {
    std::lock_guard<std::mutex> lock(m);
    results.clear();
    pages.store(0, std::memory_order_relaxed);
    records.clear();
}

std::vector<BookRecord> Storage::snapshotRecords() const {
    std::vector<BookRecord> out;
    out.reserve(records.size());
    for (const auto& r : records) out.push_back(r);
    return out;
}