//
// Created by stepavly on 12.12.2021.
//

#ifndef PARSER_GENERATOR_GRAMMAR_GRAMMAR_ANALYZER_H_
#define PARSER_GENERATOR_GRAMMAR_GRAMMAR_ANALYZER_H_

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <set>
#include "../parser/rule_cfg.h"

class grammar_analyzer {
 public:
  grammar_analyzer(const std::vector<rule_cfg>& rules_cfg, const std::string& start);

  std::set<std::string> get_first(const std::vector<std::string> &rule);
  std::set<std::string> get_follow(const std::string &non_term);

 private:
  std::map<std::string, std::vector<std::vector<std::string>>> rules;
  std::set<std::string> terminals;
  std::map<std::vector<std::string>, std::set<std::string>> first;
  std::map<std::string, std::set<std::string>> follow;
  std::string start;

  void init_first();
  void init_follow();

  void check_ll1();
};

#endif //PARSER_GENERATOR_GRAMMAR_GRAMMAR_ANALYZER_H_
