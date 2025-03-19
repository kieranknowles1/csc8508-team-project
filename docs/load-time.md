# Load Times

Tests performed with the following:

Methodology: Basic time measurement was added, and each test was repeated 3
times. Nowhere near scientific.

Home PC:
- Clang++ 19.1.7
- Linux 
- Debug build
- Ryzen 7 5800X CPU, 8 cores, 16 threads

| Before Threading | 12 Threads |
| ---------------- | ---------- |
| 418ms            | 52ms       |
| 424ms            | 52ms       |
| 424ms            | 52ms       |
| Average          |            |
| 422ms            | 52ms       |

Platform:
- MSVC
- Windows 10
- Debug Build
- i5-8600K CPU, 6 cores, 6 threads

| Before Threading | 6 Threads |
| ---------------- | --------- |
| 2387ms           | 995ms     |
| 2305ms           | 1021ms    |
| 2309ms           | 825ms     |
|                  | 809ms     |
| Average          |           |
| 2333ms           | 912ms     |


2387
2305
2309
