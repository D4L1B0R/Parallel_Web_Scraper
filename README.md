Parallel Web Data Scraper Using Intel TBB
Course: Parallel Programming, SIIT 2025
Language: C++
Project Type: Console-based parallel web scraper

📌 Overview
This project implements a parallel web data scraper using Intel Threading Building Blocks (TBB). The program efficiently downloads and analyzes web pages concurrently, storing results in thread-safe data structures. The scraper is optimized for multicore processors and supports scalable parallel execution. The project includes:

Task-based parallel page downloading

Concurrent HTML content analysis

Thread-safe storage of URLs, metadata, and text content

Configurable parallelism and performance statistics

🧩 Features

Simultaneous downloading of pre-indexed pages from https://books.toscrape.com/index.html

Timeout and retry logic for reliable HTTP/HTTPS requests

Parallel extraction of page data (book ratings, prices, and custom metrics)

Thread-safe tracking of visited URLs and extracted content

Performance metrics: number of pages downloaded, unique URLs, throughput

Output of results to a file

Optional extra features for bonus points:

Automatic parallel site indexing

Parallel pipeline chaining of download → analysis → storage

🛠 Technologies Used

C++17/20

Intel Threading Building Blocks (TBB)

Concurrent containers (e.g., concurrent_vector, concurrent_unordered_map)

HTTP client libraries (e.g., libcurl)

Console output and file I/O
