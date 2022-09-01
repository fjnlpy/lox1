#!/usr/bin/env python3

# run in the `ast` directory:
#   rm -f Expr.hpp && ./generate_ast.py ExprAst.json . && less Expr.hpp

from sys import argv
import json
from pathlib import Path

def resolve(classes):
  # Only thing we need to do is merge the common children with the subclass-specific ones.
  common_children = classes["commonChildren"]
  for _, c in classes["subClasses"].items():
    print(c["children"])
    c["children"] += common_children
    print(c["children"])

def emit(classes, output_dir):
    base_class = classes["baseClass"]
    subclass_names = classes["subClasses"].keys()

    output_dir = Path(output_dir)
    assert output_dir.exists() and output_dir.is_dir(), f"Output dir is not valid directory: {output_dir}"

    output_file = output_dir / f"{base_class}.hpp"
    assert not output_file.exists(), f"Output file already exists: {output_file}"

    forward_decls_snippet = make_forward_decls_snippet(subclass_names) # needed because variant refers to the subclasses
    includes_snippet = make_includes_snippet()
    variant_snippet = make_variant_snippet(base_class, subclass_names)
    subclasses_snippet = make_subclasses_snippet(classes["subClasses"].items())
    visitor_snippet = make_visitor_snippet(base_class, subclass_names)

    full_class = (
      "#pragma once\n" +
      includes_snippet + "\n" +
      "namespace ast {\n" +
      "".join([forward_decls_snippet, variant_snippet, subclasses_snippet, visitor_snippet]) +
      "}\n"
    )

    with open(output_file, "x") as writer:
      writer.write(full_class)

def make_includes_snippet():
  # We have a known set of includes
  return """
#include <string>
#include <variant>
#include <memory>
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
    subclass_methods.append(f"  void operator()(std::unique_ptr<{c}> &{camel_c}) {{ visit{c}(*{camel_c}); }}\n")

  visit_method = f"""
  void
  visit({base_class} &{camel_base_class})
  {{
    std::visit(*this, {camel_base_class});
  }}
"""

  return top + "\n".join(subclass_methods) + visit_method + "\n};\n"

def make_variant_snippet(base_class, subclasses):
  top =  f"""
using {base_class} = std::variant<
"""
  lines = ",\n".join(map(lambda s: f"  std::unique_ptr<{s}>", subclasses))
  return top + lines + "\n>;\n"

def make_forward_decls_snippet(subclass_names):
  comment_about_decls ="""
// Forward declarations are needed because the superclass and subclasses mutually refer to each other.
"""
  return comment_about_decls + "\n".join(map(lambda s: f"struct {s};", subclass_names)) + "\n"

def make_subclasses_snippet(subclasses):
  def split_child(child):
    split = child.split(" ")
    assert len(split) == 2, f"Expected exactly two parts after splitting child on space: {split}"
    return split

  # Accessors are added because they might be useful later for changing the internal representation
  # without breaking clients too badly.
  def make_accessor(child):
    return f"  {child[0]} &{child[1]}() {{ return {child[1]}_; }}"

  def make_constructor(name, arguments):
    argument_list = ",\n    ".join(map(lambda a: f"{a[0]} {a[1]}", arguments))
    initializers = ",\n    ".join(map(lambda a: f"{a[1]}_(std::move({a[1]}))", arguments))
    return f"  {name}(\n    {argument_list}\n  ): {initializers} {{ }}"

  snippets = []

  for c, o in subclasses:
    # Split child definitions into a pair of type and name, since we often
    # need each piece of data separately.
    split_children = list(map(split_child, o["children"]))

    enum_definition =  "  " + o.get("enumDefinition") + "\n" if o.get("enumDefinition") else ""
    constructor = make_constructor(c, split_children)
    fields = "\n".join(map(lambda c: f"  {c[0]} {c[1]}_;", split_children))
    accessors = "\n".join(map(make_accessor, split_children))

    snippets.append(f"""
class {c} {{
public:
{enum_definition}
{constructor}

{accessors}

private:
{fields}
}};
""")

    # snippets.append(
    #   top +
    #   (f"  {enum_definition}\n" if enum_definition else "") +
    #   constructor +
    #   accessors + 
    #   "\n\nprivate:\n" +
    #   fields +
    #   "\n};\n"
    # ) # TODO: make this use a string literal? It's a bit of a mess right now.

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

  resolve(classes)
  emit(classes, output_dir)

if __name__ == "__main__":
  main()
