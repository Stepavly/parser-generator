//
// Created by stepavly on 13.12.2021.
//

#ifndef UTILS_PARSE_GENERATOR_
#define UTILS_PARSE_GENERATOR_
#include <string>
#include <vector>
#include <algorithm>

template<typename Collection>
inline std::string to_string(const Collection &collection) {
  std::string res;
  res += "{";
  for (auto it = collection.begin(); it != collection.end(); ++it) {
    if (it != collection.begin()) {
      res += ",";
    }
    res += "\"";
    res += *it;
    res += "\"";
  }
  res += "}";
  return res;
}

std::string escape(const std::string &s);

char unescape(char c);
#endif