#pragma once

#include "base.h"

int start(int arguments_count, char *arguments[]);

/*****************************************************************************/

typedef enum
{
  reporting_type_comment,
  reporting_type_caution,
  reporting_type_failure,
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

#define report_comment(...) report(reporting_type_comment, __VA_ARGS__)
#define report_caution(...) report(reporting_type_caution, __VA_ARGS__)
#define report_failure(...) report(reporting_type_failure, __VA_ARGS__)

/*****************************************************************************/

typedef enum
{
#define X(identifier, value, representation) token_tag_##identifier = value,
  #include "proglosa_tokens.inc"
#undef X
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

typedef struct parser parser;

void v_report_token(reporting_type type, parser *parser, const utf8 *message, vargs vargs);

void report_token(reporting_type type, parser *parser, const utf8 *message, ...);

#define report_token_comment(...) report_token(reporting_type_comment, __VA_ARGS__)
#define report_token_caution(...) report_token(reporting_type_caution, __VA_ARGS__)
#define report_token_failure(...) report_token(reporting_type_failure, __VA_ARGS__)

/*****************************************************************************/

typedef enum
{
#define X(identifier, body) node_tag_##identifier,
  #include "proglosa_nodes.inc"
#undef X
} node_tag;

extern const utf8 *node_tag_representations[];

typedef struct expression expression;

#define X(identifier, body) typedef struct identifier##_node identifier##_node;
  #include "proglosa_nodes.inc"
#undef X

#define X(identifier, body) struct identifier##_node body;
  #include "proglosa_nodes.inc"
#undef X

struct expression
{
  node_tag tag;
  union
  {
#define X(identifier, body) identifier##_node identifier;
  #include "proglosa_nodes.inc"
#undef X
  } data[];
};

#define identifier_allocator_chunk_size (8)
#define maximum_identifier_size         (uint_bits_count * identifier_allocator_chunk_size)

typedef struct
{
  scope_node global_scope;
} program;

/*****************************************************************************/

struct parser
{
  allocator general_allocator;
  
  const utf8 *source_path;
  utf8 *source;
  uint  source_size;

  uint offset;
  uint row;
  uint column;

  utf32 rune;
  uints increment;

  bit finished_parsing : 1;

  jump_point *failure_jump_point;
  
  token       token;
  program    *program;
  scope_node *current_scope;
};

void parse(const utf8 *path, program *program, parser *parser);
