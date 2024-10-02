#pragma once

#include "proglosa.h"

/*****************************************************************************/

typedef enum
{
  comment_reporting_type,
  caution_reporting_type,
  failure_reporting_type,
} reporting_type;

static const utf8 reporting_type_representation[][8] =
{
  [comment_reporting_type] = "comment",
  [caution_reporting_type] = "caution",
  [failure_reporting_type] = "failure",
};

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
  #define XPASTE(identifier, value, representation) identifier##_token_type = value,
    #include "proglosa_tokens.inc"
  #undef XPASTE
} token_type;

static const utf8 token_type_representations[][16] =
{
  #define XPASTE(identifier, value, representation) [identifier##_token_type] = representation,
    #include "proglosa_tokens.inc"
  #undef XPASTE
};

typedef struct
{
  token_type type;
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

  token token;
} parser;

void load_into_parser(const utf8 *path, parser *parser);

token_type tokenize(parser *parser);
