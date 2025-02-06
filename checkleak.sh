#!/usr/bin/env bash
set -euo pipefail

cmake -DCMAKE_BUILD_TYPE=Debug build
make --directory build -j12
valgrind --leak-check=full --show-leak-kinds=all ./build/TeamProject/CSC8508
