#pragma once

#include "proglosa_commons.h"

int start(int arguments_count, char *arguments[]);

/*****************************************************************************/

typedef enum
{
  comment_reporting_type,
  caution_reporting_type,
  failure_reporting_type,
} reporting_type;

extern const utf8 reporting_type_representation[][8];

void v_report(reporting_type type, const utf8 *source, const utf8 *path, uint beginning, uint ending, uint row, uint column, const utf8 *message, vargs vargs);

inline void report(reporting_type type, const utf8 *source, const utf8 *path, uint beginning, uint ending, uint row, uint column, const utf8 *message, ...)
{
  vargs vargs;
  get_vargs(vargs, message);
  v_report(type, source, path, beginning, ending, row, column, message, vargs);
  end_vargs(vargs);
}

#define report_comment(...) report(comment_reporting_type, __VA_ARGS__)
#define report_caution(...) report(caution_reporting_type, __VA_ARGS__)
#define report_failure(...) report(failure_reporting_type, __VA_ARGS__)

/*****************************************************************************/

typedef enum
{
  #define XPASTE(identifier, value, representation) identifier##_token_tag = value,
    #include "proglosa_tokens.inc"
  #undef XPASTE
} token_tag;

extern const utf8 token_tag_representations[][16];

typedef struct
{
  token_tag tag;
  uint beginning;
  uint ending;
  uint row;
  uint column;
} token;

inline void v_report_token(reporting_type type, token *token, const utf8 *source, const utf8 *path, const utf8 *message, vargs vargs)
{
  v_report(type, source, path, token->beginning, token->ending, token->row, token->column, message, vargs);
}

inline void report_token(reporting_type type, token *token, const utf8 *source, const utf8 *path, const utf8 *message, ...)
{
  vargs vargs;
  get_vargs(vargs, message);
  v_report_token(type, token, source, path, message, vargs);
  end_vargs(vargs);
}

#define report_token_comment(...) report_token(comment_reporting_type, __VA_ARGS__)
#define report_token_caution(...) report_token(caution_reporting_type, __VA_ARGS__)
#define report_token_failure(...) report_token(failure_reporting_type, __VA_ARGS__)

/*****************************************************************************/

typedef enum
{
  #define XPASTE(identifier, body) identifier##_node_tag,
    #include "proglosa_nodes.inc"
  #undef XPASTE
} node_tag;

extern const utf8 node_tag_representations[][16];

#define XPASTE(identifier, body) typedef struct identifier##_node identifier##_node;
  #include "proglosa_nodes.inc"
#undef XPASTE

typedef struct
{
  node_tag tag;
  union
  {
    #define XPASTE(identifier, body) identifier##_node *identifier;
      #include "proglosa_nodes.inc"
    #undef XPASTE
  } data[];
} node;

typedef struct
{
  utf8 *identifier;
  uints identifier_size;
  union
  {
    structure_node *structure;
    procedure_node *procedure;
    value_node     *value;
  };
} symbol;

#define XPASTE(identifier, body) struct identifier##_node body;
  #include "proglosa_nodes.inc"
#undef XPASTE

typedef struct
{
  scope_node global_scope;
} program;

/*****************************************************************************/

typedef struct
{
  const utf8 *source_path;
  utf8 *source;
  uint  source_size;

  uint offset;
  uint row;
  uint column;

  utf32 rune;
  uints increment;

  bool finished_parsing;
  jump_point etx_jump_point;
  jump_point failure_jump_point;
  token token;

  program *program;
} parser;

void parse(const utf8 *path, program *program, parser *parser);
