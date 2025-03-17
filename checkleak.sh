#!/usr/bin/env bash
set -euo pipefail

TARGET=CSC8508-1

cmake -DCMAKE_BUILD_TYPE=Debug build
make --directory build -j12 $TARGET

valgrind --suppressions=valgrind-ignore.supp --leak-check=full ./build/TeamProject/$TARGET

# Extra aggressive options
# valgrind --leak-check=full --show-leak-kinds=all ./build/TeamProject/$TARGET
