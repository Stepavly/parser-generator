//
// Created by stepavly on 12.12.2021.
//

#include "rule_parser.h"
#include "../utils/utils.h"

rule_parser::rule_parser(std::string s, lexer_generator &lexer_generator)
  : lexer_(std::move(s))
  , lexer_generator_(lexer_generator) {}

rule rule_parser::parse() {
  rule cur_rule;
  parse_rule(cur_rule);
  return cur_rule;
}

void rule_parser::parse_rule(rule &cur_rule) {
  parse_rule_name(cur_rule);
  parse_inh_attr(cur_rule);
  if (lexer_.next().type != TOKEN_TYPE::COLON) {
    throw std::runtime_error("':' expected");
  }
  parse_rule_inner(cur_rule);
  if (lexer_.next().type != TOKEN_TYPE::END) {
    throw std::runtime_error("EOF expected");
  }
}

void rule_parser::parse_rule_name(rule &cur_rule) {
  auto token = lexer_.next();
  if (token.type != TOKEN_TYPE::ID) {
    throw std::runtime_error("Identifier expected");
  }
  cur_rule.set_name(token.data);
}

void rule_parser::parse_inh_attr(rule &cur_rule) {
  auto token = lexer_.next();
  if (token.type == TOKEN_TYPE::COLON) {
    lexer_.undo();
    return;
  } else if (token.type != TOKEN_TYPE::OPEN_SQUARE_BRACKET) {
    throw std::runtime_error("'[' expected");
  }

  while (true) {
    auto var_type = lexer_.next();
    if (var_type.type == TOKEN_TYPE::CLOSE_SQUARE_BRACKET) {
      return;
    } else if (var_type.type != TOKEN_TYPE::ID) {
      throw std::runtime_error("Identifier expected");
    }
    auto var_name = lexer_.next();
    if (var_name.type != TOKEN_TYPE::ID) {
      throw std::runtime_error("Identifier expected");
    }
    cur_rule.add_inh_var(var_type.data, var_name.data);
  }
}

void rule_parser::parse_rule_inner(rule &cur_rule) {
  while (true) {
    auto token = lexer_.next();
    if (token.type == TOKEN_TYPE::ID) {
      if (lexer_.is_rule_name(token.data)) {
        cur_rule.add_transition(token.data);
      } else if (lexer_.is_regex_name(token.data)) {
        cur_rule.add_transition_regex(token.data);
      } else {
        throw std::runtime_error("Nor non-terminal name nor regex name found: '" + token.data + "'");
      }
    } else if (token.type == TOKEN_TYPE::TEXT) {
      cur_rule.add_str(token.data);
      lexer_generator_.add_token(escape(token.data));
    } else if (token.type == TOKEN_TYPE::DOLLAR) {
      auto fun = lexer_.next();
      if (fun.type != TOKEN_TYPE::ID) {
        throw std::runtime_error("Identifier expected");
      }
      cur_rule.add_fun_call(fun.data);
    } else if (token.type == TOKEN_TYPE::OPEN_BRACKET) {
      auto var_name = lexer_.next();
      if (var_name.type != TOKEN_TYPE::ID) {
        throw std::runtime_error("Identifier expected");
      } else if (lexer_.next().type != TOKEN_TYPE::ASSIGN) {
        throw std::runtime_error("'=' expected");
      }

      auto rule_name = lexer_.next();
      if (rule_name.type != TOKEN_TYPE::ID && rule_name.type != TOKEN_TYPE::TEXT) {
        throw std::runtime_error("Identifier expected");
      } else if (lexer_.next().type != TOKEN_TYPE::CLOSE_BRACKET) {
        throw std::runtime_error("')' expected");
      }

      if (rule_name.type == TOKEN_TYPE::ID) {
        if (lexer_.is_rule_name(rule_name.data)) {
          cur_rule.add_assign_rule(var_name.data, rule_name.data);
        } else if (lexer_.is_regex_name(rule_name.data)) {
          cur_rule.add_assign_regex(var_name.data, rule_name.data);
        } else {
          throw std::runtime_error("Nor non-terminal name nor regex name found: '" + rule_name.data + "'");
        }
      } else {
        cur_rule.add_assign_text(var_name.data, rule_name.data);
        lexer_generator_.add_token(escape(rule_name.data));
      }
    } else {
      throw std::runtime_error("Unknown token");
    }

    auto next_type = lexer_.next().type;
    if (next_type == TOKEN_TYPE::END) {
      break;
    }
    lexer_.undo();
  }
}
