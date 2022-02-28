//
// Created by stepavly on 27.02.2022.
//

#ifndef PARSER_GENERATOR_PARSER_RULE_CFG_H_
#define PARSER_GENERATOR_PARSER_RULE_CFG_H_

#include <vector>
#include <string>
#include "rule_utils.h"

struct rule_cfg {
  std::string name;
  std::vector<std::pair<RULE_TYPE, std::string>> data;

  rule_cfg(const std::string &name, const std::vector<std::pair<RULE_TYPE, std::string>> &data);
};

#endif //PARSER_GENERATOR_PARSER_RULE_CFG_H_
