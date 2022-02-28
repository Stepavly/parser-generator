//
// Created by stepavly on 12.12.2021.
//

#include "lexer.h"
#include <utility>
#include <stdexcept>
#include <algorithm>
#include "../utils/utils.h"

token::token(TOKEN_TYPE type, std::string data)
  : type(type)
  , data(std::move(data)) {}

lexer::lexer(std::string data)
  : data(std::move(data))
  , pos(0)
  , prev_pos(0) {}

bool is_identifier_tail(char c) {
  return std::isalnum(c) || c == '_';
}

bool is_identifier_start(char c) {
  return std::isalpha(c);
}

std::string lexer::extract_id() {
  std::string res;
  if (is_identifier_start(data[pos])) {
    res += data[pos++];
    for (; pos < data.size() && is_identifier_tail(data[pos]); pos++) {
      res += data[pos];
    }
  }
  return res;
}

token lexer::next() {
  for (prev_pos = pos; pos < data.size() && std::isspace(data[pos]); pos++) {}
  if (pos >= data.size()) {
    return {TOKEN_TYPE::END, ""};
  }

  if (data[pos] == '(') {
    return {TOKEN_TYPE::OPEN_BRACKET, {data[pos++]}};
  } else if (data[pos] == ')') {
    return {TOKEN_TYPE::CLOSE_BRACKET, {data[pos++]}};
  } else if (data[pos] == ',') {
    return {TOKEN_TYPE::COMMA, {data[pos++]}};
  } else if (data[pos] == ':') {
    return {TOKEN_TYPE::COLON, {data[pos++]}};
  } else if (data[pos] == '[') {
    return {TOKEN_TYPE::OPEN_SQUARE_BRACKET, {data[pos++]}};
  } else if (data[pos] == ']') {
    return {TOKEN_TYPE::CLOSE_SQUARE_BRACKET, {data[pos++]}};
  } else if (data[pos] == '$') {
    return {TOKEN_TYPE::DOLLAR, {data[pos++]}};
  } else if (data[pos] == '\'') {
    std::string text;
    for (pos++; pos < data.size(); ) {
      if (data[pos] == '\\') {
        if (pos == data.size() - 1) {
          throw std::logic_error("Escaping character expected in\n" + data.substr(pos));
        }
        text += unescape(data[pos + 1]);
        pos += 2;
        continue;
      }
      if (data[pos] != '\'') {
        text += data[pos++];
      } else {
        pos++;
        if (text.empty()) {
          throw std::logic_error("Strings must be non-empty, use EPS instead of them");
        }
        return {TOKEN_TYPE::TEXT, text};
      }
    }
    throw std::logic_error("Closing apostrophe expected in\n" + data.substr(prev_pos));
  } else if (data[pos] == '=') {
    return {TOKEN_TYPE::ASSIGN, {data[pos++]}};
  }

  std::string id = extract_id();
  if (id.empty()) {
    throw std::logic_error("unknown token");
  } else {
    return {TOKEN_TYPE::ID, id};
  }
}

void lexer::undo() {
  pos = prev_pos;
}

bool lexer::is_regex_name(const std::string &name) const {
  return std::all_of(name.begin(), name.end(), [](auto c) {
    return c == '_' || (std::isalpha(c) && std::isupper(c));
  });
}

bool lexer::is_rule_name(const std::string &name) const {
  return std::all_of(name.begin(), name.end(), [](auto c) {
    return c == '_' || (std::isalpha(c) && std::islower(c));
  });
}
