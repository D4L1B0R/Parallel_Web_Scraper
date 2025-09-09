#include "UrlManager.hpp"
#include "Downloader.hpp"
#include "Analyzer.hpp"
#include "Storage.hpp"
#include <tbb/task_group.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <string>

int main(int argc, char** argv) {
    UrlManager urlManager;

    // 1) Load initial URLs
    size_t loaded = urlManager.loadFromFile("urls.txt");
    if (loaded == 0) {
        std::cout << "No urls.txt or file empty. You can enter URLs manually.\n";
        urlManager.loadFromConsole();
    }

    Downloader downloader(10, 3);
    Analyzer analyzer;
    Storage storage;

    auto urls = urlManager.getUrlsSnapshot(); // snapshot svih URL-ova

    auto start = std::chrono::steady_clock::now();

    tbb::task_group tg;
    for (auto url : urls) { // **capture by value!**
        tg.run([url, &downloader, &analyzer, &storage]() {
            try {
                std::string html = downloader.downloadPage(url);
                if (html.empty()) {
                    std::cerr << "[main] Failed to download: " << url << "\n";
                    return;
                }
                else {
                    std::cout << "[main] Downloaded " << url
                        << " (length=" << html.size() << ")\n";
                }

                AnalysisResult r = analyzer.analyzePage(html);
                storage.storeResult(r);
                storage.incrementPagesProcessed();
            }
            catch (const std::exception& ex) {
                std::cerr << "[main] Exception: " << ex.what() << " for " << url << "\n";
            }
            });
    }
    tg.wait();

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    double seconds = elapsed.count();
    int pages = storage.pagesProcessed();
    AnalysisResult total = storage.getAggregatedResult();
    int uniqueUrls = static_cast<int>(urls.size());

    double avgPrice = (total.bookCount ? total.totalPrice / total.bookCount : 0.0);
    double throughput = (seconds > 0.0 ? pages / seconds : pages);

    std::ofstream out("results.txt");
    out << "Parallel Web Scraper Results\n";
    out << "============================\n";
    out << "Pages downloaded: " << pages << "\n";
    out << "Unique URLs (visited): " << uniqueUrls << "\n";
    out << "Elapsed time (s): " << seconds << "\n";
    out << "Throughput (pages/sec): " << throughput << " pages/s\n\n";

    out << "Analysis summary (aggregated):\n";
    out << "Total books found (aggregate count): " << total.bookCount << "\n";
    out << "Number of 5-star books: " << total.fiveStarBooks << "\n";
    out << "Average price: " << avgPrice << "\n";
    out << "Books with price greater than 50 pounds: " << total.priceOver50 << "\n";
    out << "Books containing 'Poem' keyword: " << total.containsPoem << "\n";
    out << "Most expensive book: " << total.maxPriceTitle << " (£" << total.maxPrice << ")\n";
    out.close();

    std::cout << "Completed. Results saved to results.txt\n";
    std::cout << "Pages downloaded: " << pages
        << ", elapsed: " << seconds
        << " s, throughput: " << throughput << " pages/s\n";

    return 0;
}