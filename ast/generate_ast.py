#!/usr/bin/env python3

# run in the `ast` directory:
#   rm -f Expr.hpp && ./generate_ast.py ExprAst.json . && less Expr.hpp

from sys import argv
import json
from pathlib import Path

def emit(classes, output_dir):
    # TODO: Read the JSON and emit C++ structs to `output dir/<baseClass>.hpp`

    base_class = classes["baseClass"]
    subclass_names = classes["subClasses"].keys()

    output_dir = Path(output_dir)
    assert output_dir.exists() and output_dir.is_dir(), f"Output dir is not valid directory: {output_dir}"

    output_file = output_dir / f"{base_class}.hpp"
    assert not output_file.exists(), f"Output file already exists: {output_file}"

    includes_snippet = make_includes_snippet()
    subclasses_snippet = make_subclasses_snippet(classes["subClasses"].items())
    variant_snippet = make_variant_snippet(base_class, subclass_names)
    visitor_snippet = make_visitor_snippet(base_class, subclass_names)

    full_class = (
      includes_snippet + "\n" +
      "namespace ast {\n" +
      "".join([subclasses_snippet, variant_snippet, visitor_snippet]) +
      "}\n"
    )

    with open(output_file, "x") as writer:
      writer.write(full_class)

def make_includes_snippet():
  # We have a known set of includes
  return """
#include <string>
#include <variant>
"""

def make_visitor_snippet(base_class, subclass_names):
  camel_base_class = to_camel_case(base_class)

  top = f"""
class Visitor {{
public:
  virtual ~Visitor() =default;

"""

  subclass_methods = []
  for c in subclass_names:
    camel_c = to_camel_case(c)
    subclass_methods.append(f"  virtual void visit{c}({c} &{camel_c}) =0;")
    subclass_methods.append(f"  void operator()({c} &{camel_c}) {{ visit{c}({camel_c}); }}\n")

  visit_method = f"""
  void
  visit({base_class} &{camel_base_class})
  {{
    std::visit(*this, {camel_base_class});
  }}
"""

  return top + "\n".join(subclass_methods) + visit_method + "\n};\n"

def make_variant_snippet(base_class, subclasses):
  return f"""
using {base_class} = std::variant<{", ".join(subclasses)}>;
"""

def make_subclasses_snippet(subclasses):
  snippets = []

  for c, o in subclasses:
    top = f"struct {c} {{\n"
    enum_definition = o.get("enumDefinition") # might be None
    fields = "\n".join(map(lambda f: f"  {f};", o["fields"]))
    snippets.append(
      top +
      (f"  {enum_definition}\n" if enum_definition else "") +
      fields +
      "\n};\n"
    )

  return "\n" + "\n".join(snippets)

def to_camel_case(pascal_case_word):
  if len(pascal_case_word) > 0 and pascal_case_word[0].isupper():
    return pascal_case_word[0].lower() + pascal_case_word[1:]
  else:
    return pascal_case_word

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
