#include <iostream>
#include <unordered_map>
#include <algorithm>
#include "generators/lexer_generator.h"
#include "grammar/grammar_analyzer.h"
#include "parser/fun.h"
#include "parser/fun_parser.h"
#include "parser/regex_parser.h"
#include "parser/rule_parser.h"
#include "utils/utils.h"

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Expected arguments: <file with grammar> <folder to place parser>" << std::endl;
    return 1;
  }

  const std::string grammar_file(argv[1]);
  const std::string folder_path(argv[2]);

  try {
    std::ifstream in(grammar_file);
    if (!in.is_open()) {
      throw std::runtime_error("File " + grammar_file + " can not be opened.");
    }
    in.exceptions(std::ios_base::failbit);

    std::string line;
    lexer_generator lexer_generator_;
    std::unordered_map<std::string, rule> rules;
    std::unordered_map<std::string, fun> funs;
    std::vector<rule_cfg> rules_cfg;
    std::string start;
    while (getline(in, line)) {
      if (std::all_of(line.begin(), line.end(), std::iswspace)) {
        continue;
      }

      if (line[0] == '$') { // Function: $name[exported params list separated by ','] { multiline code }
        parse_fun(line, in, funs);
      } else if (line.find(':') == std::string::npos) { // Starting non-terminal
        start = line;
        break;
      } else if (std::isalpha(line[0]) && std::islower(line[0])) {
        rule cur_rule = rule_parser(line, lexer_generator_).parse();
        auto rules_it = rules.find(cur_rule.get_name());
        if (rules_it == rules.end()) {
          rules.emplace(cur_rule.get_name(), cur_rule);
        } else {
          rules_it->second.merge(cur_rule);
        }
        rules_cfg.push_back(cur_rule.generate_cfg());
      } else if (std::isalpha(line[0]) && std::isupper(line[0])) {
        auto[name, regex] = parse_regex(line);
        lexer_generator_.add_regex(name, escape(regex));
      } else {
        throw std::runtime_error("Unexpected first char in:\n" + line);
      }
    }
    in.close();

    // Propagate variables info to rules from functions
    for (auto&[rule_name, cur_rule]: rules) {
      for (const auto &fun: cur_rule.get_fun_names()) {
        auto attr_it = funs.find(fun);
        for (const auto&[var_type, var_name]: attr_it->second.get_exported_vars()) {
          cur_rule.add_exported_var(var_type, var_name);
        }
        cur_rule.add_fun_code(fun, attr_it->second.get_code());
      }
    }

    for (auto&[rule_name, cur_rule]: rules) {
      for (const auto&[prefix, assign_rule_name]: cur_rule.get_assigns()) {
        auto it = rules.find(assign_rule_name);
        if (it == rules.end()) { // Text assign: (var='...')
          std::cerr << "Found assignment to not existing rule (" << prefix << "=" << assign_rule_name << ")"
                    << std::endl;
          return 1;
        }
        for (const auto &exported_var: it->second.get_exported_vars()) {
          cur_rule.add_var(exported_var.first, prefix + "_" + exported_var.second);
        }
      }
    }

    std::unordered_map<std::string, std::string> constructors;
    for (const auto&[rule_name, cur_rule]: rules) {
      constructors.emplace(rule_name, cur_rule.generate_constructor());
    }

    grammar_analyzer analyzer(rules_cfg, start);

    std::ofstream result_cpp(folder_path + "/gen.cpp");
    std::ofstream result_h(folder_path + "/gen.h");
    if (!result_cpp.is_open()) {
      throw std::runtime_error("File " + folder_path + "/gen.cpp can not be created.");
    }
    if (!result_h.is_open()) {
      throw std::runtime_error("File " + folder_path + "/gen.h can not be created.");
    }
    result_cpp.exceptions(std::ios_base::failbit);
    result_h.exceptions(std::ios_base::failbit);

    result_h
      << "#include <bits/stdc++.h>" << std::endl
      << std::endl
      << "struct base_node {" << std::endl
      << "\tvirtual void visit() = 0;" << std::endl
      << "};" << std::endl
      << std::endl
      << "struct text_node : public base_node {" << std::endl
      << "\ttext_node(std::string text_) : text(std::move(text_)) {}" << std::endl
      << std::endl
      << "\tvoid visit() override {}" << std::endl
      << std::endl
      << "\tstd::string text;" << std::endl
      << "};" << std::endl
      << std::endl
      << "struct inner_node : public base_node {" << std::endl
      << "\tvoid add_child(const std::shared_ptr<base_node> &node) {" << std::endl
      << "\t\tchildren.push_back(node);" << std::endl
      << "\t}" << std::endl
      << std::endl
      << "\tvirtual void parse() = 0;" << std::endl
      << std::endl
      << "\tstd::vector<std::shared_ptr<base_node>> children;" << std::endl
      << "};" << std::endl;

    result_h << lexer_generator_.generate() << std::endl << std::endl;

    result_cpp << "#include \"gen.h\"" << std::endl
               << "lexer lexer_(\"\");" << std::endl;

    std::unordered_map<std::string, std::set<std::string>> exported_vars;
    for (const auto&[rule_name, cur_rule]: rules) {
      std::set<std::string> exported_vars_names;
      for (const auto &var: cur_rule.get_exported_vars()) {
        exported_vars_names.insert(var.second);
      }
      exported_vars.emplace(rule_name, exported_vars_names);
    }

    for (const auto&[rule_name, cur_rule]: rules) {
      auto[header_code, cpp_code] = cur_rule.generate_class(constructors, exported_vars, analyzer);
      result_h << header_code << std::endl << std::endl;
      result_cpp << cpp_code << std::endl;
    }

    result_cpp
      << "std::shared_ptr<" << start << "_node> parse(std::string text) {" << std::endl
      << "\tlexer_ = lexer(text);" << std::endl
      << "\tauto node = std::make_shared<" << start << "_node>();" << std::endl
      << "\tnode->parse();" << std::endl
      << "\tif (lexer_.next().first != TOKEN_TYPE::END) {" << std::endl
      << "\t\tthrow std::runtime_error(\"EOF expected\");" << std::endl
      << "\t}" << std::endl
      << "\treturn node;" << std::endl
      << "}" << std::endl;

    result_cpp
      << std::endl
      << "int main() {" << std::endl
      << "\tstd::string s;" << std::endl
      << "\tstd::getline(std::cin, s);" << std::endl
      << "\tauto node = parse(s);" << std::endl;
    for (const auto &exported_var: exported_vars.find(start)->second) {
      result_cpp
        << "\tstd::cout << \"" << exported_var << " = \" << node->" << exported_var << " << std::endl;" << std::endl;
    }
    result_cpp
      << "\treturn 0;" << std::endl
      << "}" << std::endl;

    result_cpp.close();
    result_h.close();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
