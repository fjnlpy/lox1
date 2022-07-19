#!/usr/bin/env bash

# Invoke me from repo root!

clean=
run=
test=

while getopts "crt" opt; do
  case "$opt" in
    c)  clean="true"
      ;;
    r)  run="true"
      ;;
    t)  test="true"
      ;;
  esac
done

mkdir -p build &&
cd build &&
CXX=/usr/bin/clang++ cmake ../ || exit -1

make_flags=""
[[ "$clean" == "true" ]] && make_flags+=" clean "
make_flags+=" Lox1 "
[[ "$run" == "true" ]] && make_flags+=" main "
[[ "$test" == "true" ]] && make_flags+=" tests "
make $make_flags || exit -1

[[ "$test" == "true" ]] && ./tests

[[ "$run" == "true" ]] &&
echo "" &&
echo "=== RUNNING MAIN ===" &&
cd ../ && # Make relative paths match initial working dir.
shift &&
./build/main "$@"
