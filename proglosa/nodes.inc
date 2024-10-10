/* (identifier, body) */

XPASTE(scope,
{
  scope_node *parent;
  symbol *structure_symbols;
  symbol *procedure_symbols;
  symbol *value_symbols;
  uint structure_symbols_count;
  uint procedure_symbols_count;
  uint value_symbols_count;
})

XPASTE(structure,
{
  symbol *symbol;
  scope_node scope;
})

XPASTE(procedure,
{
  symbol *symbol;
})

XPASTE(value,
{
  symbol *symbol;
})
