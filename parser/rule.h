//
// Created by stepavly on 13.12.2021.
//

#ifndef PARSER_GENERATOR_PARSER_RULE_H_
#define PARSER_GENERATOR_PARSER_RULE_H_

#include <string>
#include <utility>
#include <vector>
#include <set>
#include <variant>
#include <unordered_map>
#include "../grammar/grammar_analyzer.h"
#include "rule_cfg.h"
#include "rule_utils.h"

using var_t = std::pair<std::string, std::string>;
using rule_token_t = std::variant<std::string, var_t>;

struct rule {
 public:
  rule();

  void add_var(const std::string &var_type, const std::string &var_name);
  void add_inh_var(const std::string &var_type, const std::string &var_name);
  void add_exported_var(const std::string &var_type, const std::string &var_name);
  void add_fun_code(const std::string &name, const std::string &code);
  void add_assign_rule(const std::string &var_name, const std::string &rule);
  void add_assign_text(const std::string &var_name, const std::string &text);
  void add_assign_regex(const std::string &var_name, const std::string &regex_name);
  void add_transition(const std::string &rule);
  void add_transition_regex(const std::string &regex_name);
  void add_fun_call(const std::string &fun);
  void add_str(const std::string &str);
  void set_name(const std::string &name);

  const std::string &get_name() const;
  std::vector<var_t> get_assigns() const;
  const std::set<var_t> &get_exported_vars() const;
  std::vector<std::string> get_fun_names() const;

  void merge(rule& other);

  std::pair<std::string, std::string> generate_class(
    const std::unordered_map<std::string, std::string> &constructors,
    const std::unordered_map<std::string, std::set<std::string>> &exported_vars_names,
    grammar_analyzer &analyzer) const;
  rule_cfg generate_cfg() const;
  std::string generate_constructor() const;

 private:
  std::string rule_name;
  std::set<var_t> vars;
  std::vector<var_t> inh_vars;
  std::set<var_t> exported_vars;
  std::unordered_map<std::string, std::string> funs;
  std::vector<std::vector<std::pair<RULE_TYPE, rule_token_t>>> rules_;
};

#endif //PARSER_GENERATOR_PARSER_RULE_H_
