//
// Created by stepavly on 27.02.2022.
//

#include "rule_cfg.h"

rule_cfg::rule_cfg(const std::string &name, const std::vector<std::pair<RULE_TYPE, std::string>> &data)
  : name(name)
  , data(data) {}