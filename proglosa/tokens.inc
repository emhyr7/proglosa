/* (identifier, value, representation) */

XPASTE(unknown, 0, "unknown")
XPASTE(etx, '\3', "ETX")

/* single characters */
XPASTE(exclamation,       '!', "'!'")
XPASTE(octothorpe,        '#', "'#'")
XPASTE(dollar,            '$', "'$'")
XPASTE(percent,           '%', "'%'")
XPASTE(ampersand,         '&', "'&'")
XPASTE(left_parenthesis,  '(', "'('")
XPASTE(right_parenthesis, ')', "')'")
XPASTE(asterisk,          '*', "'*'")
XPASTE(plus,              '+', "'+'")
XPASTE(comma,             ',', "','")
XPASTE(dash,              '-', "'-'")
XPASTE(dot,               '.', "'.'")
XPASTE(slash,             '/', "'/'")
XPASTE(colon,             ':', "':'")
XPASTE(semicolon,         ';', "';'")
XPASTE(left_angle,        '<', "'<'")
XPASTE(equals,            '=', "'='")
XPASTE(right_angle,       '>', "'>'")
XPASTE(question,          '?', "'?'")
XPASTE(at,                '@', "'@'")
XPASTE(left_bracket,      '[', "'['")
XPASTE(right_bracket,     ']', "']'")
XPASTE(caret,             '^', "'^'")
XPASTE(left_brace,        '{', "'{'")
XPASTE(bar,               '|', "'|'")
XPASTE(right_brace,       '}', "'}'")
XPASTE(tilde,             '~', "'~'")

/* double characters */
XPASTE(equals2,      130, "'=='")
XPASTE(left_angle2,  131, "'<<'")
XPASTE(right_angle2, 132, "'>>'")
XPASTE(arrow,        133, "'->'")

/* keywords */
XPASTE(proc_keyword,   140, "`proc`")
XPASTE(struct_keyword, 141, "`struct`")
XPASTE(enum_keyword,   142, "`enum`")
XPASTE(union_keyword,  143, "`union`")

/* literals */
XPASTE(identifier, 150, "identifier")
XPASTE(number,     151, "number")
XPASTE(string,     152, "string")
