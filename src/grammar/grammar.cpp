#include "grammar.hpp"
#include <sstream>
#include <cctype>
#include <set>
#include <map>

/*
В первой строке содержатся 3 целых числа ∣N∣,∣Σ∣ и ∣P∣ --- количество нетерминальных символов, терминальных символов и правил в порождающей грамматике. Все числа неотрицательные и не превосходят 100.
Во второй строке содержатся ∣N∣ нетерминальных символов. Нетерминальные символы являются заглавными латинскими буквами.
В третьей строке содержатся ∣Σ∣ символов алфавита. Символы являются строчными латинскими буквами, цифрами, скобками или знаками арифметических операций.

В каждой из следующих P строк записано одно правило грамматики в формате левая часть правила -> правая части правила. 
ε в правой части правила обозначается отсутствием правой части (концом строки после ->).

Следующая строка состоит из одного нетерминального символа --- стартового символа грамматики.
*/
void Grammar::CreateFromStdin() {  
	std::cin >> neTerminalsSize_ >> alphabetSize_ >> rulesSize_;

	if (neTerminalsSize_ > 100) {
		throw GrammarException("Number of not terminals must me below 100 or 100.");
	} else if (alphabetSize_ > 100) {
		throw GrammarException("Number of terminals must me below 100 or 100.");
	} else if (rulesSize_ > 100) {
		throw GrammarException("Number of rules must me below 100 or 100.");
	} 

	for (size_t i = 0; i < neTerminalsSize_; ++i) {
		char neTerminal = 0;
		std::cin >> neTerminal;
		neTerminals_.push_back (NeTerminal(neTerminal));
	}

	for (size_t i = 0; i < alphabetSize_; ++i) {
		char terminal = 0;
		std::cin >> terminal;
		alphabet_.push_back (Terminal(terminal));
	}

	char skip;
	std::cin.get(skip);

	try {
		for (size_t i = 0; i < rulesSize_; ++i) {
			ReadRuleFromStdin();
		}
	} catch (GrammarException e) {
		throw e;
	}

	char start;	
	std::cin>>start;
	if(!IsNeTerminal(start)){
		throw GrammarException("Error: start must be non-terminal.");
	}
	start_ = NeTerminal(start);
}

void Grammar::ReadRuleFromStdin() {
	std::string rule;
	std::getline(std::cin, rule);

	std::istringstream ruleStream(rule);


	char left = 0;
	ruleStream >> left;

	if (!IsNeTerminal(left)) {
		throw GrammarException ("Wrong rule syntax: left part must consist from a non-terminal.");
	}	

	char next1 = 0;
	char next2 = 0;
	ruleStream >> next1;
	ruleStream >> next2;

	if (next1 != '-' || next2 != '>') {
		throw GrammarException ("Wrong rule syntax: rule doesn't contain '->' delimiter between left and right parts.");
	}

	// printf ("rule = %s\n", rule.c_str());
	char symbol = 0;
	std::vector<GrammarSymbol> right;
	while (ruleStream.peek() != EOF && ruleStream.peek() != '\n') {
		ruleStream >> symbol;
		if (isspace(symbol) || symbol == 0)	
			continue;
		// printf ("symbol = [%d]\n", symbol);
		if (IsNeTerminal(symbol)){
			right.push_back(NeTerminal(symbol));
		} else if (IsTerminal(symbol)) {
			right.push_back(Terminal(symbol));
		} else {
			throw GrammarException("Wrong rule syntax: right part contains unknown symbol.");
		}
	}

	if (!rules__.count(NeTerminal(left))){
		rules__[NeTerminal(left)] = std::vector<Rule>();
	}
	rules__[NeTerminal(left)].push_back(Rule(NeTerminal(left), right));
}

void Grammar::Print(){
	std::cout << "Alphabet:";
	for (auto elem: alphabet_) {
		std::cout << elem.GetAsString() << ",";
	}
	std::cout << std::endl;

	std::cout << "Non-terminals:";
	for (auto elem: neTerminals_) {
		if (elem.symbol_ < 256) {
			std::cout << elem.GetAsString() << ",";
		} else {
			std::cout << elem.symbol_ << ",";
		}
	}
	std::cout << std::endl;

	std::cout << "Rules:\n";
	for (auto& neTerminal: neTerminals_) {
		for (auto& rule: rules__[neTerminal]) {
			rule.Print();
		}
	}
	std::cout << std::endl;
}

void Grammar::ConvertToChomsky() {
	RemoveLongRules();
	RemoveEmptyRules();
	RemoveChainRules();
    RemoveUselessSymbols();    
	RemoveRemain();
}

void Grammar::RemoveEmptyRules() {
	for (auto elem: neTerminals_) {
		for (auto it = rules__[elem].begin(), end = rules__[elem].end(); it != end; ++it) {
			if (it->right_.size() == 1 && it->right_[0] == Epsilon) {	
				Rule rule = *it;
				if (!(it->left_ == start_)) {
					rules__[elem].erase(it);
				}
				RemoveEmptyRule(rule);
			} 
		}
	}
}

void Grammar::RemoveEmptyRule(Rule& empty_rule) {
	for (auto left: neTerminals_) {
		for (auto rule: rules__[left]) {
			AddRuleWithEmptyN(rule, empty_rule.left_);
		}
	}
}

void Grammar::AddRuleWithEmptyN (Rule& rule, NeTerminal neTerminal) {
	size_t count = 0;
	std::vector<GrammarSymbol> new_right;
	for (size_t i = 0, size = rule.right_.size(); i < size; ++i) {
		if (rule.right_[i] == neTerminal) {
			++count;
			continue;
		}
		new_right.push_back(rule.right_[i]);
	}
	if (count) {
		rules__[rule.left_].push_back(Rule(rule.left_, new_right));
	}
}

void Grammar::RemoveLongRules() {
	for (auto elem: neTerminals_) {
		std::vector<Rule> new_rules;
		for (auto it = rules__[elem].begin(), end = rules__[elem].end(); it != end; ++it) {
			if (it->right_.size() > 2) {
				RemoveLongRule(new_rules, *it);
			} else {
				new_rules.push_back(*it);
			}
		}
		rules__[elem] = new_rules;
	}
}

void Grammar::RemoveLongRule(std::vector<Rule>& new_rules, Rule& rule) {
	size_t n = rule.right_.size();
	NeTerminal prevNeTerminal = rule.left_;
	for (size_t i = 0; i < n - 2; ++i) {
		NeTerminal newNeTerminal = NeTerminal(lastFreeSpecialSymbol_); 

		neTerminals_.push_back(newNeTerminal);
		if (!i) {
			new_rules.push_back (Rule(prevNeTerminal, {rule.right_[i], newNeTerminal}));	
		} else {
			rules__[prevNeTerminal] = std::vector<Rule>();
			rules__[prevNeTerminal].push_back(Rule(prevNeTerminal, {rule.right_[i], newNeTerminal}));
		}

		prevNeTerminal = newNeTerminal;

		UpdateLatestFreeNeTerminal();
	}
	rules__[prevNeTerminal].push_back(Rule(prevNeTerminal, {rule.right_[n - 2], rule.right_[n - 1]}));
	neTerminalsSize_ += n - 2;
	rulesSize_ += n - 2;
}

void Grammar::RemoveChainRules() {
	for (auto elem: neTerminals_) {
		size_t count = 0;
		std::vector<Rule> new_rules;

		for (auto it = rules__[elem].begin(), end = rules__[elem].end(); it != end; ++it) {
			if (it->right_.size() == 1 && !((it->right_)[0].IsTerminal())) {	
				RemoveChainRule(*it, new_rules);
				++count;
			} else {
				new_rules.push_back(*it);
			}
		}

		if (count) {
			rulesSize_ += new_rules.size() - rules__[elem].size();
			rules__[elem] = new_rules;
		}
	}
}

void Grammar::RemoveChainRule(Rule& rule, std::vector<Rule>& new_rules) {
	NeTerminal left = rule.left_;
	NeTerminal right = *(reinterpret_cast<NeTerminal*>(&rule.right_[0]));
	if (left == right) {
		return;
	}
	for (auto rule: rules__[right]) {
		if (!IsChainRule(rule)) {
			new_rules.emplace_back(left, rule.right_);
		}
	}
}

bool Grammar::IsChainRule(Rule& rule) {
	return rule.right_.size() == 1 && !(rule.right_[0].isTerminal_);
}


void Grammar::RemoveUselessSymbols(){
	RemoveNonGeneratingRules();
	RemoveNonAchievableRules();
}
void Grammar::RemoveNonGeneratingRules(){
	std::set<NeTerminal> generating;
	std::set<NeTerminal> non_generating;
	for (auto elem: neTerminals_){
		bool non_gen = true;
		for (auto rule: rules__[elem]) {
			if (rule.right_[0].isTerminal_) {
				if (rule.right_.size() == 1 || rule.right_[1].isTerminal_) {
					generating.insert(elem);
					non_gen = false;
					break;
				}
			}
		}
		if (non_gen)
			non_generating.insert(elem);
	}

	size_t last = 0;
	while (last != generating.size()) {
		last = generating.size();
		auto it = non_generating.begin();
		auto end = non_generating.end();
		while (it != end) {
			NeTerminal elem = *it;
			bool non_gen = true;
			for (auto rule: rules__[elem]) {
				if (IsGeneratingRule(rule, generating)) {
					generating.insert(elem);
					non_gen = false;
					break;
				} 
			}
			++it;
			if (!non_gen)
				non_generating.erase(elem);
		}
	}

	if (non_generating.size()) {
		for (auto elem: non_generating) {
			rulesSize_ -= rules__[elem].size();
			rules__[elem] = std::vector<Rule>();
		}
		for (auto elem: generating) {
			std::vector<Rule> new_rules;
			for (auto rule: rules__[elem]) {
				if (!IsNonGeneratingRule(rule, non_generating)) {
					new_rules.push_back(rule);
				} 
			}

			if (new_rules.size() != rules__[elem].size()) {
				rulesSize_ += new_rules.size();
				rulesSize_ -= rules__[elem].size();
				rules__[elem] = new_rules;
			}
		}
	}
}

void Grammar::RemoveNonAchievableRules(){
	std::set<NeTerminal> achievable = {start_}; 
	std::set<NeTerminal> achievable_temp = {start_}; 
	std::set<NeTerminal> not_achievable;
	for (auto elem:neTerminals_) {
		if (elem == start_)
			continue;
		not_achievable.insert(elem);
	}

	size_t last = 0;
	while (last != achievable.size()) {
		last = achievable.size();
		std::set<NeTerminal> new_achievable;

		for (auto elem: achievable_temp){
			for (auto rule: rules__[elem]) {
				for (size_t i = 0, len = rule.right_.size(); i < len; ++i) {
					if (!(rule.right_[i].isTerminal_)) {
						NeTerminal right = *(reinterpret_cast<NeTerminal *>(&rule.right_[i]));
						if (not_achievable.count(right)) {
							new_achievable.insert(right);
							achievable.insert(right);
							not_achievable.erase(right);
						}
					}
				}
			}
		}

		achievable_temp.clear();
		achievable_temp = new_achievable;
	}


	if (not_achievable.size()) {
		for (auto elem: not_achievable) {
			rulesSize_ -= rules__[elem].size();
			rules__[elem] = std::vector<Rule>();
		}
		for (auto elem: achievable) {
			std::vector<Rule> new_rules;
			for (auto rule: rules__[elem]) {
				if (!IsNonAchievableRule(rule, not_achievable)) {
					new_rules.push_back(rule);
				}
			}
			if (new_rules.size() != rules__[elem].size()) {
				rulesSize_ -= rules__[elem].size();
				rulesSize_ += new_rules.size();
				rules__[elem] = new_rules;
			}
		}
	}
}

bool Grammar::IsNonAchievableRule(Rule&rule, std::set<NeTerminal>& not_achievable) {
	size_t len = rule.right_.size();
	for (size_t i = 0; i < len; ++i) {
		if (!(rule.right_[i].isTerminal_)) {
			NeTerminal right = *(reinterpret_cast<NeTerminal *>(&rule.right_[i]));
			if (not_achievable.count(right)) {
				return true;
			}
		}
	}
	return false;
}

bool Grammar::IsNonGeneratingRule(Rule&rule, std::set<NeTerminal>& non_generating) {
	size_t len = rule.right_.size();
	for (size_t i = 0; i < len; ++i) {
		if (!(rule.right_[i].isTerminal_)) {
			NeTerminal right = *(reinterpret_cast<NeTerminal *>(&rule.right_[i]));
			if (non_generating.count(right)) {
				return true;
			}
		}
	}
	return false;
}

bool Grammar::IsGeneratingRule(Rule&rule, std::set<NeTerminal>& generating) {
	size_t len = rule.right_.size();
	for (size_t i = 0; i < len; ++i) {
		if (!(rule.right_[i].isTerminal_)) {
			NeTerminal right = *(reinterpret_cast<NeTerminal *>(&rule.right_[i]));
			if (!generating.count(right)) {
				return false;
			}
		}
	}
	return true;
}

void Grammar::RemoveRemain(){
	std::map<GrammarSymbol, NeTerminal> map;

	for (auto elem:neTerminals_) {
		std::vector<Rule> new_rules;
	
		for (auto rule: rules__[elem]) {
			if (rule.right_.size() == 2) {
				bool isFromTerminal = false;
				GrammarSymbol symb1 = rule.right_[0];
				GrammarSymbol symb2 = rule.right_[1];
	
				if (rule.right_[0].isTerminal_) {
					isFromTerminal = true;
					if (!map.count(rule.right_[0])) {
						NeTerminal newNeTerminal = NeTerminal(lastFreeSpecialSymbol_);
						UpdateLatestFreeNeTerminal();
						rules__[newNeTerminal] = std::vector<Rule>();
						rules__[newNeTerminal].push_back (Rule(newNeTerminal, {rule.right_[0]}));
						map[rule.right_[0]] = newNeTerminal;
						++rulesSize_;
					}
					symb1 = map[rule.right_[0]];
				}
	
				if (rule.right_[1].isTerminal_) {
					isFromTerminal = true;
					if (!map.count(rule.right_[1])) {
						NeTerminal newNeTerminal = NeTerminal(lastFreeSpecialSymbol_);
						UpdateLatestFreeNeTerminal();
						rules__[newNeTerminal] = std::vector<Rule>();
						rules__[newNeTerminal].push_back (Rule(newNeTerminal, {rule.right_[1]}));
						map[rule.right_[1]] = newNeTerminal;
						++rulesSize_;
					}
					symb2 = map[rule.right_[1]];
				}

				if (isFromTerminal) {
					new_rules.push_back(Rule(elem, {symb1, symb2}));
				} else {
					new_rules.push_back(rule);
				}			
			} else {
				new_rules.push_back(rule);
			}
		}

		rulesSize_ -= rules__[elem].size() - new_rules.size();
		rules__[elem] = new_rules;
	}	
}

void Grammar::ConvertToEarley() {
    NeTerminal new_start = NeTerminal(lastFreeSpecialSymbol_);
    UpdateLatestFreeNeTerminal();
    rules__[new_start] = std::vector<Rule>();
    rules__[new_start].push_back(Rule(new_start, {start_}));
    start_ = new_start;
}
