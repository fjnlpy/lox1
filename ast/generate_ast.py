#!/usr/bin/env python3

from sys import argv

def main():
  if len(argv) != 3:
    print(f"Usage: {argv[0]} <ast-file> <output-dir>")
    exit(-1)

  # TODO: Open file and try to parse as JSON
  # TODO: Read the JSON and emit C++ structs to `output dir/<baseClass>.hpp`

if __name__ == "__main__":
  main()
