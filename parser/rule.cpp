//
// Created by stepavly on 13.12.2021.
//

#include <algorithm>
#include <unordered_set>
#include "rule.h"
#include "../utils/utils.h"
#include <cassert>

const static size_t NON_ASSIGN_TYPE = 0;
const static size_t ASSIGN_TYPE = 1;

rule::rule()
  : rules_({{}}) {}

/**
 * Add
 */
void rule::add_var(const std::string &var_type, const std::string &var_name) {
  vars.emplace(var_type, var_name);
}

/**
 * Add var type and name inherited by this non-terminal
 */
void rule::add_inh_var(const std::string &var_type, const std::string &var_name) {
  vars.emplace(var_type, var_name);
  inh_vars.emplace_back(var_type, var_name);
}

/**
 * Add var type and name exported from this non-terminal
 */
void rule::add_exported_var(const std::string &var_type, const std::string &var_name) {
  vars.emplace(var_type, var_name);
  exported_vars.emplace(var_type, var_name);
}

/**
 * Add fun code called from this non-terminal
 */
void rule::add_fun_code(const std::string &name, const std::string &code) {
  funs.emplace(name, code);
}

/**
 * Add non-terminal rule assign to variable
 */
void rule::add_assign_rule(const std::string &var_name, const std::string &rule) {
  rules_.back().emplace_back(RULE_TYPE::ASSIGN_RULE, var_t(var_name, rule));
}

/**
 * Add text assign to variable
 */
void rule::add_assign_text(const std::string &var_name, const std::string &text) {
  rules_.back().emplace_back(RULE_TYPE::ASSIGN_TEXT, var_t(var_name, text));
  vars.emplace("std::string", var_name);
}

void rule::add_assign_regex(const std::string &var_name, const std::string &regex_name) {
  rules_.back().emplace_back(RULE_TYPE::ASSIGN_REGEX, var_t(var_name, regex_name));
  vars.emplace("std::string", var_name);
}

/**
 * Add other non-terminal rule transition
 */
void rule::add_transition(const std::string &rule) {
  rules_.back().emplace_back(RULE_TYPE::TRANSITION, rule);
}

/**
 * Add regex transition
 */
void rule::add_transition_regex(const std::string &regex_name) {
  rules_.back().emplace_back(RULE_TYPE::TRANSITION_REGEX, regex_name);
}

/**
 * Add function call
 */
void rule::add_fun_call(const std::string &fun) {
  assert(rules_.size() == 1);
  rules_.back().emplace_back(RULE_TYPE::CALL, fun);
}

/**
 * Add string transition
 */
void rule::add_str(const std::string &str) {
  assert(rules_.size() == 1);
  rules_.back().emplace_back(RULE_TYPE::TEXT, str);
}

/**
 * Set non-terminal name
 */
void rule::set_name(const std::string &name) {
  rule_name = name;
}

/**
 * Get non-terminal name
 */
const std::string &rule::get_name() const {
  return rule_name;
}

/**
 * Merge two equally named non-terminal rules, namely merges vars info
 */
void rule::merge(rule &other) {
  assert(rule_name == other.rule_name);
  set_name(other.rule_name);
  vars.merge(other.vars);
  exported_vars.merge(other.exported_vars);
  rules_.insert(rules_.end(), other.rules_.begin(), other.rules_.end());
}

std::string generate_rule(
  const std::vector<std::pair<RULE_TYPE, rule_token_t>> &rule,
  const std::string &name,
  const std::unordered_map<std::string, std::string> &constructors,
  const std::unordered_map<std::string, std::set<std::string>> &exported_vars_names) {
  std::string code;
  code +=
    "void " + name + "() {\n"
    "\tstd::pair<TOKEN_TYPE, std::string> token;\n";
  for (const auto&[type, data]: rule) {
    if (type == RULE_TYPE::TEXT) {
      const std::string &data_ = std::get<NON_ASSIGN_TYPE>(data);
      code +=
        "\ttoken = lexer_.next();\n"
        "\tif (token.first != TOKEN_TYPE::TEXT || token.second != \"" + escape(data_) + "\") {\n"
        "\t\tthrow std::runtime_error(\"Found token '\" + token.second + \"' but expected '" + escape(data_) + "'\");\n"
        "\t}\n"
        "\t{\n"
        "\t\tauto node = std::make_shared<text_node>(token.second);\n"
        "\t\tadd_child(node);\n"
        "\t}\n";
    } else if (type == RULE_TYPE::TRANSITION) {
      const std::string &data_ = std::get<NON_ASSIGN_TYPE>(data);
      code +=
        "\t{\n"
        "\t\tauto node = std::make_shared<" + data_ + "_node>" + constructors.find(data_)->second + ";\n"
        "\t\tnode->parse();\n"
        "\t\tadd_child(node);\n"
        "\t}\n";
    } else if (type == RULE_TYPE::TRANSITION_REGEX) {
      const std::string &data_ = std::get<NON_ASSIGN_TYPE>(data);
      if (data_ == "EPS") {
        continue;
      }
      code +=
        "\ttoken = lexer_.next();\n"
        "\tif (token.first != TOKEN_TYPE::" + data_ + ") {\n"
        "\t\tthrow std::runtime_error(\"Found unexpected token, expected " + data_ + " \");\n"
        "\t}\n"
        "\t{\n"
        "\t\tauto node = std::make_shared<text_node>(token.second);\n"
        "\t\tadd_child(node);\n"
        "\t}\n";
    } else if (type == RULE_TYPE::ASSIGN_RULE) {
      const auto&[var_name, assign_rule] = std::get<ASSIGN_TYPE>(data);
      code +=
        "\t{\n"
        "\t\tauto node = std::make_shared<" + assign_rule + "_node>" + constructors.find(assign_rule)->second + ";\n"
        "\t\tnode->parse();\n"
        "\t\tadd_child(node);\n";
      for (const auto &exported_var: exported_vars_names.find(assign_rule)->second) {
        code += "\t\t" + var_name + "_" + exported_var + " = node->" + exported_var + ";\n";
      }
      code += "\t}\n";
    } else if (type == RULE_TYPE::ASSIGN_TEXT) {
      const auto&[var_name, assign_text] = std::get<ASSIGN_TYPE>(data);
      code +=
        "\ttoken = lexer_.next();\n"
        "\tif (token.first != TOKEN_TYPE::TEXT || token.second != \"" + escape(assign_text) + "\") {\n"
        "\t\tthrow std::runtime_error(\"Found token '\" + token.second + \"' but expected '" + escape(assign_text) + "'\");\n"
        "\t}"
        "\t{\n"
        "\t\t" + var_name + " = token.second;\n"
        "\t}\n";
    } else if (type == RULE_TYPE::ASSIGN_REGEX) {
      const auto&[var_name, regex_name] = std::get<ASSIGN_TYPE>(data);
      code +=
        "\ttoken = lexer_.next();\n"
        "\tif (token.first != TOKEN_TYPE::" + regex_name + ") {\n"
        "\t\tthrow std::runtime_error(\"Found unexpected token, expected " + regex_name + "\");\n"
        "\t}\n"
        "\t{\n"
        "\t\tauto node = std::make_shared<text_node>(token.second);\n"
        "\t\tadd_child(node);\n"
        "\t\t" + var_name + " = token.second;\n"
        "\t}\n";
    } else { // RULE_TYPE::CALL
      code += "\t" + std::get<NON_ASSIGN_TYPE>(data) + "();\n";
    }
  }
  code += "}";
  return code;
}

std::string rule::generate_constructor() const {
  std::string code = "(";
  for (size_t i = 0; i < inh_vars.size(); i++) {
    if (i != 0) {
      code += ", ";
    }
    code += inh_vars[i].second;
  }
  code += ")";
  return code;
}

std::pair<std::string, std::string> rule::generate_class(
  const std::unordered_map<std::string, std::string> &constructors,
  const std::unordered_map<std::string, std::set<std::string>> &exported_vars_names,
  grammar_analyzer &analyzer) const {
  std::string code, struct_name = rule_name + "_node";
  code += "struct " + struct_name + " : public inner_node {\n";
  ////////// VARIABLES GENERATION //////////
  for (const auto&[var_type, var_name]: vars) {
    code += "\t" + var_type + " " + var_name + ";\n";
  }

  ////////// CONSTRUCTOR CODE GENERATION //////////
  code += "\n"
          "\t" + struct_name + "(";
  std::set<std::string> cons_arg;
  for (size_t i = 0; i < inh_vars.size(); i++) {
    if (i != 0) {
      code += ", ";
    }
    code += inh_vars[i].first + " _" + inh_vars[i].second;
    cons_arg.insert(inh_vars[i].second);
  }
  code += ") {\n";
  for (const auto&[var_type, var_name]: vars) {
    if (cons_arg.find(var_name) != cons_arg.end()) {
      code += "\t\t" + var_name + " = _" + var_name + ";\n";
    }
  }
  code += "\t}\n";

  ////////// PARSER CODE GENERATION //////////
  std::string parse_code;
  code +=
    "\tvoid visit() override;\n"
    "\tvoid parse() override;\n";
  parse_code +=
    "void " + struct_name + "::parse() {\n"
    "\tauto token = lexer_.next();\n";
  size_t rule_id = 0;
  std::string rules_code, other_funs;
  std::optional<std::string> eps_rule;
  for (const auto &cur_rule: rules_) {
    std::vector<std::string> filtered_rule;
    for (const auto&[type, rule]: cur_rule) {
      if (type == RULE_TYPE::TRANSITION || type == RULE_TYPE::TRANSITION_REGEX) {
        filtered_rule.push_back(std::get<NON_ASSIGN_TYPE>(rule));
      } else if (type == RULE_TYPE::ASSIGN_RULE || type == RULE_TYPE::ASSIGN_REGEX) {
        filtered_rule.push_back(std::get<ASSIGN_TYPE>(rule).second);
      } else if (type == RULE_TYPE::TEXT) {
        filtered_rule.push_back("'" + std::get<NON_ASSIGN_TYPE>(rule) + "'");
      } else if (type == RULE_TYPE::ASSIGN_TEXT) {
        filtered_rule.push_back("'" + std::get<ASSIGN_TYPE>(rule).second + "'");
      }
    }

    for (const auto &f: analyzer.get_first(filtered_rule)) {
      if (f == "EPS") {
        eps_rule = "parse_" + std::to_string(rule_id);
      } else {
        if (f[0] == '\'' && f.back() == '\'') {
          parse_code +=
            "\tif (token.first == TOKEN_TYPE::TEXT && token.second == \"" + f.substr(1, f.size() - 2) + "\") {\n"
            "\t\tlexer_.undo();\n"
            "\t\tparse_" + std::to_string(rule_id) + "();\n"
            "\t\treturn;\n"
            "\t}\n";
        } else {
          parse_code +=
            "\tif (token.first == TOKEN_TYPE::" + f + ") {\n"
            "\t\tlexer_.undo();\n"
            "\t\tparse_" + std::to_string(rule_id) + "();\n"
            "\t\treturn;\n"
            "\t}\n";
        }
      }
    }

    other_funs += "\tvoid parse_" + std::to_string(rule_id) + "();\n";
    rules_code += generate_rule(cur_rule,
                                struct_name + "::parse_" + std::to_string(rule_id++),
                                constructors,
                                exported_vars_names);
    rules_code += "\n\n";
  }

  if (eps_rule.has_value()) {
    for (auto f: analyzer.get_follow(rule_name)) {
      if (f[0] == '\'' && f.back() == '\'') {
        f = f.substr(1, f.size() - 2); // Remove quotes from begin and end
        parse_code +=
          "\tif (token.first == TOKEN_TYPE::TEXT && token.second == \"" + f + "\") {\n"
          "\t\tlexer_.undo();\n"
          "\t\t" + *eps_rule + "();\n"
          "\t\treturn;\n"
          "\t}\n";
      } else {
        parse_code +=
          "\tif (token.first == TOKEN_TYPE::" + f + ") {\n"
          "\t\tlexer_.undo();\n"
          "\t\t" + *eps_rule + "();\n"
          "\t\treturn;\n"
          "\t}\n";
      }
    }
  }
  parse_code +=
    "\tlexer_.undo();\n"
    "}\n";
  code += other_funs;

  ////////// ATTRIBUTES CODE GENERATION //////////
  for (const auto &[attr_name, attr_code]: funs) {
    code +=
      "\n\n\tvoid " + attr_name + "() {\n" +
      attr_code +
      "\t}";
  }
  code += "\n};\n";

  ////////// VISITOR CODE GENERATION //////////
  rules_code += "void " + struct_name + "::visit() {}";
  return {code, parse_code + rules_code};
}

const std::set<var_t> &rule::get_exported_vars() const {
  return exported_vars;
}

std::vector<var_t> rule::get_assigns() const {
  std::vector<var_t> assigns;
  for (const auto &rule: rules_) {
    for (const auto&[type, token]: rule) {
      if (type == RULE_TYPE::ASSIGN_RULE) {
        assigns.emplace_back(std::get<ASSIGN_TYPE>(token));
      }
    }
  }
  return assigns;
}

std::vector<std::string> rule::get_fun_names() const {
  std::unordered_set<std::string> fun_names;
  for (const auto &rule: rules_) {
    for (const auto&[type, token]: rule) {
      if (type == RULE_TYPE::CALL) {
        fun_names.insert(std::get<NON_ASSIGN_TYPE>(token));
      }
    }
  }
  std::vector<std::string> res;
  res.reserve(fun_names.size());
  res.insert(res.begin(), fun_names.begin(), fun_names.end());
  return res;
}

rule_cfg rule::generate_cfg() const {
  std::vector<std::pair<RULE_TYPE, std::string>> cfg;
  for (const auto&[type, rule]: rules_.back()) {
    if (type == RULE_TYPE::TRANSITION) {
      cfg.emplace_back(RULE_TYPE::TRANSITION, std::get<NON_ASSIGN_TYPE>(rule));
    } else if (type == RULE_TYPE::ASSIGN_RULE) {
      cfg.emplace_back(RULE_TYPE::TRANSITION, std::get<ASSIGN_TYPE>(rule).second);
    } else if (type == RULE_TYPE::TRANSITION_REGEX) {
      cfg.emplace_back(RULE_TYPE::TRANSITION_REGEX, std::get<NON_ASSIGN_TYPE>(rule));
    } else if (type == RULE_TYPE::ASSIGN_REGEX) {
      cfg.emplace_back(RULE_TYPE::TRANSITION_REGEX, std::get<ASSIGN_TYPE>(rule).second);
    } else if (type == RULE_TYPE::TEXT) {
      cfg.emplace_back(RULE_TYPE::TEXT, "'" + std::get<NON_ASSIGN_TYPE>(rule) + "'");
    } else if (type == RULE_TYPE::ASSIGN_TEXT) {
      cfg.emplace_back(RULE_TYPE::TEXT, "'" + std::get<ASSIGN_TYPE>(rule).second + "'");
    }
  }
  return rule_cfg(rule_name, cfg);
}
