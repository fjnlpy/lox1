#!/usr/bin/env python3

# run in the `ast` directory:
#   rm -f Expr.hpp && ./generate_ast.py ExprAst.json . && less Expr.hpp

from sys import argv
import json
from pathlib import Path

def resolve(classes):
  # Merge the common children with the subclass-specific ones.
  common_children = classes["commonChildren"]
  for _, c in classes["subClasses"].items():
    c["children"] += common_children

  # Split the child definitions into (type,name) pairs.
  def split_child(child):
    split = child.split(" ")
    assert len(split) == 2, f"Expected exactly two parts after splitting child on space: {split}"
    return split

  for _,c in classes["subClasses"].items():
    c["children"] = [split_child(child) for child in c["children"]]

def emit(classes, output_dir):
    base_class = classes["baseClass"]
    subclass_names = classes["subClasses"].keys()

    output_dir = Path(output_dir)
    assert output_dir.exists() and output_dir.is_dir(), f"Output dir is not valid directory: {output_dir}"

    output_file = output_dir / f"{base_class}.hpp"
    assert not output_file.exists(), f"Output file already exists: {output_file}"

    forward_decls_snippet = make_forward_decls_snippet(subclass_names) # needed because variant refers to the subclasses
    includes_snippet = make_includes_snippet(classes["includesForAst"])
    variant_snippet = make_variant_snippet(base_class, subclass_names)
    subclasses_snippet = make_subclasses_snippet(classes["subClasses"].items())
    factory_funs_snippet = make_factory_funs_snippet(classes["subClasses"].items(), classes["autoProvidedDefs"])
    visitor_snippet = make_visitor_snippet(base_class, subclass_names)

    full_class = (
      "#pragma once\n" +
      includes_snippet + "\n" +
      "namespace ast {\n" +
      "".join([forward_decls_snippet, variant_snippet, subclasses_snippet, factory_funs_snippet, visitor_snippet]) +
      "}\n"
    )

    with open(output_file, "x") as writer:
      writer.write(full_class)

def make_includes_snippet(includes_for_ast):
  # We have a known set of includes based on hard-coded code generation logic,
  # plus includes used by the AST nodes themselves.
  known_includes = [ "<variant>", "<memory>", "<utility>" ]
  system_includes, project_includes = [], []
  for include in set(known_includes + includes_for_ast):
    if include[0] == "<":
      system_includes.append(include)
    else:
      project_includes.append(include)
  return (
    "\n" +
    "\n".join([f"#include {x}" for x in system_includes]) +
    "\n\n" +
    "\n".join([f"#include {x}" for x in project_includes]) +
    "\n"
  )

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
  # Accessors are added because they might be useful later for changing the internal representation
  # without breaking clients too badly.
  def make_accessor(child):
    return f"  {child[0]} &{child[1]}() {{ return {child[1]}_; }}"

  def make_constructor(name, arguments):
    argument_list = ",\n    ".join(map(lambda a: f"{a[0]} {a[1]}", arguments))
    initializers = ",\n    ".join(map(lambda a: f"{a[1]}_(std::move({a[1]}))", arguments))
    return f"  {name}(\n    {argument_list}\n  ): {initializers} {{ }}"

  def make_enum(definition):
    if definition is None:
      return ""

    name = definition["name"]
    values = ", ".join(definition["values"])
    return f"  enum class {name} {{ {values} }};\n"

  snippets = []

  for c, o in subclasses:
    enum_definition =  make_enum(o.get("enumDefinition")) # use `get` because definition may not exist
    constructor = make_constructor(c, o["children"])
    fields = "\n".join(map(lambda c: f"  {c[0]} {c[1]}_;", o["children"]))
    accessors = "\n".join(map(make_accessor, o["children"]))

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

  return "\n" + "\n".join(snippets)

def make_factory_funs_snippet(subclasses, auto_provided_defs):
  # - If a child is in the list of auto-provided children, use a lookup to find
  #   out how to populate that child, instead of expecting it as an argument.
  # - Returns a unique_ptr to the created AST node, which will implicitly convert
  #   to the base class variant type if passed to another factory function.
  # - If a subclass has an enum definition, create one function per enum value,
  #   which sets the enum to the appropriate value.

  def make_snippet(class_type, name, children, enum_name_and_value):
    arguments = ",\n".join([
      f"  {t} &&{n}" for t, n in children if (
          n not in auto_provided_defs and (enum_name_and_value is None or t != enum_name_and_value["name"])
        )
      ])
    # Argument types need to be fully specified (including qualifications on enum values).
    # The compiler can't seem to infer them from the arguments and return type.
    argument_types = ",\n".join([
      f"    {class_type}::{child[0]}" if (
        enum_name_and_value is not None and child[0] == enum_name_and_value["name"]
        ) else f"    {child[0]}"  for child in children
      ])
    forwards = []
    for t, n in children:
      if n in auto_provided_defs:
        forwards.append(auto_provided_defs[n])
      elif enum_name_and_value is not None and t == enum_name_and_value["name"]:
        # Need to fully qualify the enum value because we are out of class_type's scope here.
        forwards.append(f"    {class_type}::{enum_name_and_value['name']}::{enum_name_and_value['value']}")
      else:
        forwards.append(f"    std::move({n})")
    forwards = ",\n".join(forwards)

    return f"""
std::unique_ptr<{class_type}>
{name}(
{arguments}
) {{
  return std::make_unique<
    {class_type},
{argument_types}
  >(
{forwards}
  );
}}
"""

  defs = []
  for c, o in subclasses:
    if enum := o.get("enumDefinition"):
      # The subclass has an enum. 
      # Make a factory for each value of the enum, named after the enum.
      # E.g. Add, Sub, Div, etc. factories instead of one binOp factory.
      for v in enum["values"]:
        children_with_enum_type = [child for child in o["children"] if child[0] == enum["name"]]
        assert len(children_with_enum_type) == 1, (
          f"Expected enum {enum['name']} to appear once as child of subclass {c}"
        )
        assert children_with_enum_type[0][1] not in auto_provided_defs, (
          f"Enum {enum['name']} can't be auto-provided in {c}"
        )
        defs.append(make_snippet(c, to_camel_case(v), o["children"], { "name": enum["name"], "value" : v }))
    else:
      # No enum. Just make one factory function, based on the name of the subclass.
      defs.append(make_snippet(c, to_camel_case(c), o["children"], None))

  return "\n".join(defs)

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
