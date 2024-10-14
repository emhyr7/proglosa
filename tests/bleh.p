parse_declaration :: (declaration: @declaration_node, parser: @parser) {
  assert parser.token.tag == token_tag.identifier;

  symbol: @symbol = declaration.symbol = push_type symbol, 1, &parser.general_allocator;
  symbol.declaration = declaration;
  symbol.identifier_size = get_token_size parser;
  symbol.identifier = parse_identifier @symbol.identifier_size, parser;
  ensure_token token_tag.colon, parser;
  get_token parser;
  symbol.type_definition = parse_type_definition parser;

  symbol.is_constant = 0;
  parser.token.tag == token_tag.colon ? {
    symbol.is_constant = 1;
    jump_to parse_assignment;
  }
  : parser.token.tag == token_tag.equality ? {
  [parse_assignment]:
    get_token parser;
    symbol.assignment = parse_expression precedence.default, parser;
  }
  : !.type_definition ? {
    symbol.assignment = parse_expression precedence.default, parser;
    !symbol.assignment ? {
      report_token_failure parser, "A declaration with an implicit type must have an assignment.";
      switch_to [parser.failure_jump_point], 1;
    };
  };
}

/*
parse_declaration :: (declaration: @declaration_node, parser: @parser)
{
  assert parser.token.tag == .identifier;

  symbol: @symbol = declaration.symbol = push_type symbol, 1, &parser.general_allocator;
  symbol {
    .declaration = declaration;
    .identifier_size = get_token_size parser;
    .identifier = parse_identifier @.identifier_size, parser;
    ensure_token 'colon, parser;
    get_token parser;
    .type_definition = parse_type_definition parser;

    .is_constant = 0;
    parser.token.tag == 'colon ? {
      .is_constant = 1;
      jump_to parse_assignment;
    }
    : parser.token.tag == 'equality ? {
    [parse_assignment]:
      get_token parser;
      .assignment = parse_expression 'default, parser;
    }
    : !.type_definition ? {
      .assignment = parse_expression 'default, parser;
      !.assignment ? {
        report_token_failure parser, "A declaration with an implicit type must have an assignment.";
        switch_to [parser.failure_jump_point], 1;
      };
    };
  };
}
*/
