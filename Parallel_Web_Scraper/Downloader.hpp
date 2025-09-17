// Project: Parallel Web Scraper
// Name of an author: Nikolić Dalibor SV13-2023
// Date and time of the last changes: 16.09.2025. 10:09

#pragma once
#include <string>

class Downloader {
    int timeoutSec;
    int maxRetries;
public:
    Downloader(int timeout_seconds = 10, int max_retries = 3);
    // download page content; returns empty string on permanent failure
    std::string downloadPage(const std::string& url);
};