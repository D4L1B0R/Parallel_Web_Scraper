#pragma once
#include "Analyzer.hpp"
#include "Common.hpp"
#include <tbb/concurrent_vector.h>
#include <atomic>
#include <mutex>
#include <vector>

class Storage {
    mutable std::mutex m;
    std::vector<AnalysisResult> results;
    tbb::concurrent_vector<BookRecord> records; // thread-safe append
    std::atomic<int> pages{ 0 };
public:
    void storeResult(const AnalysisResult& result);
    void storeRecords(const std::vector<BookRecord>& records);
    AnalysisResult getAggregatedResult() const;
    int pagesProcessed() const;
    void incrementPagesProcessed();
    void reset();

    // accessor for recorded books
    std::vector<BookRecord> snapshotRecords() const;
};