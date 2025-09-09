#pragma once
#include "Analyzer.hpp"
#include <tbb/concurrent_vector.h>
#include <atomic>
#include <mutex>

class Storage {
public:
    void storeResult(const AnalysisResult& result);
    AnalysisResult getAggregatedResult() const;
    void incrementPagesProcessed();
    int pagesProcessed() const;
private:
    tbb::concurrent_vector<AnalysisResult> results;
    std::atomic<int> pages{ 0 };
    // optional mutex for safer non-atomic ops (not strictly necessary here)
    mutable std::mutex m;
};