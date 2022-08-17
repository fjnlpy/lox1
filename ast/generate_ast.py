#!/usr/bin/env python3

from sys import argv
import json
from pathlib import Path

def emit(classes, output_dir):
    # TODO: Read the JSON and emit C++ structs to `output dir/<baseClass>.hpp`

    base_class = classes["baseClass"]

    output_dir = Path(output_dir)
    assert output_dir.exists() and output_dir.is_dir(), f"Output dir is not valid directory: {output_dir}"

    output_file = output_dir / f"{base_class}.hpp"
    assert not output_file.exists(), f"Output file already exists: {output_file}"

    includes_snippet = make_includes_snippet()
    base_snippet = make_base_class_snippet(base_class)

    full_class = "".join([includes_snippet, base_snippet]) + "\n"

    with open(output_file, "x") as writer:
      writer.write(full_class)

def make_includes_snippet():
  # We have a known set of includes
  return """
  #include <string>
  """

def make_base_class_snippet(base_class_name):
  return """
  // TODO: emit base class.
  """

def main():
  if len(argv) != 3:
    raise Exception(f"Usage: {argv[0]} <ast-file> <output-dir>")

  ast_file = argv[1]
  output_dir = argv[2]

  with open(ast_file) as f:
    classes = json.loads("".join(f.readlines()))
  emit(classes, output_dir)

if __name__ == "__main__":
  main()
