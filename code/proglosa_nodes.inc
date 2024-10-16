// (identifier, body)

X(undefined, {})

// expressions
#define UNARY_NODE_BODY   { expression *expression; }
#define BINARY_NODE_BODY  { expression *left; expression *right; }
#define TERNARY_NODE_BODY { struct BINARY_NODE_BODY; struct UNARY_NODE_BODY; }

X(unary,   UNARY_NODE_BODY)
X(binary,  BINARY_NODE_BODY)
X(ternary, TERNARY_NODE_BODY)

// scoped expressions
X(subexpression, UNARY_NODE_BODY) // `(` x `)`
X(indexation,    UNARY_NODE_BODY) // `[` x `]`

// logical expressions
X(logical_negation,  UNARY_NODE_BODY)  // `!` x   
X(conjunction,       BINARY_NODE_BODY) // x `&&` x
X(disjunction,       BINARY_NODE_BODY) // x `||` x
X(equality,          BINARY_NODE_BODY) // x `==` x
X(inequality,        BINARY_NODE_BODY) // x `!=` x
X(inclusive_greater, BINARY_NODE_BODY) // x `>=` x
X(inclusive_lesser,  BINARY_NODE_BODY) // x `<=` x
X(greater,           BINARY_NODE_BODY) // x `>` x 
X(lesser,            BINARY_NODE_BODY) // x `<` x 

// arithmetic expressions
X(negation,          UNARY_NODE_BODY)  // `-` x  
X(addition,          BINARY_NODE_BODY) // x `+` x
X(subtraction,       BINARY_NODE_BODY) // x `-` x
X(multiplication,    BINARY_NODE_BODY) // x `*` x
X(division,          BINARY_NODE_BODY) // x `/` x
X(modulo,            BINARY_NODE_BODY) // x `%` x

// bitwise expressions
X(bitwise_negation,              UNARY_NODE_BODY)  // `~` x   
X(bitwise_conjunction,           BINARY_NODE_BODY) // x `&` x 
X(bitwise_disjunction,           BINARY_NODE_BODY) // x `|` x 
X(bitwise_exclusive_disjunction, BINARY_NODE_BODY) // x `^` x 
X(bitwise_left_shift,            BINARY_NODE_BODY) // x `<<` x
X(bitwise_right_shift,           BINARY_NODE_BODY) // x `>>` x

// assignment expressions
X(assignment,                               BINARY_NODE_BODY) // x `=` x  
X(constant_assignment,                      BINARY_NODE_BODY) // x `:` x  
X(addition_assignment,                      BINARY_NODE_BODY) // x `+=` x 
X(subtraction_assignment,                   BINARY_NODE_BODY) // x `-=` x 
X(multiplication_assignment,                BINARY_NODE_BODY) // x `*=` x 
X(division_assignment,                      BINARY_NODE_BODY) // x `/=` x 
X(modulo_assignment,                        BINARY_NODE_BODY) // x `%=` x 
X(bitwise_conjunction_assignment,           BINARY_NODE_BODY) // x `&=` x 
X(bitwise_disjunction_assignment,           BINARY_NODE_BODY) // x `|=` x 
X(bitwise_exclusive_disjunction_assignment, BINARY_NODE_BODY) // x `^=` x 
X(bitwise_left_shift_assignment,            BINARY_NODE_BODY) // x `<<=` x
X(bitwise_right_shift_assignment,           BINARY_NODE_BODY) // x `>>=` x

// other expressions
X(reference,   UNARY_NODE_BODY)   // `@` x  
X(invocation,  BINARY_NODE_BODY)  // x x               
X(resolution,  BINARY_NODE_BODY)  // x `.` x
X(list,        BINARY_NODE_BODY)  // x `,` x           
X(lambda,      BINARY_NODE_BODY)  // x `->` x
X(declaration, TERNARY_NODE_BODY) // x `:` x [(':'|'=') x]
X(condition,   TERNARY_NODE_BODY) // x `?` x [`:` x]

#undef TERNARY_NODE_BODY
#undef BINARY_NODE_BODY
#undef UNARY_NODE_BODY

// literals
X(identifier, { utf8   *runes; uint runes_count; }) // "[`_`|letter]+ [`_`|letter|digit]"
X(string,     { utf8   *runes; uint runes_count; })
X(rune,       { utf32   value; })
X(digital,    { uint64  value; })
X(decimal,    { float64 value; })

// `{` [x|lambda] {x [';']} `}`
X(scope,
{
  lambda_node  *lambda;
  expression  **expressions;
  uint          expressions_count;
})
