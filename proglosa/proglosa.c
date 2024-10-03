#include "proglosa.h"

#include "proglosa_commons.c"

/*****************************************************************************/

const utf8 reporting_type_representation[][8] =
{
  [comment_reporting_type] = "comment",
  [caution_reporting_type] = "caution",
  [failure_reporting_type] = "failure",
};

void v_report(reporting_type type, const utf8 *source, const utf8 *path, uint beginning, uint ending, uint row, uint column, const utf8 *message, vargs vargs)
{
  const utf8 *type_representation = reporting_type_representation[type];
  fprintf(stderr, "%s(%u, %u): %s: ", path, row, column, type_representation);
  vfprintf(stderr, message, vargs);
  fputc('\n', stderr);

  if (beginning == ending) return;
  const utf8 *caret = source + beginning - column + 1;

  fprintf(stderr, "\t%u | ", row++);
  while (caret != source + beginning) fputc(*caret++, stderr);

  fprintf(stderr, "\x1b[1m");
  while (caret != source + ending)
  {
    if (*caret == '\n')
    {
      // TODO: align the row number
      fprintf(stderr, "\t%u | ", row);
      ++row;
    }
    fputc(*caret++, stderr);
  }
  fprintf(stderr, "\x1b[0m");

  while (*caret != '\n' && *caret != '\0') fputc(*caret++, stderr);
  fputc('\n', stderr);
}

/*****************************************************************************/

const utf8 token_tag_representations[][16] =
{
  #define XPASTE(identifier, value, representation) [identifier##_token_tag] = representation,
    #include "proglosa_tokens.inc"
  #undef XPASTE
};

static utf32 peek(uints *increment, parser *parser)
{
  uint peek_offset = parser->offset + parser->increment;
  if (peek_offset >= parser->source_size)
  {
    *increment = 0;
    return etx_token_tag;
  }
  utf32 rune;
  *increment = decode_utf8(&rune, parser->source + peek_offset);
  return rune;
}

static utf32 advance(parser *parser)
{
  uints increment;
  utf32 rune = peek(&increment, parser);

  parser->offset += parser->increment;
  if (parser->rune == '\n')
  {
    parser->row   += 1;
    parser->column = 0;
  }
  parser->column++;
  parser->rune      = rune;
  parser->increment = increment;
  if (parser->rune == (utf32)etx_token_tag)
    jump(parser->etx_jump_point, 1);
  return rune;
}

static bool on(utf32 rune, parser *parser)
{
  return rune == parser->rune;
}

static bool on_space(parser *parser)
{
  return iswspace(parser->rune);
}

static bool on_letter(parser *parser)
{
  return iswalpha(parser->rune);
}

static bool on_number(parser *parser)
{
  return iswdigit(parser->rune);
}

static void load_into_parser(const utf8 *path, parser *parser)
{
  parser->source_path = path;
  handle source_handle = open_file(parser->source_path);
  parser->source_size = (uint)get_file_size(source_handle);
  parser->source = (utf8 *)push((uint)align_forwards(parser->source_size, 4), universal_alignment);
  read_from_file(parser->source, parser->source_size, source_handle);
  close_file(source_handle);

  parser->offset    = 0;
  parser->row       = 0;
  parser->rune      = '\n',
  parser->increment = 0;
  advance(parser);
}

static token_tag tokenize(parser *parser)
{
  token *token = &parser->token;
  while (on_space(parser))
    advance(parser);

repeat:
  token->beginning = parser->offset;
  token->row       = parser->row;
  token->column    = parser->column;

  uints peeked_increment;
  utf32 peeked_rune;
  switch (parser->rune)
  {
  case '"':
    for(;;)
    { 
      advance(parser);           
      if (parser->rune == '"')
        break;
      if (parser->rune == '\\')
      {
        switch(advance(parser))
        {
        default:
          advance(parser);
          break;
        }
      }
    }
    advance(parser);
    token->tag = string_token_tag;
    break;
  case '=':
    peeked_rune = peek(&peeked_increment, parser);
    if (peeked_rune == '=')
    {
      token->tag = equals2_token_tag;
      goto double_rune;
    }
    goto set_single_rune;
  case '<':
    peeked_rune = peek(&peeked_increment, parser);
    if (peeked_rune == '<')
    {
      token->tag = left_angle2_token_tag;
      goto double_rune;
    }
    goto set_single_rune;
  case '>':
    peeked_rune = peek(&peeked_increment, parser);
    if (peeked_rune == '>')
    {
      token->tag = right_angle2_token_tag;
      goto double_rune;
    }
    goto set_single_rune;
      
  case '-':
    peeked_rune = peek(&peeked_increment, parser);
    if (peeked_rune == '-')
    {
      while (advance(parser) != '\n');
      goto repeat;
    }
    else if (peeked_rune == '>')
    {
      token->tag = arrow_token_tag;
      goto double_rune;
    }
    goto set_single_rune;

  case '!':
  case '#':
  case '$':
  case '%':
  case '&':
  case '(':
  case ')':
  case '*':
  case '+':
  case ',':
  case '.':
  case '/':
  case ':':
  case ';':
  case '?':
  case '@':
  case '[':
  case ']':
  case '^':
  case '{':
  case '|':
  case '}':
  case '~':
  set_single_rune:
      token->tag = (token_tag)parser->rune;
      goto single_rune;

  double_rune:
      advance(parser);
  single_rune:
      advance(parser);
      break;
      
  default:
      if (on_letter(parser) || on('_', parser))
      {
          do advance(parser);
          while(on_letter(parser) || on('_', parser) || on_number(parser));
          utf8 *text = parser->source + token->beginning;
          utf8 *ending = text + parser->offset;
          utf8 ending_rune = *ending;
          *ending = 0;
               if (!compare_literal_string("proc",   text)) token->tag = proc_keyword_token_tag;
          else if (!compare_literal_string("struct", text)) token->tag = struct_keyword_token_tag;
          else if (!compare_literal_string("enum",   text)) token->tag = enum_keyword_token_tag;
          else if (!compare_literal_string("union",  text)) token->tag = union_keyword_token_tag;
          else token->tag = identifier_token_tag;
          *ending = ending_rune;
      }
      else if (on_number(parser))
      {
          do advance(parser);
          while(on_number(parser) || on('_', parser));
          token->tag = number_token_tag;
      }
      else
      {
          token->tag = unknown_token_tag;
          advance(parser);
      }
      break;
  }

  token->ending = parser->offset;
  if (token->tag == unknown_token_tag)
  {
      report_token_failure(token, parser->source, parser->source_path, "unknown token.");
      jump(parser->failure_jump_point, 1);
  }
  return token->tag;
}

/*****************************************************************************/

const utf8 node_tag_representations[][16] =
{
  #define XPASTE(identifier, body) [identifier##_node_tag] = #identifier,
    #include "proglosa_nodes.inc"
  #undef XPASTE
};

/*****************************************************************************/

void parse(const utf8 *path, program *program, parser *parser)
{
  parser->program = program;

  if (set_jump_point(parser->failure_jump_point))
  {
    UNIMPLEMENTED();
  }
  jump_point *prior_context_failure_jump_point = context.failure_jump_point;
  context.failure_jump_point = &parser->failure_jump_point;

  load_into_parser(path, parser);

  if (set_jump_point(parser->etx_jump_point))
    goto done_parsing;

  for (;;)
  {
    token_tag type = tokenize(parser);
    const utf8 *representation = token_tag_representations[type];
    report_token_comment(&parser->token, parser->source, parser->source_path, representation);
  }

done_parsing:

  context.failure_jump_point = prior_context_failure_jump_point;
}

/*****************************************************************************/

static bool initialize_this_thread(void)
{
  default_allocator.state = &default_allocator_state;
  context.allocator = &default_allocator;

  if (set_jump_point(default_failure_jump_point))
    return false;

  return true;
}

int start(int arguments_count, char *arguments[])
{
  if (!initialize_this_thread())
    return -1;

  if (arguments_count <= 1)
  {
    print_failure("A source path wasn't given.");
    return -1;
  }

  parser parser;
  program program;
  parse(arguments[1], &program, &parser);

  return 0;
}
