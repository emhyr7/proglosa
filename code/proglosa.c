#include "proglosa.h"

#include "base.c"

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

  fflush(stderr);
}

/*****************************************************************************/

const utf8 token_tag_representations[][16] =
{
#define XPASTE(identifier, value, representation) [token_tag_##identifier] = representation,
  #include "proglosa_tokens.inc"
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
  /* TODO: complete this */
  return (parser->rune >= '\t' && parser->rune <= '\r')
         || (parser->rune == ' ');
}

static bit on_letter(parser *parser)
{
  return (parser->rune >= 'A' && parser->rune <= 'Z')
         || (parser->rune >= 'a' && parser->rune <= 'z');
}

static bit on_number(parser *parser)
{
  return (parser->rune >= '0' && parser->rune <= '9');
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
  const utf8 *failure_message = 0;

  token *token = &parser->token;
  while (on_space(parser)) advance(parser);

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
      token->tag = token_tag_digital;
      if (on('0', parser))
      {
        peeked_rune = peek(&peeked_increment, parser);
        switch (peeked_rune)
        {
        case 'b': token->tag = token_tag_binary;      break;
        case 'x': token->tag = token_tag_hexadecimal; break;
        default: break;
        }
        advance(parser);
      }

      do
      {
        if (on('.', parser))
        {
          switch (parser->token.tag)
          {
          case token_tag_binary:
          case token_tag_hexadecimal:
            failure_message = "Weird ass number.";
            goto failed;
          default:
            break;
          }
          token->tag = token_tag_decimal;
          advance(parser);
        }
        advance(parser);
      }
      while(on_number(parser) || on('_', parser) || on('.', parser));
    }
    else
    {
      token->tag = token_tag_unknown;
      advance(parser);
      failure_message = "Unknown token.";
      goto failed_no_skip;
    }
    break;
  }

  token->ending = parser->offset;
  return token->tag;

failed:
  while (!on_space(parser)) advance(parser);

failed_no_skip:
  token->ending = parser->offset;
  report_token_failure(parser, failure_message);
  jump(*parser->failure_jump_point, 1);
}

static void ensure_token(token_tag tag, parser *parser)
{
  if (parser->token.tag != tag)
  {
    report_token_failure(parser, "Expected token: %s.", token_tag_representations[tag]);
    jump(*parser->failure_jump_point, 1);
  }
}

static void ensure_get_token(token_tag tag, parser *parser)
{
  ensure_token(tag, parser);
  get_token(parser);
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
  #include "proglosa_nodes.inc"
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

/* the same as C's */
typedef uintb precedence;
static const precedence precedences[] =
{
  [node_tag_declaration]                              = 16,

  [node_tag_invocation]                               = 15,
  [node_tag_resolution]                               = 15,

  [node_tag_negation]                                 = 14,
  [node_tag_positive]                                 = 14,
  [node_tag_negative]                                 = 14,
  [node_tag_bitwise_negation]                         = 14,
  [node_tag_reference]                                = 14,
   
  [node_tag_multiplication]                           = 13,
  [node_tag_division]                                 = 13,
  [node_tag_modulo]                                   = 13,
  
  [node_tag_addition]                                 = 12,
  [node_tag_subtraction]                              = 12,
  
  [node_tag_bitwise_left_shift]                       = 11,
  [node_tag_bitwise_right_shift]                      = 11,
  
  [node_tag_greater]                                  = 10,
  [node_tag_lesser]                                   = 10,
  [node_tag_inclusive_greater]                        = 10,
  [node_tag_inclusive_lesser]                         = 10,
  
  [node_tag_equality]                                 = 9,
  [node_tag_inequality]                               = 9,
  
  [node_tag_bitwise_conjunction]                      = 8,
  
  [node_tag_bitwise_exclusive_disjunction]            = 7,
  
  [node_tag_bitwise_disjunction]                      = 6,
  
  [node_tag_conjunction]                              = 5,
  
  [node_tag_disjunction]                              = 4,
  
  [node_tag_condition]                                = 3,
  
  [node_tag_assignment]                               = 2,
  [node_tag_addition_assignment]                      = 2,
  [node_tag_subtraction_assignment]                   = 2,
  [node_tag_multiplication_assignment]                = 2,
  [node_tag_division_assignment]                      = 2,
  [node_tag_modulo_assignment]                        = 2,
  [node_tag_bitwise_conjunction_assignment]           = 2,
  [node_tag_bitwise_disjunction_assignment]           = 2,
  [node_tag_bitwise_exclusive_disjunction_assignment] = 2,
  [node_tag_bitwise_left_shift_assignment]            = 2,
  [node_tag_bitwise_right_shift_assignment]           = 2,
  
  [node_tag_list]                                     = 1,
};
static const uint precedences_count = countof(precedences);

static void parse_declaration(declaration_node      *result, parser *parser);
static void parse_identifier (identifier_node       *reuslt, parser *parser);
static void parse_string     (string_node           *result, parser *parser);
static void parse_rune       (rune_node             *result, parser *parser);
static void parse_number     (expression            *result, parser *parser);
static void parse_structure  (structure_node        *result, parser *parser);
static void parse_procedure  (procedure_node        *result, parser *parser);

static expression *parse_expression(precedence precedence, parser *parser);
static statement  *parse_statement (                       parser *parser);

void parse_declaration(declaration_node *result, parser *parser)
{
  ASSERT(parser->token.tag == token_tag_identifier);

  parse_identifier(&result->identifier, parser);
  ensure_get_token(token_tag_colon, parser);
  switch (parser->token.tag)
  {
  case token_tag_colon:
  case token_tag_equality:
    break;
  default:
    result->type_definition = parse_expression(precedences[node_tag_declaration], parser);
    break;
  }

  result->is_constant = 0;
  switch (parser->token.tag)
  {
  case token_tag_colon:
    result->is_constant = 1;
  case token_tag_equality:
    get_token(parser);
    result->assignment = parse_expression(0, parser);
    break;
  default:
    if (!result->type_definition)
    {
      result->assignment = parse_expression(0, parser);
      if (!result->assignment)
      {
        report_token_failure(parser, "A declaration with an implicit type must have an assignment.");
        jump(*parser->failure_jump_point, 1);
      }
    }
    break;
  }
}

void parse_identifier(identifier_node *result, parser *parser)
{
  ASSERT(parser->token.tag == token_tag_identifier);

  result->runes_count = get_token_size(parser);
  result->runes = push_type(utf8, result->runes_count + 1, &parser->general_allocator);
  result->runes[result->runes_count] = 0;
  copy_typed(utf8, result->runes, get_token_pointer(parser), result->runes_count);
  get_token(parser);
}

void parse_string(string_node *result, parser *parser)
{
  ASSERT(parser->token.tag == token_tag_string);

  result->runes_count = get_token_size(parser);
  result->runes = push_type(utf8, result->runes_count + 1, &parser->general_allocator);
  result->runes[result->runes_count] = 0;
  copy_typed(utf8, result->runes, get_token_pointer(parser), result->runes_count);
  get_token(parser);
}

void parse_rune(rune_node *result, parser *parser)
{
  UNIMPLEMENTED();
}

void parse_number(expression *result, parser *parser)
{
  ASSERT(parser->token.tag == token_tag_digital
         || parser->token.tag == token_tag_hexadecimal
         || parser->token.tag == token_tag_binary
         || parser->token.tag == token_tag_decimal);
  
  static thread_local utf8 string[80];
  uint string_size = get_token_size(parser);
  if (string_size >= countof(string))
  {
    report_token_failure(parser, "number is too long.");
    jump(*parser->failure_jump_point, 1);
  }
  copy_typed(utf8, string, get_token_pointer(parser), string_size);
  string[string_size] = 0;
  utf8 *string_ending;

  uintb base;
  switch (parser->token.tag)
  {
  case token_tag_binary:      base = 2;  break;
  case token_tag_digital:     base = 10; break;
  case token_tag_hexadecimal: base = 16; break;
  default:                    base = 0; break;
  }

  /* FIX: this isn't failure-checked */
  if (base)
  {
    result->tag = node_tag_digital;
    result->data->digital.value = strtoull(string, &string_ending, base);
  }
  else
  {
    result->tag = node_tag_decimal;
    result->data->decimal.value = strtod(string, &string_ending);
  }

  get_token(parser); /* skip number */
}

void parse_structure(structure_node *result, parser *parser)
{
  get_token(parser); /* get the first onset */

  /* TODO: upon the failure of an iteration, deallocate the allocated memory
           from the iteration, and skip to a valid onset. */

  result->declarations_count = 0;

  for (statement *prior_declaration = 0;;)
  {
    statement *next_declaration = 0;

    switch (parser->token.tag)
    {
    case token_tag_identifier:
      /* encountered a declaration */
      next_declaration = push_typed_train(statement, declaration_node, &parser->general_allocator);
      next_declaration->expression.tag = node_tag_declaration;
      parse_declaration(&next_declaration->expression.data->declaration, parser);
      break;
    case token_tag_semicolon:
      get_token(parser); /* ignore */
      break;
    case token_tag_right_brace:
      /* because a file scope is implicitely a structure, encountering `{` is
         erroneous if we're at the global scope.
      */
      if (parser->current_scope == &parser->program->global_scope)
      {
        report_token_failure(parser, "Encountered extraneous %s.", token_tag_representations[token_tag_right_brace]);
        goto failed;
      }
      else
      {
        get_token(parser);
        break;
      }
    default:
      report_token_failure(parser, "Expected %s, %s, or %s.",
                           token_tag_representations[token_tag_identifier],
                           token_tag_representations[token_tag_semicolon],
                           token_tag_representations[token_tag_right_brace]);
      goto failed;
    }

    /* link the next declaration */
    if (next_declaration)
    {
      if (prior_declaration) prior_declaration = prior_declaration->next = next_declaration;
      else result->declarations = prior_declaration = next_declaration;
      result->declarations_count += 1;
      report_token_comment(parser, "Parsed.");
    }

    continue;

  failed:
    jump(*parser->failure_jump_point, 1);
  }
}

void parse_procedure(procedure_node *result, parser *parser)
{
  
  UNIMPLEMENTED();
}

expression *parse_expression(precedence left_precedence, parser *parser)
{
  /* parse left */
  expression *left = 0;

  {
    switch (parser->token.tag)
    {
      /* structure */
    case token_tag_left_brace:
      left = push_typed_train(expression, structure_node, &parser->general_allocator);
      parse_structure(&left->data->structure, parser);
      goto finished;

    case token_tag_left_parenthesis:
      get_token(parser); /* skip `(` */
      left = parse_expression(0, parser);
      ensure_get_token(token_tag_right_parenthesis, parser);     
      break;

    case token_tag_identifier:
      left = push_typed_train(expression, identifier_node, &parser->general_allocator);
      left->tag = node_tag_identifier;
      parse_identifier(&left->data->identifier, parser);
      break;
      
    case token_tag_at:
      get_token(parser); /* skip `@` */
      left = push_typed_train(expression, unary_node, &parser->general_allocator);
      left->tag = node_tag_reference;
      left->data->unary.expression = parse_expression(0, parser);
      break;

    case token_tag_binary:
    case token_tag_digital:
    case token_tag_hexadecimal:
    case token_tag_decimal:
      left = push_typed_train(expression, digital_node, &parser->general_allocator);
      parse_number(left, parser);
      break;

    case token_tag_right_parenthesis:
      goto finished;
      
    default:
      UNIMPLEMENTED();
      break;
    }
  }

  /* handle a possibly chained expression */
  for (;;)
  {
    node_tag right_tag;
    switch (parser->token.tag)
    {
      /* procedure [type] */
    case token_tag_arrow:
      {
        get_token(parser); /* skip `->` */
        expression *arguments = left;
        left = parser->token.tag == token_tag_left_brace
             ? push_typed_train(expression, procedure_node, &parser->general_allocator)
             : push_typed_train(expression, procedure_type_node, &parser->general_allocator);
        left->tag = node_tag_procedure_type;
        left->data->procedure_type.arguments = arguments;
        left->data->procedure_type.results = parse_expression(0, parser);

        /* procedure type */
        if (parser->token.tag != token_tag_left_brace) goto finished;

        /* procedure */
    case token_tag_left_brace:
        left->tag = node_tag_procedure;
        parse_procedure(&left->data->procedure, parser);
        break;
      }
      
      /* logical */
    case token_tag_and2:                 right_tag = node_tag_conjunction;       break;
    case token_tag_bar2:                 right_tag = node_tag_disjunction;       break;
    case token_tag_equality2:            right_tag = node_tag_equality;          break;
    case token_tag_exclamation_equality: right_tag = node_tag_inequality;        break;
    case token_tag_right_angle:          right_tag = node_tag_greater;           break;
    case token_tag_left_angle:           right_tag = node_tag_lesser;            break;
    case token_tag_right_angle_equality: right_tag = node_tag_inclusive_greater; break;
    case token_tag_left_angle_equality:  right_tag = node_tag_inclusive_lesser;  break;
      
      /* arithmetic */
    case token_tag_plus:     right_tag = node_tag_addition;       break;
    case token_tag_dash:     right_tag = node_tag_subtraction;    break;
    case token_tag_asterisk: right_tag = node_tag_multiplication; break;
    case token_tag_slash:    right_tag = node_tag_division;       break;
    case token_tag_percent:  right_tag = node_tag_modulo;         break;

      /* bitwise */
    case token_tag_and:          right_tag = node_tag_bitwise_conjunction;           break;
    case token_tag_bar:          right_tag = node_tag_bitwise_disjunction;           break;
    case token_tag_caret:        right_tag = node_tag_bitwise_exclusive_disjunction; break;
    case token_tag_left_angle2:  right_tag = node_tag_bitwise_left_shift;            break;
    case token_tag_right_angle2: right_tag = node_tag_bitwise_right_shift;           break;

      /* assignment */
    case token_tag_equality:              right_tag = node_tag_assignment;                               break;
    case token_tag_plus_equality:         right_tag = node_tag_addition_assignment;                      break;
    case token_tag_minus_equality:        right_tag = node_tag_subtraction_assignment;                   break;
    case token_tag_asterisk_equality:     right_tag = node_tag_multiplication_assignment;                break;
    case token_tag_slash_equality:        right_tag = node_tag_division_assignment;                      break;
    case token_tag_percent_equality:      right_tag = node_tag_modulo_assignment;                        break;
    case token_tag_and_equality:          right_tag = node_tag_bitwise_conjunction_assignment;           break;
    case token_tag_bar_equality:          right_tag = node_tag_bitwise_disjunction_assignment;           break;
    case token_tag_caret_equality:        right_tag = node_tag_bitwise_exclusive_disjunction_assignment; break;
    case token_tag_left_angle2_equality:  right_tag = node_tag_bitwise_left_shift_assignment;            break;
    case token_tag_right_angle2_equality: right_tag = node_tag_bitwise_right_shift_assignment;           break;

      /* resolution */
    case token_tag_dot: right_tag = node_tag_resolution; break;
      
      /* other */
    case token_tag_comma:     right_tag = node_tag_list;       break;
    case token_tag_question:  right_tag = node_tag_condition;  break;
    case token_tag_semicolon:
    case token_tag_right_parenthesis:
      goto finished;
    default:                  right_tag = node_tag_invocation; break;
    }
    
    if (!left) goto finished;

    /* prevent chained expressions with procedures. */
    if (left->tag == node_tag_procedure && right_tag != node_tag_invocation) goto finished;

    /* check precedence */
    precedence right_precedence = precedences[right_tag];
    if (right_precedence <= left_precedence) goto finished;

    /* skip the operator */
    if (right_tag != node_tag_invocation) get_token(parser);

    expression *right = right_tag != node_tag_condition
          ? push_typed_train(expression, binary_node, &parser->general_allocator)
          : push_typed_train(expression, ternary_node, &parser->general_allocator);
    right->tag = right_tag;
    right->data->binary.left = left;
    if (right_tag != node_tag_condition)
    {
      right->data->binary.right = parse_expression(right_precedence, parser);
    }
    else /* the expression is ternary */
    {
      /* parse the right expression as a parenthesized expression */
      right->data->ternary.right = parse_expression(0, parser);

      /* parse a possible other expression */
      if (parser->token.tag == token_tag_colon)
      {
        get_token(parser); /* skip `:` */
        right->data->ternary.other = parse_expression(right_precedence, parser);
      }
    }
    
    left = right;
  }
  
finished:
  return left;
}

void parse(const utf8 *path, program *program, parser *parser)
{
  fill(parser, sizeof(*parser), 0);

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
  parse_structure(&parser->program->global_scope, parser);

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
    print_failure("A source path wasn't given.\n");
    return -1;
  }

  program program;

  parser parser;
  parse(arguments[1], &program, &parser);

  return 0;
}
