// Project: Parallel Web Scraper
// Name of an author: Nikolić Dalibor SV13-2023
// Date and time of the last changes: 16.09.2025. 10:09

#pragma once
#include <string>

struct BookRecord {
    std::string title;
    double price = 0.0;
    int rating = 0;
};