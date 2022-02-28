//
// Created by stepavly on 13.12.2021.
//

#ifndef PARSER_GENERATOR_GENERATORS_LEXER_GENERATOR_H_
#define PARSER_GENERATOR_GENERATORS_LEXER_GENERATOR_H_

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <regex>

class lexer_generator {
 public:
  lexer_generator() = default;

  void add_token(const std::string& token_text);
  void add_regex(const std::string& regex_name, const std::string& regex);

  std::string generate();

 private:
  std::unordered_set<std::string> tokens;
  std::unordered_map<std::string, std::string> regexes;
};

#endif //PARSER_GENERATOR_GENERATORS_LEXER_GENERATOR_H_
