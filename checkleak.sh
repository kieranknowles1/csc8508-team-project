#!/usr/bin/env bash
set -euo pipefail

TARGET=CSC8508-1

showhelp() {
    cat <<EOF
Usage: $0 [tool=memcheck]
  -h|--help:
    Show this help message and exit
  -s|--strict:
    Exit on first error
  -S|--suppressions:
    Generate suppressions
EOF
}

opts=""
errexit=no
tool=memcheck
while [[ $# -gt 0 ]]; do
  case $1 in
    -h|--help)
      showhelp
      ;;
    -s|--strict)
      errexit=yes
      ;;
    -S|--suppressions)
      opts="$opts --gen-suppressions=yes"
      ;;
    *)
      tool=$1
      ;;
  esac
  shift
done


case $tool in
  memcheck)
    opts="$opts --leak-check=full"
    ;;
esac

cmake -DCMAKE_BUILD_TYPE=Debug build
make --directory build -j12 $TARGET

valgrind --tool="$tool" --error-exitcode=1 --exit-on-first-error=$errexit --suppressions=valgrind-ignore.supp $opts ./build/TeamProject/$TARGET

# Extra aggressive options
# valgrind --leak-check=full --show-leak-kinds=all ./build/TeamProject/$TARGET
