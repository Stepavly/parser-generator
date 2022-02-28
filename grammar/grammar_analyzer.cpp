//
// Created by stepavly on 12.12.2021.
//

#include "grammar_analyzer.h"
#include <sstream>
#include <algorithm>

grammar_analyzer::grammar_analyzer(const std::vector<rule_cfg>& rules_cfg, const std::string& start) {
  for (const rule_cfg& cur_rule: rules_cfg) {
    const auto& rule_name = cur_rule.name;
    auto cur_rules = rules.insert({rule_name, {}}).first;
    cur_rules->second.emplace_back();
    for (const auto&[type, data]: cur_rule.data) {
      cur_rules->second.back().push_back(data);
      if (type == RULE_TYPE::TEXT || type == RULE_TYPE::TRANSITION_REGEX) {
        terminals.insert(data);
      }
    }
  }

  for (auto&[non_term, cur_rules]: rules) {
    if (non_term == start) {
      std::for_each(cur_rules.begin(), cur_rules.end(), [](auto &rule) { rule.emplace_back("END"); });
    }
  }

  this->start = start;
  terminals.emplace("END");
  init_first();
  init_follow();
  check_ll1();
}

void grammar_analyzer::init_first() {
  for (const auto&[non_terminal, _]: rules) {
    first.insert({{non_terminal}, {}});
  }
  for (const auto &terminal: terminals) {
    first.insert({{terminal}, {terminal}});
  }
  first[{"END"}].insert("EPS");

  while (true) {
    bool updated = false;
    for (const auto&[non_terminal, cur_rules]: rules) {
      auto &cur_first = first[{non_terminal}];
      for (const auto &rule: cur_rules) {
        auto old_size = cur_first.size();
        const auto &merge_first = get_first(rule);
        cur_first.insert(merge_first.begin(), merge_first.end());
        updated |= cur_first.size() != old_size;
      }
    }
    if (!updated) {
      break;
    }
  }
}

std::set<std::string> grammar_analyzer::get_first(const std::vector<std::string> &rule) {
  if (rule.empty()) {
    return {"EPS"};
  }
  auto cached_first = first.find(rule);
  if (cached_first != first.end() && (!cached_first->second.empty() || rule.size() == 1)) {
    return cached_first->second;
  }

  if (terminals.find(rule[0]) == terminals.end()) {
    first[rule] = get_first({rule[0]});
  } else {
    first[rule] = {rule[0]};
  }

  std::set<std::string> &first_rule = first[rule];
  if (rule.size() > 1 && first_rule.count("EPS")) {
    std::vector<std::string> rule_suffix(rule.begin() + 1, rule.end());
    for (const auto &s: get_first(rule_suffix)) {
      first_rule.insert(s);
    }
  }
  return first_rule;
}

void grammar_analyzer::init_follow() {
  follow[start].emplace("END");
  while (true) {
    bool changed = false;
    for (const auto&[A, rules_a]: rules) {
      for (const auto &rule: rules_a) {
        for (size_t i = 0; i < rule.size(); i++) {
          const auto &B = rule[i];
          if (terminals.find(B) != terminals.end()) {
            continue;
          }
          std::vector<std::string> suffix(rule.begin() + static_cast<std::ptrdiff_t>(i) + 1, rule.end());
          auto old_size = follow[B].size();
          const auto &suffix_first = get_first(suffix);
          follow[B].insert(suffix_first.begin(), suffix_first.end());
          if (follow[B].erase("EPS") > 0) {
            follow[B].insert(follow[A].begin(), follow[A].end());
          }
          changed |= old_size != follow[B].size();
        }
      }
    }
    if (!changed) {
      break;
    }
  }
}

std::string rule_to_string(const std::string &non_terminal, const std::vector<std::string> &rule) {
  std::string s = non_terminal + " :";
  for (const auto &r: rule) {
    if (r != "END") {
      s += " " + r;
    }
  }
  return s;
}

template<typename T>
bool intersects(const std::set<T> &a, const std::set<T> &b) {
  if (a.size() > b.size()) {
    return intersects(b, a);
  }
  return std::any_of(a.begin(), a.end(), [&b](const T &value) { return b.find(value) != b.end(); });
}

void grammar_analyzer::check_ll1() {
  for (const auto&[non_terminal, cur_rules]: rules) {
    size_t rule1_i = 0;
    for (const auto &rule1: cur_rules) {
      size_t rule2_i = 0;
      for (const auto &rule2: cur_rules) {
        if (rule1_i != rule2_i) {
          std::vector<std::string> intersection;
          const auto &first1 = get_first(rule1);
          const auto &first2 = get_first(rule2);
          if (intersects(first1, first2) ||
            (first1.find("EPS") != first1.end() && intersects(follow[non_terminal], first2))) {
            throw std::runtime_error("Parser can not be generated because of rules:\n" +
              rule_to_string(non_terminal, rule1) + "\n" +
              rule_to_string(non_terminal, rule2));
          }
        }
        rule2_i++;
      }
      rule1_i++;
    }
  }
}

std::set<std::string> grammar_analyzer::get_follow(const std::string &non_term) {
  return follow[non_term];
}
