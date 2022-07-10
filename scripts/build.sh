#!/usr/bin/env bash

# Invoke from repo root.

clean=
run=

if (( $# > 0)); then
  case "$1" in
    "cr" | "rc")
      clean="true"
      run="true"
    ;;

    "r")
      run="true"
    ;;

    "c")
      clean="true"
    ;;
  esac
fi

[[ "$clean" == "true" ]] && rm -r build

mkdir -p build &&
cd build &&
CXX=/usr/bin/clang++ cmake ../ &&
make &&

[[ "$run" == "true" ]] && ./Lox1
