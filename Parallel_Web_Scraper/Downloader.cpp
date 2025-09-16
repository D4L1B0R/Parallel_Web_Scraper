// Project: Parallel Web Scraper
// Name of an author: Nikolić Dalibor SV13-2023
// Date and time of the last changes: 16.09.2025. 10:11

#include "Downloader.hpp"
#include <curl/curl.h>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <iostream>

Downloader::Downloader(int timeout_seconds, int max_retries)
    : timeoutSec(timeout_seconds), maxRetries(max_retries) {
}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string Downloader::downloadPage(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "[Downloader] curl_easy_init failed\n";
        return "";
    }

    std::string buffer;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, static_cast<long>(timeoutSec));
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "ParallelWebScraper/1.0");

    for (int attempt = 1; attempt <= maxRetries; ++attempt) {
        buffer.clear();
        CURLcode res = curl_easy_perform(curl);

        long response_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        if (res == CURLE_OK && response_code >= 200 && response_code < 400) {
            curl_easy_cleanup(curl);
			return buffer; // successfull download
        }
        else if (response_code == 429) {
            char* retry_after = nullptr;
            curl_easy_getinfo(curl, CURLINFO_RETRY_AFTER, &retry_after);
            int wait = retry_after ? atoi(retry_after) : (1 << (attempt - 1));
            std::this_thread::sleep_for(std::chrono::seconds(wait));
            continue;
        }
        else if (response_code >= 400 && response_code < 500) { // client error — don't retry
            break;
        }

        std::cerr << "[Downloader] attempt " << attempt
            << " failed for " << url
            << " (curl=" << curl_easy_strerror(res)
            << ", http=" << response_code << ")\n";

        // exponencial backoff: 200ms * 2^(attempt-1)
        std::this_thread::sleep_for(
            std::chrono::milliseconds(200 * (1 << (attempt - 1))));
    }

    curl_easy_cleanup(curl);
    std::cerr << "[Downloader] failed to download URL after " << maxRetries
        << " attempts: " << url << "\n";
    return "";
}