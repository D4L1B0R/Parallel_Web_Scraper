#pragma once
#include <string>

class Downloader {
public:
    Downloader(int timeout_seconds = 10, int max_retries = 3);
    // download page content; returns empty string on permanent failure
    std::string downloadPage(const std::string& url);
private:
    int timeoutSec;
    int maxRetries;
};