cmake_minimum_required(VERSION 3.20)
project(parser_generator)

set(CMAKE_CXX_STANDARD 17)

add_executable(parser_generator main.cpp grammar/grammar_analyzer.cpp grammar/grammar_analyzer.h parser/rule_parser.cpp parser/rule_parser.h parser/lexer.cpp parser/lexer.h parser/rule.cpp parser/rule.h generators/lexer_generator.cpp generators/lexer_generator.h parser/fun.cpp parser/fun.h utils/utils.h parser/fun_parser.h parser/regex_parser.h parser/rule_cfg.h parser/rule_cfg.cpp parser/rule_cfg.h parser/rule_utils.h utils/utils.cpp parser/regex_parser.cpp)

add_executable(test test/gen.cpp test/gen.h)
