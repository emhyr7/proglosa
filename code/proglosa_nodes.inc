/* (identifier, body) */

XPASTE(undefined, {})

/* literals */
XPASTE(identifier,     { utf8   *runes; uint runes_count; })
XPASTE(string,         { utf8   *runes; uint runes_count; })
XPASTE(rune,           { utf32   value; })
XPASTE(number,         { union { uint64  uint64; float64 float64; }; })

#define PROCEDURE_TYPE_NODE_BODY { expression *arguments; expression *results; }
XPASTE(procedure_type, PROCEDURE_TYPE_NODE_BODY)

XPASTE(declaration,
{
  identifier_node identifier;
  expression *type_definition;
  expression *assignment;
  bit is_constant : 1;
})

/* scoped literals */
XPASTE(structure,
{
  statement *declarations;
  uint       declarations_count;
})

XPASTE(procedure,
{
  struct PROCEDURE_TYPE_NODE_BODY;
  structure_node structure;
  statement     *statements;
  uint           statements_count;
})

#undef PROCEDURE_TYPE_NODE_BODY

/* expressions */
#define UNARY_NODE_BODY   { expression *expression; }
#define BINARY_NODE_BODY  { expression *left; expression *right; }
#define TERNARY_NODE_BODY { struct BINARY_NODE_BODY; expression *other; }

XPASTE(unary,   UNARY_NODE_BODY)   /* +a        */
XPASTE(binary,  BINARY_NODE_BODY)  /* a + b     */
XPASTE(ternary, TERNARY_NODE_BODY) /* a ? b : c */

/* logical expressions */
XPASTE(negation,          UNARY_NODE_BODY)  /* !a     */
XPASTE(conjunction,       BINARY_NODE_BODY) /* a && b */
XPASTE(disjunction,       BINARY_NODE_BODY) /* a || b */
XPASTE(equality,          BINARY_NODE_BODY) /* a == b */
XPASTE(inequality,        BINARY_NODE_BODY) /* a != b */
XPASTE(greater,           BINARY_NODE_BODY) /* a >  b */
XPASTE(lesser,            BINARY_NODE_BODY) /* a <  b */
XPASTE(inclusive_greater, BINARY_NODE_BODY) /* a >= b */
XPASTE(inclusive_lesser,  BINARY_NODE_BODY) /* a <= b */

/* arithmetic expressions */
XPASTE(positive,       UNARY_NODE_BODY)  /* +a    */
XPASTE(negative,       UNARY_NODE_BODY)  /* -a    */
XPASTE(addition,       BINARY_NODE_BODY) /* a + b */
XPASTE(subtraction,    BINARY_NODE_BODY) /* a - b */
XPASTE(multiplication, BINARY_NODE_BODY) /* a * b */
XPASTE(division,       BINARY_NODE_BODY) /* a / b */
XPASTE(modulo,         BINARY_NODE_BODY) /* a % b */

/* bitwise expressions */
XPASTE(bitwise_negation,              UNARY_NODE_BODY)  /* ~a     */
XPASTE(bitwise_conjunction,           BINARY_NODE_BODY) /* a &  b */
XPASTE(bitwise_disjunction,           BINARY_NODE_BODY) /* a |  b */
XPASTE(bitwise_exclusive_disjunction, BINARY_NODE_BODY) /* a %  b */
XPASTE(bitwise_left_shift,            BINARY_NODE_BODY) /* a << b */
XPASTE(bitwise_right_shift,           BINARY_NODE_BODY) /* a >> b */

/* assignment expressions */
XPASTE(assignment,                               BINARY_NODE_BODY) /* a   = b */
XPASTE(addition_assignment,                      BINARY_NODE_BODY) /* a  += b */
XPASTE(subtraction_assignment,                   BINARY_NODE_BODY) /* a  -= b */
XPASTE(multiplication_assignment,                BINARY_NODE_BODY) /* a  *= b */
XPASTE(division_assignment,                      BINARY_NODE_BODY) /* a  /= b */
XPASTE(modulo_assignment,                        BINARY_NODE_BODY) /* a  %= b */
XPASTE(bitwise_conjunction_assignment,           BINARY_NODE_BODY) /* a  &= b */
XPASTE(bitwise_disjunction_assignment,           BINARY_NODE_BODY) /* a  |= b */
XPASTE(bitwise_exclusive_disjunction_assignment, BINARY_NODE_BODY) /* a  %= b */
XPASTE(bitwise_left_shift_assignment,            BINARY_NODE_BODY) /* a <<= b */
XPASTE(bitwise_right_shift_assignment,           BINARY_NODE_BODY) /* a >>= b */

/* resolution expressions */
XPASTE(reference,  UNARY_NODE_BODY)  /* @a  */
XPASTE(resolution, BINARY_NODE_BODY) /* a.b */

/* other expressions */
XPASTE(invocation, BINARY_NODE_BODY)  /* a b       */
XPASTE(list,       BINARY_NODE_BODY)  /* a, b      */
XPASTE(condition,  TERNARY_NODE_BODY) /* a ? b : c */

#undef TERNARY_NODE_BODY
#undef BINARY_NODE_BODY
#undef UNARY_NODE_BODY
