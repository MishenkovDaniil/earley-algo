#include <cstring>
#include <set>

#include "earley.hpp"

void Earley::fit (Grammar& grammar) {
    grammar_ = grammar;
    grammar_.ConvertToEarley();
}

bool Earley::predict (const std::string& string) {
    return predict(string.c_str());
}

bool Earley::predict (const char *string) {
    size_t len = strlen(string);

    std::vector<std::set<Situation>> situation_lists(len + 1);
    situation_lists[0].insert(Situation(grammar_.rules__[grammar_.start_][0], 0, 0));

    for (size_t j = 0; j < len + 1; ++j) {
        scan (situation_lists, j, string, len);
        size_t last = -1;
        while (situation_lists[j].size() != last) {
            last = situation_lists[j].size();
            complete (situation_lists, j, string, len);
            predict (situation_lists, j, string, len);
        }
    }

    return situation_lists[len].count(Situation(grammar_.rules__[grammar_.start_][0], 1, len));
}


void Earley::scan(std::vector<std::set<Situation>> situation_lists, size_t cur_sit_idx, const char *string, size_t pos) {
    if (cur_sit_idx == 0)
        return;
    for (auto situation: situation_lists[cur_sit_idx - 1]) {
        GrammarSymbol symb = situation.rule_.right_[situation.pos_in_rule_];
        if (symb.isTerminal_ && symb.symbol_ == string[cur_sit_idx - 1]) {
            situation_lists[cur_sit_idx].insert(Situation(situation.rule_, situation.pos_in_rule_ + 1, situation.pos_in_word_));
        }
    }
} 

void Earley::predict(std::vector<std::set<Situation>> situation_lists, size_t cur_sit_idx, const char *string, size_t pos) {
    for (auto situation: situation_lists[cur_sit_idx]) {
        GrammarSymbol symb = situation.rule_.right_[situation.pos_in_rule_];
        if (symb.isTerminal_)
            continue;
        NeTerminal neTerminal = *(reinterpret_cast<NeTerminal *>(&situation.rule_.right_[situation.pos_in_rule_]));
        for (auto rule:grammar_.rules__[neTerminal]) {
            situation_lists[cur_sit_idx].insert(Situation(rule, 0, cur_sit_idx));
        }
    }
} 

void Earley::complete(std::vector<std::set<Situation>> situation_lists, size_t cur_sit_idx, const char *string, size_t pos) {
    for (auto sit: situation_lists[cur_sit_idx - 1]) {
        size_t i = sit.pos_in_word_;
        for (auto situation: situation_lists[i]) {
            if (situation.pos_in_word_ == cur_sit_idx) {
                situation_lists[cur_sit_idx].insert(Situation(situation.rule_, situation.pos_in_rule_ + 1, situation.pos_in_word_));
            }
        }
    }
} 
