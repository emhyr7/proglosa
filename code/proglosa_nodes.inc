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

XPASTE(unsinged_integer,
{
  uint64 value;
})

XPASTE(invocation,
{
  path_node path;
  parameter *parameters;
  uint parameters_count;
  uint arguments_count;
})

XPASTE(binary, { node *left; node *right; })

/* logical operators */
XPASTE(conjunction,       { node *left; node *right; })
XPASTE(disjunction,       { node *left; node *right; })
XPASTE(equality,          { node *left; node *right; })
XPASTE(inequality,        { node *left; node *right; })
XPASTE(greater,           { node *left; node *right; })
XPASTE(lesser,            { node *left; node *right; })
XPASTE(inclusive_greater, { node *left; node *right; })
XPASTE(inclusive_lesser,  { node *left; node *right; })

/* arithmetic operators */
XPASTE(addition,       { node *left; node *right; })
XPASTE(subtraction,    { node *left; node *right; })
XPASTE(multiplication, { node *left; node *right; })
XPASTE(division,       { node *left; node *right; })
XPASTE(modulo,         { node *left; node *right; })

/* bitwise operators */
XPASTE(bitwise_conjunction,           { node *left; node *right; })
XPASTE(bitwise_disjunction,           { node *left; node *right; })
XPASTE(bitwise_exclusive_disjunction, { node *left; node *right; })
XPASTE(bitwise_left_shift,            { node *left; node *right; })
XPASTE(bitwise_right_shift,           { node *left; node *right; })

/* assignment operators */
XPASTE(assignment,                               { node *left; node *right; })
XPASTE(addition_assignment,                      { node *left; node *right; })
XPASTE(subtraction_assignment,                   { node *left; node *right; })
XPASTE(multiplication_assignment,                { node *left; node *right; })
XPASTE(division_assignment,                      { node *left; node *right; })
XPASTE(modulo_assignment,                        { node *left; node *right; })
XPASTE(bitwise_conjunction_assignment,           { node *left; node *right; })
XPASTE(bitwise_disjunction_assignment,           { node *left; node *right; })
XPASTE(bitwise_exclusive_disjunction_assignment, { node *left; node *right; })
XPASTE(bitwise_left_shift_assignment,            { node *left; node *right; })
XPASTE(bitwise_right_shift_assignment,           { node *left; node *right; })

XPASTE(value,
{
  declaration_node *declaration;
})
