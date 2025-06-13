#pragma once

#include <map>
#include <set>
#include <string>
#include <algorithm>
#include "json.hpp"

using json = nlohmann::json;

class NFA {
private:
public:
    std::set<std::string> states;
    std::set<std::string> alphabet;
    std::map<std::pair<std::string, std::string>, std::set<std::string>> transitionTable;
    std::string startState;
    std::set<std::string> acceptStates;

public:
    NFA() = default;
    NFA(const std::set<std::string> states, const std::set<std::string> alphabet, std::string startState, const std::set<std::string> acceptStates)
        : states(states), alphabet(alphabet), startState(startState), acceptStates(acceptStates) { }

    void addTransition(std::string state, std::string symbol, const std::set<std::string>& nextStates);
    void newState(std::string newState);
    void addSymbol(std::string c);

    static NFA concatenation(const NFA& nfa1, const NFA& nfa2);
    static NFA alternative(const NFA& nfa1, const NFA& nfa2);

    static NFA iteration(const NFA& nfa);
    static NFA iterationPlus(const NFA& nfa);

    std::set<std::string> getUnreachable() const;
    void removeUnreachable();

    void exportToDOT(const std::string& filename) const;

    bool run(const std::vector<std::string>& input) const;

    int getStatesCount() const {
        return states.size();
    }

    json to_json() const;
    void from_json(const json& j);

    void write_to_file(const std::string& filename) const;
    bool read_from_file(const std::string& filename);

    static NFA generateRandom(int statesCount);

private:
    bool runHelper(std::string currentState, const std::vector<std::string>& input, int index) const;
    
    static std::string findAvailableSymbol(std::set<std::string> usedSymbols);
    static std::string getNewState(std::set<std::string> states, std::string symbol);
};

