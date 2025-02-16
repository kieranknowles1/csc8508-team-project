#!/usr/bin/env bash
set -euo pipefail

cmake -DCMAKE_BUILD_TYPE=Debug build
make --directory build -j12

# Suppress errors from certain libraries that we don't have control over
valgrind --suppressions=valgrind-suppress.supp --leak-check=full ./build/TeamProject/CSC8508

# Extra aggressive options
# valgrind --leak-check=full --show-leak-kinds=all ./build/TeamProject/CSC8508
