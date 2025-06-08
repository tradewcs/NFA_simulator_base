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

bool NFA::run(const std::vector<std::string>& input) const {
    return runHelper(startState, input, 0);
}

std::string getCharString(const std::set<char> states) {
    std::string res = "";

    auto it = states.begin();
    if (it != states.end()) {
        res += *it;
        it++;
    }

    while (it != states.end()) {
        res += ", " + std::string(1, *it);
        it++;
    }

    return res;
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
    out << "    rankdir=LR;\n";
    out << "    node [shape=doublecircle];";
    for (const auto& state : acceptStates)
        out << " " << state;
    out << ";\n";
    out << "    node [shape=circle];\n";
    out << "    _start [shape=point];\n";
    out << "    _start -> " << startState << ";\n";
    for (const auto& trans : transitionTable) {
        const auto& from = trans.first.first;
        const auto& symbol = trans.first.second;
        for (const auto& to : trans.second) {
            out << "    " << from << " -> " << to << " [label=\"" << symbol << "\"];\n";
        }
    }
    out << "}\n";
    out.close();
}