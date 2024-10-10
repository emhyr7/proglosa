/* (identifier, body) */

XPASTE(undefined, {})

XPASTE(scope,
{
  scope_node *parent;

  symbol *symbols;
  uint symbols_count;

  statement *statements;
  uint statements_count;
})

XPASTE(path,
{
  path_node *next;
  utf8 *identifier;
  uint identifier_size;
})

XPASTE(pointer_type,
{
  node *subtype;
})

XPASTE(structure_type,
{
  scope_node scope;
})

XPASTE(procedure_type,
{
  parameter *parameters;
  uint parameters_count;
  uint arguments_count;
})

XPASTE(declaration,
{
  symbol *symbol;
})

XPASTE(procedure,
{
  declaration_node *parameters;
  uint parameters_count;
  uint arguments_count;
  scope_node scope;
})

XPASTE(value,
{
  declaration_node *declaration;
})
