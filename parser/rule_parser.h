//
// Created by stepavly on 12.12.2021.
//

#ifndef PARSER_GENERATOR_PARSER_RULE_PARSER_H_
#define PARSER_GENERATOR_PARSER_RULE_PARSER_H_

#include <string>
#include "rule.h"
#include "lexer.h"
#include "../generators/lexer_generator.h"

class rule_parser {
 public:

  explicit rule_parser(std::string s, lexer_generator &lexer_generator);

  rule parse();

 private:

  lexer lexer_;
  lexer_generator &lexer_generator_;

  void parse_rule(rule &cur_rule);
  void parse_rule_name(rule &cur_rule);
  void parse_inh_attr(rule &cur_rule);
  void parse_rule_inner(rule &cur_rule);
};

#endif //PARSER_GENERATOR_PARSER_RULE_PARSER_H_
