//
// Created by stepavly on 28.02.2022.
//

#include "utils.h"

std::string escape(const std::string &s) {
  std::string res;
  for (size_t i = 0; i < s.size(); i++) {
    switch (s[i]) {
      case '\n':
        res += "\\n";
        break;
      case '\\':
        res += "\\\\";
        break;
      case '\r':
        res += "\\r";
        break;
      case '\t':
        res += "\\t";
        break;
      default:
        res += s[i];
        break;
    }
  }
  return res;
}

char unescape(char c) {
  switch (c) {
    case 'n':
      return '\n';
    case 'r':
      return '\r';
    case '\'':
      return '\'';
    case '"':
      return '"';
    case 't':
      return '\t';
    case '\\':
      return '\\';
    default:
      throw std::runtime_error(std::string("Failed to unescape character \\") + c);
  }
}