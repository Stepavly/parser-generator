//
// Created by stepavly on 13.12.2021.
//

#include "fun.h"
#include <utility>

fun::fun(std::string code, std::vector<var_t> exported_vars)
  : code(std::move(code))
  , exported_vars(std::move(exported_vars)) {}

const std::string &fun::get_code() const {
  return code;
}

const std::vector<var_t> &fun::get_exported_vars() const {
  return exported_vars;
}


