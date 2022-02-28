//
// Created by stepavly on 13.12.2021.
//

#include "lexer_generator.h"
#include "../utils/utils.h"

void lexer_generator::add_token(const std::string &token_text) {
  tokens.insert(token_text);
}

void lexer_generator::add_regex(const std::string &regex_name, const std::string &regex) {
  if (regex_name == "END") {
    throw std::runtime_error("END is reserved name for regex");
  }
  if (!regexes.emplace(regex_name, regex).second) {
    throw std::runtime_error("Duplicate regex name:\n" + regex_name);
  }
}

std::string generate_token_types(const std::unordered_map<std::string, std::string> &regexes) {
  std::string res;
  res += "enum class TOKEN_TYPE {\n";
  for (const auto&[regex_name, regex]: regexes) {
    res += "\t" + regex_name + ",\n";
  }
  res +=
    "\tEND\n,"
    "\tTEXT\n"
    "};\n";
  return res;
}

std::string generate_regexes(const std::unordered_map<std::string, std::string> &regexes) {
  std::string res;
  for (auto it = regexes.begin(); it != regexes.end(); ++it) {
    if (it != regexes.begin()) {
      res += ", ";
    }
    res += "{TOKEN_TYPE::" + it->first + ", std::regex(\"" + it->second + "\")}";
  }
  return "{" + res + "}";
}

std::string lexer_generator::generate() {
  size_t max_token_len = 0;
  for (const auto &token: tokens) {
    max_token_len = std::max(max_token_len, token.size());
  }

  std::string code =
    generate_token_types(regexes) +
    "class lexer {\n"
    " public:\n"
    "\n"
    "\tvoid undo() {\n"
    "\t\tpos = prev_pos;\n"
    "\t}\n"
    "\n"
    "\tstd::pair<TOKEN_TYPE, std::string> next() {\n"
    "\t\tprev_pos = pos;\n"
    "\t\tif (pos == data.size()) {\n"
    "\t\t\treturn {TOKEN_TYPE::END, \"\"};\n"
    "\t\t}\n"
    "\t\tauto regex_candidate = extract_regex();\n"
    "\t\tauto token_candidate = extract_token();\n"
    "\t\tif (regex_candidate.second.size() > token_candidate.second.size()) {\n"
    "\t\t\tpos += regex_candidate.second.size();\n"
    "\t\t\treturn regex_candidate;\n"
    "\t\t} else if (!token_candidate.second.empty()) {\n"
    "\t\t\tpos += token_candidate.second.size();\n"
    "\t\t\treturn token_candidate;\n"
    "\t\t}\n"
    "\t\tthrow std::runtime_error(\"Unexpected token\");\n"
    "\t}\n"
    "\n"
    "\tlexer(std::string data)\n"
    "\t\t: prev_pos(0)\n"
    "\t\t, pos(0)\n"
    "\t\t, data(std::move(data))\n"
    "\t\t, tokens(" + to_string(tokens) + ")\n"
    "\t\t, regexes(" + generate_regexes(regexes) + ") {}\n"
    "\n"
    " private:\n"
    "\n"
    "\tstd::pair<TOKEN_TYPE, std::string> extract_regex() {\n"
    "\t\tstd::pair<TOKEN_TYPE, std::string> res;\n"
    "\t\tfor (const auto&[type, regex]: regexes) {\n"
    "\t\t\tstd::smatch sm;\n"
    "\t\t\tauto sdata = data.substr(pos);\n"
    "\t\t\tstd::regex_search(sdata, sm, regex, std::regex_constants::match_continuous);\n"
    "\t\t\tif (!sm.empty() && res.second.size() < sm.length(0)) {\n"
    "\t\t\t\tres = {type, sm[0]};\n"
    "\t\t\t}\n"
    "\t\t}\n"
    "\t\treturn res;\n"
    "\t}\n"
    "\n"
    "\tstd::pair<TOKEN_TYPE, std::string> extract_token() {\n";
  if (tokens.empty()) {
    code += "\t\treturn {};\n";
  } else {
    code +=
      "\t\tfor (size_t cur_pos = std::min(pos + " + std::to_string(max_token_len - 1) + ", data.size() - 1);; cur_pos--) {\n"
      "\t\t\tstd::string cur_token = data.substr(pos, cur_pos - pos + 1);\n"
      "\t\t\tif (tokens.count(cur_token)) {\n"
      "\t\t\t\treturn {TOKEN_TYPE::TEXT, cur_token};\n"
      "\t\t\t}\n"
      "\t\t\tif (cur_pos == pos) {\n"
      "\t\t\t\treturn {};\n"
      "\t\t\t}\n"
      "\t\t}\n";
  }
  code +=
    "\t}\n"
    "\n"
    "\tsize_t prev_pos, pos;\n"
    "\tstd::string data;\n"
    "\tstd::unordered_set<std::string> tokens;\n"
    "\tstd::vector<std::pair<TOKEN_TYPE, std::regex>> regexes;\n"
    "};\n\n";
  return code;
}
