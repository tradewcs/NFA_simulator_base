#include <iostream>
#include "NFA.h"

int main1() {
    NFA nfa1;

    nfa1.states = {"q0", "q1", "q2"};
    nfa1.alphabet = {"0", "1"};
    
    nfa1.transitionTable[{"q0", "0"}] = {"q1"};
    nfa1.transitionTable[{"q1", "1"}] = {"q2"};
    nfa1.transitionTable[{"q2", "0"}] = {"q2"};
    
    nfa1.startState = "q0";
    nfa1.acceptStates = {"q2"};



    NFA nfa2;

    nfa2.states = {"s0", "s1", "s2", "s3"};
    nfa2.alphabet = {"0", "1"};

    nfa2.transitionTable[{"s0", "0"}] = {"s1", "s2"};
    nfa2.transitionTable[{"s1", "1"}] = {"s3"};
    nfa2.startState = "s0";
    nfa2.acceptStates = {"s2"};


    NFA res = NFA::concatenation(nfa1, nfa2);
        
    std::cout << "NFA exported to nfa.dot. Use Graphviz to generate an image:\n";
    std::cout << "    dot -Tpng nfa.dot -o nfa.png\n";

    res.exportToDOT("nfa.dot");

    return 0;
}

int main2() {
    NFA nfa1 = NFA({"S0", "S1", "S2"}, {"x", "z"}, "S0", {"S2"});
    nfa1.addTransition("S0", "z", {"S1"});
    nfa1.addTransition("S1", "x", {"S2"});

    nfa1.exportToDOT("nfa1.dot");

    return 0;
}

int main3() {
    NFA nfa1 = NFA({"S0", "S1", "S2"}, {"1", "0"}, "S0", {"S2"});
    nfa1.addTransition("S0", "1", {"S1"});
    nfa1.addTransition("S1", "1", {"S1", "S2"});
    nfa1.addTransition("S1", "0", {"S1"});
    nfa1.addTransition("S2", "0", {"S1"});
    nfa1.addTransition("S2", "1", {"S2"});

    nfa1.exportToDOT("nfa1.dot");

    return 0;
}

int main4() {
    NFA nfa1 = NFA({"S0", "S1", "S2"}, {"a", "b", "c"}, "S0", {"S2"});

    nfa1.addTransition("S0", "a", {"S1"});
    nfa1.addTransition("S1", "b", {"S2"});
    nfa1.addTransition("S2", "c", {"S2"});

    nfa1.exportToDOT("nfa1.dot");

    return 0;
}

int main() {
    NFA nfa1 = NFA({"S0", "S1", "S2"}, {"a", "b"}, "S0", {"S1"});

    nfa1.addTransition("S0", "a", {"S1"});
    nfa1.addTransition("S1", "b", {"S2"});
    nfa1.addTransition("S2", "c", {"S2"});

    NFA nfa2 = NFA({"Q0", "Q1"}, {"a", "b"}, "Q0", {"Q1"});

    nfa2.addTransition("Q0", "a", {"Q1"});
    nfa2.addTransition("Q1", "b", {"Q1"});

    NFA res = NFA::alternative(nfa1, nfa2);


    NFA res_w = res;
    res_w.removeUnreachable();
    res_w.exportToDOT("nfa1.dot");

    return 0;
}