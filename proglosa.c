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

  fprintf(stderr, "\x1b[1;31m");
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

inline void v_report_token(reporting_type type, parser *parser, const utf8 *message, vargs vargs)
{
  v_report(type, parser->source, parser->source_path, parser->token.beginning, parser->token.ending, parser->token.row, parser->token.column, message, vargs);
}

inline void report_token(reporting_type type, parser *parser, const utf8 *message, ...)
{
  vargs vargs;
  get_vargs(vargs, message);
  v_report_token(type, parser, message, vargs);
  end_vargs(vargs);
}

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
  print_comment("Loading source: %s\n", path);

  begin_clock();
  parser->source_path = path;
  handle source_handle = open_file(parser->source_path);
  parser->source_size = (uint)get_file_size(source_handle);
  parser->source = (utf8 *)push((uint)align_forwards(parser->source_size, 4), universal_alignment, &parser->general_allocator);
  read_from_file(parser->source, parser->source_size, source_handle);
  close_file(source_handle);

  parser->offset    = 0;
  parser->row       = 0;
  parser->rune      = '\n',
  parser->increment = 0;
  advance(parser);
}

static token_tag get_token(parser *parser)
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
      token->tag = token_tag_equality2;
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
          token->tag = token_tag_identifier;
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
      report_token_failure(parser, "Unknown token.");
      jump(*parser->failure_jump_point, 1);
  }
  return token->tag;
}

static void ensure_token(token_tag tag, parser *parser)
{
  if (parser->token.tag != tag)
  {
    report_token_failure(parser, "Expected token: %s.", token_tag_representations[tag]);
    jump(*parser->failure_jump_point, 1);
  }
}

static void expect_token(token_tag tag, parser *parser)
{
  get_token(parser);
  ensure_token(tag, parser);
}

/*****************************************************************************/

const utf8 *node_tag_representations[] =
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

static void parse_structure_scope(scope_node *scope, parser *parser);
static void parse_procedure_scope(scope_node *scope, parser *parser);

static utf8 *parse_identifier(uint *identifier_size, parser *parser)
{
  ASSERT(parser->token.tag == token_tag_identifier);

  *identifier_size = get_token_size(parser);
  utf8 *identifier = push_type(utf8, *identifier_size, &parser->general_allocator);
  copy_memory(identifier, get_token_pointer(parser), *identifier_size);
  get_token(parser);
  return identifier;
}

static void parse_path(path_node *path, parser *parser)
{
  ASSERT(parser->token.tag == token_tag_identifier);
  for (;;)
  {
    path->identifier_size = get_token_size(parser);
    path->identifier = push_type(utf8, path->identifier_size, &parser->general_allocator);
    copy_memory(path->identifier, get_token_pointer(parser), path->identifier_size);
    if (get_token(parser) != token_tag_dot) break;
    expect_token(token_tag_identifier, parser);
    path = path->next = push_type(path_node, 1, &parser->general_allocator);
  }
  path->next = 0;
}

static node *parse_type_definition(parser *parser)
{
  node *result = 0;
  switch (parser->token.tag)
  {
  case token_tag_identifier:
    /* path */
    result = push_train(node, sizeof(path_node), &parser->general_allocator);
    result->tag = node_tag_path;
    parse_path(&result->data->path, parser);
    break;
  case token_tag_at:
    /* pointer-type */
    get_token(parser);
    result = push_train(node, sizeof(pointer_type_node), &parser->general_allocator);
    result->tag = node_tag_pointer_type;
    result->data->pointer_type.subtype = parse_type_definition(parser);
    break;
  case token_tag_left_brace:
    /* structure-type */
    get_token(parser);
    result = push_train(node, sizeof(structure_type_node), &parser->general_allocator);
    result->tag = node_tag_structure_type;
    parse_structure_scope(&result->data->structure_type.scope, parser);
    break;
  case token_tag_left_parenthesis:
    /* procedure-type */
    {
      get_token(parser);
      result = push_train(node, sizeof(procedure_type_node), &parser->general_allocator);
      result->tag = node_tag_procedure_type;
      result->data->procedure_type.parameters_count = 0;
      parameter *prior_parameter = 0;
      while (parser->token.tag != token_tag_right_parenthesis)
      {
        parameter *last_parameter = push_type(parameter, 1, &parser->general_allocator);
        last_parameter->node = parse_type_definition(parser);
        if (prior_parameter) prior_parameter = prior_parameter->next = last_parameter;
        else result->data->procedure_type.parameters = prior_parameter = last_parameter;
        ++result->data->procedure_type.parameters_count;
        if (parser->token.tag == token_tag_comma) get_token(parser);
      }
      result->data->procedure_type.arguments_count = result->data->procedure_type.parameters_count;
      if (get_token(parser) == token_tag_arrow)
      {
        get_token(parser);
        do
        {
          parameter *last_parameter = push_type(parameter, 1, &parser->general_allocator);
          last_parameter->node = parse_type_definition(parser);
          if (prior_parameter) prior_parameter = prior_parameter->next = last_parameter;
          else result->data->procedure_type.parameters = prior_parameter = last_parameter;
          ++result->data->procedure_type.parameters_count;
          if (parser->token.tag == token_tag_comma) get_token(parser);
        }
        while (parser->token.tag == token_tag_identifier);
      }
      break;
    }
  default:
    get_token(parser);
    break;
  }

  return result;
}

typedef enum
{
  precedence_type,
  precedence_default,
  precedence_logic,
  precedence_comparison,
  precedence_term,
  precedence_factor,
  precedence_bitwise,
  precedence_cast,
  precedence_unary,
} precedence;

static node *parse_subexpression(precedence precedence, parser *parser)
{
  node *result = 0;
  /* TODO */
  return result;
}

static node *parse_expression(precedence precedence, parser *parser)
{
  node *expression = parse_subexpression(precedence, parser);
  for (;;)
  {
     
  }

  return expression;
}

static void parse_declaration(declaration_node *declaration, parser *parser)
{
  ASSERT(parser->token.tag == token_tag_identifier);

  declaration->symbol = push_type(symbol, 1, &parser->general_allocator);
  symbol *symbol = declaration->symbol;
  symbol->declaration = declaration;

  symbol->identifier_size = get_token_size(parser);
  symbol->identifier = parse_identifier(&symbol->identifier_size, parser); 
  ensure_token(token_tag_colon, parser);

  get_token(parser); /* skip `:` */
  symbol->type_definition = parse_type_definition(parser);

  bit is_constant = 0;
  switch (parser->token.tag)
  {
  case token_tag_colon:
    is_constant = 1;
  case token_tag_equality:
    get_token(parser);
    symbol->assignment = parse_expression(precedence_default, parser);
    break;
  default:
    if (!symbol->type_definition)
    {
      symbol->assignment = parse_expression(precedence_default, parser);
      if (!symbol->assignment)
      {
        report_token_failure(parser, "A declaration with an implicit type must have an assignment.");
        jump(*parser->failure_jump_point, 1);
      }
    }
    break;
  }
  symbol->is_constant = is_constant;
}

static void parse_procedure_scope(scope_node *scope, parser *parser)
{
  UNIMPLEMENTED();
}

static void parse_structure_scope(scope_node *scope, parser *parser)
{
  fill_memory(scope, sizeof(*scope), 0);

  /* TODO: upon the failure of an iteration, deallocate the allocated memory
           from the iteration, and skip to a valid onset. */

  get_token(parser);

  statement *prior_statement = 0;
  symbol *prior_symbol = 0;

  for (;;)
  {
    statement *last_statement = 0;

    switch (parser->token.tag)
    {
    case token_tag_identifier:
      last_statement = push_train(statement, sizeof(declaration_node), &parser->general_allocator);
      last_statement->tag = node_tag_declaration;
      parse_declaration(&last_statement->data->declaration, parser);
      break;
    case token_tag_semicolon:
      get_token(parser);
      break;
    case token_tag_right_brace:
      if (scope == &parser->program->global_scope)
      {
        report_token_failure(parser, "Encountered extraneous %s.", token_tag_representations[token_tag_right_brace]);
        goto failure;
      }
      else
      {
        get_token(parser);
        break;
      }
    default:
      report_token_failure(parser, "Expected %s.", token_tag_representations[token_tag_identifier]);
      goto failure;
    }

    if (last_statement)
    {
      if (prior_statement) prior_statement = prior_statement->next = last_statement;
      else scope->statements = prior_statement = last_statement;

      if (last_statement->tag == node_tag_declaration)
      {
        if (scope->symbols) prior_symbol = prior_symbol->next = last_statement->data->declaration.symbol;
        else scope->symbols = prior_symbol = last_statement->data->declaration.symbol;
        ++scope->symbols_count;
      }

      print_comment("Parsed %s.\n", node_tag_representations[last_statement->tag]);
    }

    continue;

  failure:
    jump(*parser->failure_jump_point, 1);
  }
}

void parse(const utf8 *path, program *program, parser *parser)
{
  fill_memory(parser, sizeof(*parser), 0);

  parser->program = program;

  /* initialize failure system */
  jump_point failure_jump_point;
  jump_point *prior_context_failure_jump_point = context.failure_jump_point;
  parser->failure_jump_point = &failure_jump_point;
  context.failure_jump_point = parser->failure_jump_point;
  if (set_jump_point(failure_jump_point))
  {
    print_comment("Failed to %s.\n", __FUNCTION__);
    /* TODO: handle failure here */
    goto defer;
  }

  /* load the source */
  load_into_parser(path, parser);

  /* initialize closure */
  if (set_jump_point(parser->etx_jump_point))
    goto done_parsing;

  /* parse */
  parse_structure_scope(&parser->program->global_scope, parser);

done_parsing:

defer:
  context.failure_jump_point = prior_context_failure_jump_point;
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
