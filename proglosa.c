#include "proglosa.h"

#include "proglosa/base.c"

/*****************************************************************************/

const utf8 reporting_type_representation[][8] =
{
  [reporting_type_comment] = "comment",
  [reporting_type_caution] = "caution",
  [reporting_type_failure] = "failure",
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
  #define XPASTE(identifier, value, representation) [token_tag_##identifier] = representation,
    #include "proglosa/tokens.inc"
  #undef XPASTE
};

static utf32 peek(uints *increment, parser *parser)
{
  uint peek_offset = parser->offset + parser->increment;
  if (peek_offset >= parser->source_size)
  {
    *increment = 0;
    return token_tag_etx;
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
  if (parser->rune == (utf32)token_tag_etx)
    jump(parser->etx_jump_point, 1);
  return rune;
}

static bit on(utf32 rune, parser *parser)
{
  return rune == parser->rune;
}

static bit on_space(parser *parser)
{
  return iswspace(parser->rune);
}

static bit on_letter(parser *parser)
{
  return iswalpha(parser->rune);
}

static bit on_number(parser *parser)
{
  return iswdigit(parser->rune);
}

static void load_into_parser(const utf8 *path, parser *parser)
{
  parser->source_path = path;
  handle source_handle = open_file(parser->source_path);
  parser->source_size = (uint)get_file_size(source_handle);
  parser->source = (utf8 *)allocate((uint)align_forwards(parser->source_size, 4), universal_alignment, &parser->allocator);
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
    token->tag = token_tag_string;
    break;
  case '=':
    peeked_rune = peek(&peeked_increment, parser);
    if (peeked_rune == '=')
    {
      token->tag = token_tag_equals2;
      goto double_rune;
    }
    goto set_single_rune;
  case '<':
    peeked_rune = peek(&peeked_increment, parser);
    if (peeked_rune == '<')
    {
      token->tag = token_tag_left_angle2;
      goto double_rune;
    }
    goto set_single_rune;
  case '>':
    peeked_rune = peek(&peeked_increment, parser);
    if (peeked_rune == '>')
    {
      token->tag = token_tag_right_angle2;
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
      token->tag = token_tag_arrow;
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
               if (!compare_literal_string("proc",   text)) token->tag = token_tag_proc_keyword;
          else if (!compare_literal_string("struct", text)) token->tag = token_tag_struct_keyword;
          else if (!compare_literal_string("enum",   text)) token->tag = token_tag_enum_keyword;
          else if (!compare_literal_string("union",  text)) token->tag = token_tag_union_keyword;
          else token->tag = token_tag_identifier;
          *ending = ending_rune;
      }
      else if (on_number(parser))
      {
          do advance(parser);
          while(on_number(parser) || on('_', parser));
          token->tag = token_tag_number;
      }
      else
      {
          token->tag = token_tag_unknown;
          advance(parser);
      }
      break;
  }

  token->ending = parser->offset;
  if (token->tag == token_tag_unknown)
  {
      report_token_failure(token, parser->source, parser->source_path, "unknown token.");
      jump(*parser->failure_jump_point, 1);
  }
  return token->tag;
}

/*****************************************************************************/

const utf8 node_tag_representations[][16] =
{
  #define XPASTE(identifier, body) [node_tag_##identifier] = #identifier,
    #include "proglosa/nodes.inc"
  #undef XPASTE
};

/*****************************************************************************/

static uint get_token_size(parser *parser)
{
  return parser->token.ending - parser->token.beginning;
}

static utf8 *get_token_pointer(parser *parser)
{
  return parser->source + parser->token.beginning;
}

static void parse_structure_scope(scope_node *scope, parser *parser)
{
  
}

void parse(const utf8 *path, program *program, parser *parser)
{
  fill_memory(0, parser, sizeof(*parser));
  scratch scratch;
  get_scratch(&scratch, &parser->allocator);

  parser->program = program;

  /* initialize failure system */
  jump_point failure_jump_point;
  jump_point *prior_context_failure_jump_point = context.failure_jump_point;
  parser->failure_jump_point = &failure_jump_point;
  context.failure_jump_point = parser->failure_jump_point;
  if (set_jump_point(failure_jump_point))
  {
    /* TODO: handle failure here */
    UNIMPLEMENTED();
  }

  /* load the source */
  load_into_parser(path, parser);

  /* initialize closure */
  if (set_jump_point(parser->etx_jump_point))
    goto done_parsing;

  /* parse */
  parse_structure_scope(&parser->program->global_scope, parser);

done_parsing:
  context.failure_jump_point = prior_context_failure_jump_point;

  end_scratch(&scratch);
}

/*****************************************************************************/

int start(int arguments_count, char *arguments[])
{
  if (!initialize_base())
    UNIMPLEMENTED();

  if (arguments_count <= 1)
  {
    print_failure("A source path wasn't given.");
    return -1;
  }

  program program;

  parser parser;
  parse(arguments[1], &program, &parser);

  return 0;
}
