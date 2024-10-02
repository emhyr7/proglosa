#include "proglosa_parsing.h"

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

static utf32 peek(uints *increment, parser *parser)
{
  uint peek_offset = parser->offset + parser->increment;
  if (peek_offset >= parser->source_size)
  {
    *increment = 0;
    return (utf32)etx_token_type;
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

void load_into_parser(const utf8 *path, parser *parser)
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

token_type tokenize(parser *parser)
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
  case etx_token_type:
    token->type = etx_token_type;
    goto finished;
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
    token->type = string_token_type;
    break;
  case '=':
    peeked_rune = peek(&peeked_increment, parser);
    if (peeked_rune == '=')
    {
      token->type = equals2_token_type;
      goto double_rune;
    }
    goto set_single_rune;
  case '<':
    peeked_rune = peek(&peeked_increment, parser);
    if (peeked_rune == '<')
    {
      token->type = left_angle2_token_type;
      goto double_rune;
    }
    goto set_single_rune;
  case '>':
    peeked_rune = peek(&peeked_increment, parser);
    if (peeked_rune == '>')
    {
      token->type = right_angle2_token_type;
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
      token->type = arrow_token_type;
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
      token->type = (token_type)parser->rune;
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
               if (!compare_literal_string("proc",   text)) token->type = proc_keyword_token_type;
          else if (!compare_literal_string("struct", text)) token->type = struct_keyword_token_type;
          else if (!compare_literal_string("enum",   text)) token->type = enum_keyword_token_type;
          else if (!compare_literal_string("union",  text)) token->type = union_keyword_token_type;
          else token->type = identifier_token_type;
          *ending = ending_rune;
      }
      else if (on_number(parser))
      {
          do advance(parser);
          while(on_number(parser) || on('_', parser));
          token->type = number_token_type;
      }
      else
      {
          token->type = unknown_token_type;
          advance(parser);
      }
      break;
  }

finished:
  token->ending = parser->offset;
  if (token->type == unknown_token_type)
  {
      report_token_failure(token, parser->source, parser->source_path, "unknown token.");
  }
  return token->type;
}
