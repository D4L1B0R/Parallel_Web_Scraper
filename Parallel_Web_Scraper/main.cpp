// Project: Parallel Web Scraper
// Name of an author: Nikolić Dalibor SV13-2023
// Date and time of the last changes: 16.09.2025. 13:02

#include "Downloader.hpp"
#include "Analyzer.hpp"
#include "Storage.hpp"
#include "UrlManager.hpp"
#include "Common.hpp"

#include <tbb/tbb.h>
#include <tbb/parallel_pipeline.h>
#include <tbb/global_control.h>

#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <atomic>
#include <memory>

// ------------------ Result helper -------------------
struct Result {
    int pages;
    double seconds;
    double throughput;
    AnalysisResult result;
};

// ------------------ Serial run -------------------
Result runSerial(const std::vector<std::string>& urls,
    Downloader& downloader,
    Analyzer& analyzer,
    Storage& storage,
    std::ostream& out) {
    auto start = std::chrono::steady_clock::now();

    for (const auto& url : urls) {
        try {
            std::string html = downloader.downloadPage(url);
            if (html.empty()) {
                std::cerr << "[serial] Failed to download: " << url << "\n";
                continue;
            }
            else {
                std::cout << "[serial] Downloaded " << url
                    << " (length=" << html.size() << ")\n";
            }
            auto pr = analyzer.parsePageRecords(html);
            storage.storeResult(pr.second);
            storage.storeRecords(pr.first);
            storage.incrementPagesProcessed();
        }
        catch (const std::exception& ex) {
            std::cerr << "[serial] Exception: " << ex.what()
                << " for " << url << "\n";
        }
    }

    auto end = std::chrono::steady_clock::now();
    double seconds = std::chrono::duration<double>(end - start).count();
    int pages = storage.pagesProcessed();
    AnalysisResult total = storage.getAggregatedResult();
    double avgPrice = (total.bookCount ? total.totalPrice / total.bookCount : 0.0);
    double throughput = (seconds > 0.0 ? pages / seconds : pages);

    out << "\nSerial Web Scraper Results\n";
    out << "==========================\n";
    out << "Pages downloaded: " << pages << "\n";
    out << "Unique URLs (visited): " << urls.size() << "\n";
    out << "Elapsed time (s): " << seconds << "\n";
    out << "Throughput (pages/sec): " << throughput << " pages/s\n\n";
    out << "Analysis summary (aggregated):\n";
    out << "Total books found (aggregate count): " << total.bookCount << "\n";
    out << "Number of 5-star books: " << total.fiveStarBooks << "\n";
    out << "Average price: " << avgPrice << "\n";
    out << "Books with price greater than 50 pounds: " << total.priceOver50 << "\n";
    out << "Books containing 'Poem' keyword: " << total.containsPoem << "\n";
    out << "Most expensive book: " << total.maxPriceTitle
        << " (£" << total.maxPrice << ")\n";

    return { pages, seconds, throughput, total };
}

// ------------------ Parallel pipeline run -------------------
Result runPipeline(const std::vector<std::string>& urls,
    Downloader& downloader,
    Analyzer& analyzer,
    Storage& storage,
    std::ostream& out,
    size_t maxTokens)
{
    auto start = std::chrono::steady_clock::now();

#pragma intel advisor begin ParallelPipeline

    tbb::parallel_pipeline(
        maxTokens,
        tbb::make_filter<void, std::string>(
            tbb::filter_mode::serial_in_order,
            [&urls](tbb::flow_control& fc) -> std::string {
                static size_t idx = 0;
                if (idx >= urls.size()) {
                    fc.stop();
                    return {};
                }
                return urls[idx++];
            })
        &
        tbb::make_filter<std::string, std::string>(
            tbb::filter_mode::parallel,
            [&downloader](const std::string& url) -> std::string {
                return downloader.downloadPage(url);
            })
        &
        tbb::make_filter<std::string, std::pair<std::vector<BookRecord>, AnalysisResult>>(
            tbb::filter_mode::parallel,
            [&analyzer](const std::string& page) {
                if (page.empty())
                    return std::pair<std::vector<BookRecord>, AnalysisResult>{};
                return analyzer.parsePageRecords(page);
            })
        &
        tbb::make_filter<std::pair<std::vector<BookRecord>, AnalysisResult>, void>(
            tbb::filter_mode::parallel,
            [&storage](const auto& pr) {
                if (pr.first.empty() && pr.second.bookCount == 0) return;
                storage.storeResult(pr.second);
                storage.storeRecords(pr.first);
                storage.incrementPagesProcessed();
            })
    );

#pragma intel advisor end ParallelPipeline

    auto end = std::chrono::steady_clock::now();
    double seconds = std::chrono::duration<double>(end - start).count();
    int pages = storage.pagesProcessed();

    AnalysisResult total = storage.getAggregatedResult();
    double avgPrice = (total.bookCount ? total.totalPrice / total.bookCount : 0.0);
    double throughput = (seconds > 0.0 ? pages / seconds : pages);

    out << "Parallel Pipeline Results\n";
    out << "============================\n";
    out << "Pages downloaded: " << pages << "\n";
    out << "Unique URLs (visited): " << urls.size() << "\n";
    out << "Elapsed time (s): " << seconds << "\n";
    out << "Throughput (pages/sec): " << throughput << " pages/s\n\n";
    out << "Analysis summary (aggregated):\n";
    out << "Total books found (aggregate count): " << total.bookCount << "\n";
    out << "Number of 5-star books: " << total.fiveStarBooks << "\n";
    out << "Average price: " << avgPrice << "\n";
    out << "Books with price greater than 50 pounds: " << total.priceOver50 << "\n";
    out << "Books containing 'Poem' keyword: " << total.containsPoem << "\n";
    out << "Most expensive book: " << total.maxPriceTitle
        << " (£" << total.maxPrice << ")\n";

    return { pages, seconds, throughput, total };
}

// ------------------ Main -------------------
int main(int argc, char** argv) {

    curl_global_init(CURL_GLOBAL_DEFAULT);

    int threads = 0;
    bool doCrawl = false;
    int pagesCrawl = 0;
    for (int i = 1; i < argc; ++i) {
        std::string a(argv[i]);
        if ((a == "-t" || a == "--threads") && i + 1 < argc) {
            threads = std::stoi(argv[++i]);
        }
        if ((a == "--c" || a == "--crawl") && i + 1 < argc) {
            doCrawl = true;
            pagesCrawl = std::stoi(argv[++i]);
        }
    }

    UrlManager urlManager;
    size_t loaded = urlManager.loadFromFile("urls.txt");
    if (loaded == 0) {
        std::cout << "No urls.txt or file empty. You can enter URLs manually.\n";
        urlManager.loadFromConsole();
    }

    Downloader downloader(10, 3);
    Analyzer analyzer;
    Storage storage;

    if (doCrawl) {
        std::cout << "[main] Crawling catalogue pages...\n";
        auto pages = urlManager.crawlIndex(downloader, "https://books.toscrape.com/catalogue/", pagesCrawl);
        for (auto& p : pages) urlManager.addUrl(p);
        std::cout << "[main] Crawl found " << pages.size() << " catalogue pages.\n";
    }

    auto urls = urlManager.getUrlsSnapshot();
    if (urls.empty()) {
        std::cerr << "No URLs to process.\n";
        return 1;
    }

    std::unique_ptr<tbb::global_control> gc;
    if (threads > 0) {
        gc.reset(new tbb::global_control(tbb::global_control::max_allowed_parallelism, threads));
        std::cout << "[main] Using " << threads << " threads.\n";
    }

    std::ofstream out("results.txt");

    out << "Processed URLs:\n";
    for (const auto& u : urls) {
        out << u << "\n";
    }
    out << "\n============================\n\n";

    // Parallel run (pipeline)
    storage.reset();
    std::cout << "Starting parallel pipeline run...\n";
    Result parallel = runPipeline(urls, downloader, analyzer, storage, out,
        threads > 0 ? threads : std::thread::hardware_concurrency());

    // Serial run
    storage.reset();
    std::cout << "Starting serial run...\n";
    Result serial = runSerial(urls, downloader, analyzer, storage, out);

    out.close();

    std::cout << "\nParallel pipeline completed. Pages: " << parallel.pages
        << ", elapsed: " << parallel.seconds
        << " s, throughput: " << parallel.throughput << " pages/s\n";

    std::cout << "\nSerial completed. Pages: " << serial.pages
        << ", elapsed: " << serial.seconds
        << " s, throughput: " << serial.throughput << " pages/s\n";

    // Export CSV with all books
    auto allBooks = storage.snapshotRecords();
    std::ofstream csv("books.csv");
    csv << "title,price,rating\n";
    for (auto& b : allBooks) {
        std::string t = b.title;
        size_t p = 0;
        while ((p = t.find('"', p)) != std::string::npos) {
            t.replace(p, 1, "\"\"");
            p += 2;
        }
        csv << "\"" << t << "\"," << b.price << "," << b.rating << "\n";
    }
    csv.close();

    if (parallel.result.bookCount != serial.result.bookCount ||
        parallel.result.fiveStarBooks != serial.result.fiveStarBooks ||
        parallel.result.containsPoem != serial.result.containsPoem ||
        std::abs(parallel.result.totalPrice - serial.result.totalPrice) > 1e-6 ||
        parallel.result.priceOver50 != serial.result.priceOver50)
    {
        std::cerr << "[main] Warning: Results differ between pipeline and serial!\n";
    }
    else {
        std::cout << "[main] Results are consistent (pipeline == serial).\n";
    }

    double speedup = serial.seconds / (parallel.seconds > 0 ? parallel.seconds : 1);
    double efficiency = (threads > 0 ? speedup / threads : speedup);

    std::cout << "\nPerformance summary:\n";
    std::cout << "----------------------\n";
    std::cout << "Speedup: " << speedup << "x\n";
    if (threads > 0) {
        std::cout << "Efficiency: " << (efficiency * 100.0) << "% (relative to " << threads << " threads)\n";
    }

    curl_global_cleanup();
    return 0;
}