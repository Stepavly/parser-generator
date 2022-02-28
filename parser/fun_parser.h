//
// Created by stepavly on 21.02.2022.
//

#ifndef PARSER_GENERATOR_PARSER_FUN_PARSER_H_
#define PARSER_GENERATOR_PARSER_FUN_PARSER_H_

#include "fun.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <algorithm>

static std::vector<std::string> parse_params(std::stringstream &ss) {
  std::vector<std::string> ids;
  ids.emplace_back();
  while (!ss.eof()) {
    auto c = ss.get();
    if (c == ' ' || c == ',') {
      if (!ids.back().empty()) {
        ids.emplace_back();
      }
    } else if (c == ']') {
      if (ids.back().empty()) {
        ids.pop_back();
      }
      break;
    } else {
      ids.back() += char(c);
    }
  }
  return ids;
}

void parse_fun(const std::string &header, std::ifstream &in, std::unordered_map<std::string, fun>& funs) {
  std::stringstream ss(header);
  ss.get(); // Skip '$'
  std::string fun_name;
  while (!ss.eof() && (fun_name += char(ss.get())).back() != '[') {} // read name until '['
  if (fun_name.empty() || fun_name.back() != '[') {
    throw std::runtime_error( "Expected '[' after " + fun_name);
  }
  fun_name.pop_back();

  std::vector<var_t> vars;
  auto attr_params = parse_params(ss);
  if (attr_params.size() % 2 != 0) {
    throw std::runtime_error("Type or name of variable missed");
  }
  for (size_t i = 0; i < attr_params.size(); i += 2) {
    vars.emplace_back(attr_params[i], attr_params[i + 1]);
  }

  size_t bracket_balance = 1;
  std::string code, line;
  while (getline(in, line)) {
    for (auto c: line) {
      code += c;
      if (c == '{') {
        bracket_balance++;
      } else if (c == '}') {
        bracket_balance--;
      }
    }
    if (bracket_balance == 0) {
      code.pop_back();
      break;
    }
    code += '\n';
  }

  if (bracket_balance != 0) {
    throw std::runtime_error("Invalid bracket sequence in fun " + fun_name);
  }
  if (!funs.emplace(fun_name, fun(code, vars)).second) {
    throw std::runtime_error("Duplicated fun names (" + fun_name + ")");
  }
}

#endif //PARSER_GENERATOR_PARSER_FUN_PARSER_H_
