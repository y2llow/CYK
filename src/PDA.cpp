#include "../include/PDA.h"
#include "../json.hpp"
#include <fstream>
#include <algorithm>
#include <sstream>
#include <iostream>

using json = nlohmann::json;

PDA::PDA(const string &filename) {
    // JSON-bestand openen
    ifstream input(filename);
    if (!input.is_open()) {
        cerr << "Fout: kon bestand '" << filename << "' niet openen." << endl;
        return;
    }

    json j;
    input >> j;

    //States inlezen
    if (j.contains("States") && j["States"].is_array()) {
        for (const auto &state : j["States"]) States.push_back(state.get<string>());
    }

    //Alphabet inlezen
    if (j.contains("Alphabet") && j["Alphabet"].is_array()) {
        for (const auto &alphabet : j["Alphabet"]) Alphabet.push_back(alphabet.get<string>());
    }

    // StackAlphabet inlezen
    if (j.contains("StackAlphabet") && j["StackAlphabet"].is_array()) {
        for (const auto &stackAlphabet : j["StackAlphabet"]) StackAlphabet.push_back(stackAlphabet.get<string>());
    }

    //transities inlezen
    if (j.contains("Transitions") && j["Transitions"].is_array()) {
        for (const auto &trans : j["Transitions"]) {
            Transition _transition;
            _transition.from = trans["from"].get<string>();
            _transition.input =  trans["input"].get<string>();
            _transition.stacktop = trans["stacktop"].get<string>();
            _transition.to = trans["to"].get<string>();


            if (trans.contains("replacement") && trans["replacement"].is_array()) {
                for (const auto &repl : trans["replacement"]) {
                    _transition.replacement.push_back(repl.get<string>());
                }
            }
            Transitions.push_back(_transition) ;
        }
    }


    //StartState inlezen
    if (j.contains("StartState")) {
        StartState = j["StartState"].get<string>();
    }

    //StartStack inlezen
    if (j.contains("StartStack")) {
        StartStack = j["StartStack"].get<string>();
    }
}

CFG PDA::toCFG() {
    CFG cfg ;

    // first we will get teh Varibales of the cfg from the pda
    vector<vector<string>> Variables{{"S"}};

    for (const string& State : this->States) {
        for (const string& Stacksym : this->StackAlphabet) {
            for (const string& ReturnState : this->States) {
                Variables.push_back({State,Stacksym,ReturnState});
            }
        }
    }
    cfg.V = Variables;
// //-----------------------------------------------------------------------------------------------------------------------------------------------
    // we get teh Termuinals form teh alphabet
    cfg.T = this->Alphabet;
//-----------------------------------------------------------------------------------------------------------------------------------------------      //S transitie met V[0] = S
    // get the S transitie
    for (const string& State : this->States) {
        vector<vector<string>> temp {{this->StartState, this->StartStack, State}};
        cfg.P[cfg.V[0]].push_back(temp);

    }

    // rest van de transities
    for (int _counter = 0 ; _counter < this->Transitions.size() ; _counter ++) {
        string from = this->Transitions[_counter].from;
        string stacktop = this->Transitions[_counter].stacktop;
        string input  = this->Transitions[_counter].input;
        string to = this->Transitions[_counter].to;
        vector<string> replacement = this->Transitions[_counter].replacement;

        if (replacement.size() == 0) {
            cfg.P[{from, stacktop, to}].push_back({{input}});
        }

        else if (replacement.size() == 1) {
            for (const string& State1 : this->States) {
                vector<vector<string>> temp{{input}, {from,replacement[0],State1}};
                    cfg.P[{from, stacktop, State1}].push_back(temp);

            }
        }

        else if (replacement.size() == 2) {
            for (const string& State1 : this->States) {
                for (const string& State2  : this->States) {
                    vector<vector<string>> temp{{input}, {from,replacement[0],State2} , {State2,replacement[1],State1}};
                    cfg.P[{from, stacktop, State1}].push_back(temp);
                }
            }
        }
    }
//-----------------------------------------------------------------------------------------------------------------------------------------------
    //Start variable will always be S this will always be he first variable in V
    cfg.S = cfg.V[0][0];

    return cfg;
}