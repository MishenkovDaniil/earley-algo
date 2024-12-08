#pragma once
#include "grammar/grammar.hpp"


class Situation {
public:
    Rule rule_;
    size_t pos_in_rule_ = 0;
    size_t pos_in_word_ = 0;

    Situation (Rule rule, size_t pos_in_rule, size_t pos_in_word) : 
        rule_(rule), pos_in_rule_(pos_in_rule), pos_in_word_(pos_in_word) {}

    bool operator < (const Situation& other)const {
        return pos_in_rule_ < other.pos_in_rule_ && pos_in_word_ < other.pos_in_word_;
    }
};

class Earley {
    Grammar grammar_;
public:
    void fit (Grammar& grammar);
    bool predict (const std::string& string);
    bool predict (const char *string);
private:
    void scan(std::vector<std::set<Situation>> situation_lists, size_t cur_sit_idx, const char *string, size_t pos); 
    void predict(std::vector<std::set<Situation>> situation_lists, size_t cur_sit_idx, const char *string, size_t pos); 
    void complete(std::vector<std::set<Situation>> situation_lists, size_t cur_sit_idx, const char *string, size_t pos); 
};
