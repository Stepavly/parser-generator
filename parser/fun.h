//
// Created by stepavly on 13.12.2021.
//

#ifndef PARSER_GENERATOR_PARSER_FUN_H_
#define PARSER_GENERATOR_PARSER_FUN_H_

#include <string>
#include <vector>

using var_t = std::pair<std::string, std::string>;

class fun {
 public:

  fun(std::string code, std::vector<var_t> exported_vars);

  const std::string& get_code() const;
  const std::vector<var_t>& get_exported_vars() const;

 private:
  std::string code;
  std::vector<var_t> exported_vars;
};

#endif //PARSER_GENERATOR_PARSER_FUN_H_
