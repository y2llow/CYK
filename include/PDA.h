#ifndef MB_PROGRAMMEEROPDRACHTEN_PDA_H
#define MB_PROGRAMMEEROPDRACHTEN_PDA_H

#include <utility>
#include <vector>
#include <map>
#include <string>
#include "CFG.h"

using namespace std;

struct Transition {
    string from;
    string input;
    string stacktop;
    string to;
    vector<string> replacement;
};

class PDA {
public:
    vector<std::string> States;
    vector<std::string> Alphabet;
    vector<std::string> StackAlphabet;
    vector <Transition> Transitions ;
    string StartState;
    string StartStack;

    explicit PDA(const string &filename);

    CFG toCFG();
};



#endif //MB_PROGRAMMEEROPDRACHTEN_PDA_H