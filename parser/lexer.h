//
// Created by stepavly on 12.12.2021.
//

#ifndef PARSER_GENERATOR_PARSER_LEXER_H_
#define PARSER_GENERATOR_PARSER_LEXER_H_

#include <string>

enum class TOKEN_TYPE {
  ID,
  COMMA,
  COLON,
  OPEN_BRACKET,
  CLOSE_BRACKET,
  OPEN_SQUARE_BRACKET,
  CLOSE_SQUARE_BRACKET,
  DOLLAR,
  ASSIGN,
  TEXT,
  END
};

struct token {
  TOKEN_TYPE type;
  std::string data;

  token(TOKEN_TYPE type, std::string data);
};

struct lexer {
 public:
  token next();
  void undo();
  explicit lexer(std::string data);

  bool is_regex_name(const std::string& name) const;
  bool is_rule_name(const std::string& name) const;

 private:
  std::string data;
  size_t pos, prev_pos;

  std::string extract_id();
};

#endif //PARSER_GENERATOR_PARSER_LEXER_H_
