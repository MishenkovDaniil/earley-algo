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
        if (pos_in_rule_ < other.pos_in_rule_)
            return true;
        if (pos_in_rule_ == other.pos_in_rule_) {
            if (pos_in_word_ < other.pos_in_word_) {
                return true;
            }
            if (pos_in_word_ == other.pos_in_word_) {
                if (rule_.left_ < other.rule_.left_) {
                    return true;
                }
                if (rule_.left_ == other.rule_.left_) {
                    if (rule_.right_.size() < other.rule_.right_.size()) {
                        return true;
                    }
                    if (rule_.right_.size() == other.rule_.right_.size()) {
                        size_t i = 0;
                        while (i < rule_.right_.size() && rule_.right_[i] == other.rule_.right_[i]) {
                            ++i;
                        }
                        if (i < rule_.right_.size() && rule_.right_[i] < other.rule_.right_[i]) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }
    void Print () {
        std::cout << "Situation:" << rule_.left_.GetAsString() << "->";
        for (size_t i = 0; i < pos_in_rule_; ++i) {
            std::cout << rule_.right_[i].GetAsString();
        }
        std::cout << "*";
        for (size_t i = pos_in_rule_, max = rule_.right_.size(); i < max; ++i) {
            std::cout << rule_.right_[i].GetAsString();
        }
        std::cout << ", " << pos_in_word_ << "\n";
    }
};

class Earley {
    Grammar grammar_;
public:
    void fit (Grammar& grammar);
    bool predict (const std::string& string);
    bool predict (const char *string);
private:
    void scan(std::vector<std::set<Situation>>& situation_lists, size_t cur_sit_idx, const char *string, size_t pos); 
    void predict(std::vector<std::set<Situation>>& situation_lists, size_t cur_sit_idx, const char *string, size_t pos); 
    void complete(std::vector<std::set<Situation>>& situation_lists, size_t cur_sit_idx, const char *string, size_t pos); 
};
