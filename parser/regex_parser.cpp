//
// Created by stepavly on 28.02.2022.
//

#include <stdexcept>
#include "regex_parser.h"
#include "lexer.h"

std::pair<std::string, std::string> parse_regex(const std::string &line) {
  lexer lexer_(line);
  std::string name, regex;
  token t = lexer_.next();
  if (t.type != TOKEN_TYPE::ID || !lexer_.is_regex_name(t.data)) {
    throw std::runtime_error("Expected regex name, but found \"" + t.data + "\"");
  }
  name = std::move(t.data);

  t = lexer_.next();
  if (t.type != TOKEN_TYPE::COLON) {
    throw std::runtime_error("Expected colon after regex name, but found \"" + t.data + "\"");
  }

  t = lexer_.next();
  if (t.type != TOKEN_TYPE::TEXT) {
    throw std::runtime_error("Expected regex after regex name, but found \"" + t.data + "\"");
  }
  regex = std::move(t.data);

  if (lexer_.next().type != TOKEN_TYPE::END) {
    throw std::runtime_error("Unexpected data found after regex\n" + line);
  }
  return {name, regex};
}
