#include "NFA.h"

#include <iostream>
#include <algorithm>
#include <vector>
#include <utility>
#include <fstream>
#include <random>


void NFA::addTransition(std::string state, std::string symbol, const std::set<std::string>& nextStates) {
    if (nextStates.count(startState) > 0) {
        throw std::invalid_argument("Transitions to the start state are not allowed.");
    }

    transitionTable[{state, symbol}] = nextStates;
}

void NFA::newState(std::string newState) {
    states.insert(newState);
}

void NFA::addSymbol(std::string c) {
    alphabet.insert(c);
}

json NFA::to_json() const {
    json j;

    j["states"] = states;
    j["alphabet"] = alphabet;

    j["transition_table"] = json::array();
    for (const auto& [key, value] : transitionTable) {
        json transition;
        transition["from_state"] = key.first;
        transition["symbol"] = key.second;
        transition["to_states"] = value;

        j["transition_table"].push_back(transition);
    }

    j["start_state"] = startState;
    j["accept_states"] = acceptStates;

    return j;
}

void NFA::from_json(const json& j) {
    states = j.at("states").get<std::set<std::string>>();
    alphabet = j.at("alphabet").get<std::set<std::string>>();

    transitionTable.clear();
    if (j.contains("transition_table")) {
        for (const auto& transition : j.at("transition_table")) {
            std::string fromState = transition.at("from_state").get<std::string>();
            std::string symbol = transition.at("symbol").get<std::string>();
            std::set<std::string> toStates = transition.at("to_states").get<std::set<std::string>>();

            transitionTable[{fromState, symbol}] = toStates;
        }
    }

    startState = j.at("start_state").get<std::string>();
    acceptStates = j.at("accept_states").get<std::set<std::string>>();
}

void NFA::write_to_file(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file) {
        throw std::ios_base::failure("Failed to open file for writing");
    }
    file << to_json().dump(4);
}

bool NFA::read_from_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        return false;
    }
    try {
        json j;
        file >> j;
        from_json(j);
    } catch (const std::exception&) {
        return false;
    }
    return true;
}

void NFA::exportToDOT(const std::string& filename) const {
    std::ofstream out(filename);
    out << "digraph NFA {\n";
    out << "    rankdir=LR;\n\n";
    out << "    graph [fontname=\"Arial\"];\n";
    out << "    node [fontname=\"Arial\", fontsize=14];\n";
    out << "    edge [fontname=\"Arial\", fontsize=14];\n\n";

    for (const auto& state : states) {
        if (acceptStates.count(state))
            out << "    " << state << " [shape=doublecircle];\n";
        else
            out << "    " << state << " [shape=circle];\n";
    }
    out << "\n";

    out << "    _start [shape=point];\n";
    out << "    _start -> " << startState << ";\n\n";

    std::map<std::pair<std::string, std::string>, std::set<std::string>> grouped;
    for (const auto& [key, targets] : transitionTable) {
        const auto& from = key.first;
        const auto& symbol = key.second;
        for (const auto& to : targets) {
            grouped[{from, to}].insert(symbol);
        }
    }

    for (const auto& [key, symbols] : grouped) {
        const auto& from = key.first;
        const auto& to = key.second;
        std::string label;
        bool first = true;
        for (const auto& s : symbols) {
            if (!first) label += ",";
            label += s;
            first = false;
        }
        out << "    " << from << " -> " << to << " [label=\"" << label << "\"];\n";
    }

    out << "}\n";
    out.close();
}

std::set<std::string> NFA::getUnreachable() const {
    std::set<std::string> reachableStates;
    std::set<std::string> boundary;

    reachableStates.insert(startState);
    boundary.insert(startState);

    while (!boundary.empty()) {
        std::string s = *boundary.rbegin();
        boundary.erase(s);
        for (const std::string& symbol : alphabet) {
            auto it = transitionTable.find({ s, symbol });
            if (it != transitionTable.end()) {
                for (std::string state : it->second) {
                    if (reachableStates.find(state) == reachableStates.end()) {
                        reachableStates.insert(state);
                        boundary.insert(state);
                    }
                }
            }
        }
    }

    std::set<std::string> unreachable;
    for (const std::string& state : states) {
        if (reachableStates.find(state) == reachableStates.end()) {
            unreachable.insert(state);
        }
    }

    return unreachable;
}

void NFA::removeUnreachable() {
    std::set<std::string> unreachable = getUnreachable();

    for (const std::string& state : unreachable) {
        states.erase(state);
        acceptStates.erase(state);
    }

    for (auto it = transitionTable.begin(); it != transitionTable.end(); ) {
        const std::string& from = it->first.first;
        const std::string& to = it->first.second;

        if (unreachable.count(from)) {
            it = transitionTable.erase(it);
            continue;
        }

        std::set<std::string> updatedTargets;
        for (const std::string& dest : it->second) {
            if (!unreachable.count(dest)) {
                updatedTargets.insert(dest);
            }
        }

        if (updatedTargets.empty()) {
            it = transitionTable.erase(it);
        } else {
            it->second = updatedTargets;
            ++it;
        }
    }
}

NFA NFA::concatenation(const NFA& nfa1, const NFA& nfa2) {
    std::set<std::string> newStates;
    std::set<std::string> newAlphabet = nfa1.alphabet;
    newAlphabet.insert(nfa2.alphabet.begin(), nfa2.alphabet.end());

    std::map<std::pair<std::string, std::string>, std::set<std::string>> newTransitions;

    std::set<std::string> newAcceptStates;
    for (std::string state : nfa2.acceptStates) {
        newAcceptStates.insert(state);
    }

    for (std::string state : nfa1.states) newStates.insert(state);
    for (std::string state : nfa2.states) newStates.insert(state);

    for (const auto& entry : nfa1.transitionTable) {
        const auto& key = entry.first;
        const auto& value = entry.second;
        newTransitions[key] = value;
    }

    for (const auto& entry : nfa2.transitionTable) {
        const auto& key = entry.first;
        const auto& value = entry.second;
        std::set<std::string> newValue;
        for (std::string state : value) {
            newValue.insert(state);
        }

        newTransitions[std::make_pair(key.first, key.second)] = newValue;
        
    }

    for (const std::string& acceptState : nfa1.acceptStates) {
        for (const std::string& symbol : nfa2.alphabet) {
            auto it = nfa2.transitionTable.find({ nfa2.startState, symbol });
            if (it != nfa2.transitionTable.end()) {
                auto oldStates = it->second;
                std::set<std::string> newStates;
                for (std::string s : oldStates) {
                    newStates.insert(s);
                }

                newTransitions[std::make_pair(acceptState, symbol)].insert(newStates.begin(), newStates.end());
            }
        }
    }


    if (nfa2.acceptStates.find(nfa2.startState) != nfa2.acceptStates.end()) {
        newAcceptStates.insert(nfa1.acceptStates.begin(), nfa1.acceptStates.end());
    }

    NFA newNfa = NFA(newStates, newAlphabet, nfa1.startState, newAcceptStates);
    newNfa.transitionTable = newTransitions;

    return newNfa;
}

NFA NFA::alternative(const NFA& nfa1, const NFA& nfa2) {
    std::set<std::string> newStates = nfa1.states;
    newStates.insert(nfa2.states.begin(), nfa2.states.end());
    
    std::set<std::string> newAlphabet = nfa1.alphabet;
    newAlphabet.insert(nfa2.alphabet.begin(), nfa2.alphabet.end());

    std::string newStartState = NFA::getNewState(newStates, NFA::findAvailableSymbol(newStates));
    newStates.insert(newStartState);

    std::map<std::pair<std::string, std::string>, std::set<std::string>> newTransitions = nfa1.transitionTable;
    for (const auto& [key, value] : nfa2.transitionTable) {
        newTransitions[key].insert(value.begin(), value.end());
    }

    for (const std::string& symbol : newAlphabet) {
        auto it1 = nfa1.transitionTable.find({nfa1.startState, symbol});
        auto it2 = nfa2.transitionTable.find({nfa2.startState, symbol});
        std::set<std::string> combined;

        if (it1 != nfa1.transitionTable.end())
            combined.insert(it1->second.begin(), it1->second.end());

        if (it2 != nfa2.transitionTable.end())
            combined.insert(it2->second.begin(), it2->second.end());

        if (!combined.empty())
            newTransitions[{newStartState, symbol}] = combined;
    }

    std::set<std::string> newAcceptStates = nfa1.acceptStates;
    newAcceptStates.insert(nfa2.acceptStates.begin(), nfa2.acceptStates.end());

    bool q1Accept = nfa1.acceptStates.count(nfa1.startState) > 0;
    bool q2Accept = nfa2.acceptStates.count(nfa2.startState) > 0;
    if (q1Accept || q2Accept)
        newAcceptStates.insert(newStartState);

    NFA result(newStates, newAlphabet, newStartState, newAcceptStates);
    result.transitionTable = newTransitions;
    return result;
}


bool NFA::run(const std::vector<std::string>& input) const {
    return runHelper(startState, input, 0);
}

bool NFA::runHelper(std::string currentState, const std::vector<std::string>& input, int index) const {
    return false;
}

std::string NFA::findAvailableSymbol(std::set<std::string> usedSymbols) {
    std::vector<std::string> possibleSybmols = {"S", "Q", "P", "A", "B", "C"};

    for (const std::string& candidate : possibleSybmols) {
        bool found = false;
        for (const std::string& used : usedSymbols) {
            if (used[0] == candidate[0]) {
                found = true;
                break;
            }
        }

        if (!found) {
            return candidate;
        }
    }

    return "X";
}

std::string NFA::getNewState(std::set<std::string> states, std::string symbol) {
    int max = -1;

    for (const std::string& state : states) {
        if (state.substr(0, 1) == symbol) {
            int number = std::stoi(state.substr(1));
            if (number > max) {
                max = number;
            }
        }
    }

    return symbol + std::to_string(max + 1);
}
