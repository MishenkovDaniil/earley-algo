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
    // grammar_.Print();
    size_t len = strlen(string);
    // printf ("1\n");
    std::vector<std::set<Situation>> situation_lists(len + 1);
    // printf ("2\n");
    situation_lists[0].insert(Situation(grammar_.rules__[grammar_.start_][0], 0, 0));
    // printf ("3\n");

    for (size_t j = 0; j < len + 1; ++j) {
        // printf ("4\n");
        scan (situation_lists, j, string, len);
        size_t last = -1;
        // printf ("5\n");
        while (situation_lists[j].size() != last) {
            // printf ("6\n");
            last = situation_lists[j].size();
            // printf ("7\n");
            complete (situation_lists, j, string, len);
            // printf ("8\n");
            predict (situation_lists, j, string, len);
            // printf ("9\n");
        }
        // printf ("10\n");
        // for (size_t i = 0; i <= j; ++i) {
        //     std::cout << "situations " << i << "\n";
        //     for (auto sit:situation_lists[i]) {
        //         std::cout << "\t";
        //         sit.Print();
        //     }
        //     std::cout << "\n";
        // }
    }
    // printf ("11\n");

    return situation_lists[len].count(Situation(grammar_.rules__[grammar_.start_][0], 1, 0));
}


void Earley::scan(std::vector<std::set<Situation>>& situation_lists, size_t cur_sit_idx, const char *string, size_t pos) {
    if (cur_sit_idx == 0)
        return;
    for (auto situation: situation_lists[cur_sit_idx - 1]) {
        GrammarSymbol symb = situation.rule_.right_[situation.pos_in_rule_];
        if (symb.isTerminal_ && symb.symbol_ == string[cur_sit_idx - 1]) {
            situation_lists[cur_sit_idx].insert(Situation(situation.rule_, situation.pos_in_rule_ + 1, situation.pos_in_word_));
        }
    }
} 

void Earley::predict(std::vector<std::set<Situation>>& situation_lists, size_t cur_sit_idx, const char *string, size_t pos) {
    for (auto situation: situation_lists[cur_sit_idx]) {
        GrammarSymbol symb = situation.rule_.right_[situation.pos_in_rule_];
        if (symb.isTerminal_) {
            // printf ("terminal\n");
            continue;
        }
        NeTerminal neTerminal = *(reinterpret_cast<NeTerminal *>(&(situation.rule_.right_[situation.pos_in_rule_])));
        for (auto rule: grammar_.rules__[neTerminal]) {
            // printf ("insert");
            // std::cout << neTerminal.GetAsString() << "\n";
            // rule.Print();
            // printf ("%d\n", situation_lists[cur_sit_idx].count(Situation(rule, 0, cur_sit_idx)));
            situation_lists[cur_sit_idx].insert(Situation(rule, 0, cur_sit_idx));
        }
    }
} 

void Earley::complete(std::vector<std::set<Situation>>& situation_lists, size_t cur_sit_idx, const char *string, size_t pos) {
    // printf("$1\n");
    for (auto sit: situation_lists[cur_sit_idx]) {
        // printf("$2\n");
        if((sit.rule_.right_.size() != sit.pos_in_rule_) &&
            !(sit.rule_.right_[0] == Epsilon))
            continue;
        size_t i = sit.pos_in_word_;
        for (auto situation: situation_lists[i]) {
            // printf("$3, %d\n", i);
            if (!(situation.rule_.right_[situation.pos_in_rule_].isTerminal_)) {
                // printf("$5\n");
                situation_lists[cur_sit_idx].insert(Situation(situation.rule_, situation.pos_in_rule_ + 1, situation.pos_in_word_));
            }
        }
    }
} 
