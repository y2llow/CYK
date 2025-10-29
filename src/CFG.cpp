#include "../include/CFG.h"
#include "../json.hpp"
#include <fstream>
#include <algorithm>
#include <sstream>
#include <iostream>

using json = nlohmann::json;

CFG::CFG(const string &filename) {
    std::ifstream input(filename);
    if (!input.is_open()) {
        std::cerr << "Fout: kon bestand '" << filename << "' niet openen." << std::endl;
        return;
    }

    json j;
    input >> j;

    // Variabelen (as vector<vector<string>>)
    if (j.contains("Variables") && j["Variables"].is_array()) {
        for (const auto &v : j["Variables"]) {
            // Each variable is a string, but we wrap it inside a vector<string>
            V.push_back({v.get<string>()});
        }
    }

    // Terminals
    if (j.contains("Terminals") && j["Terminals"].is_array()) {
        for (const auto &t : j["Terminals"]) {
            T.push_back(t.get<string>());
        }
    }

    // Producties
    if (j.contains("Productions") && j["Productions"].is_array()) {
        for (const auto &prod : j["Productions"]) {
            string head = prod["head"].get<string>();
            vector<string> body;

            if (prod.contains("body") && prod["body"].is_array()) {
                for (const auto &sym : prod["body"]) {
                    body.push_back(sym.get<string>());
                }
            }

            // Now we wrap both the head and body in extra vectors to match your map type
            P[{head}].push_back({body});
        }
    }

    // Startsymbool
    if (j.contains("Start")) {
        S = j["Start"].get<string>();
    }
}

void CFG::print() const {
    std::ostringstream out;

    // --- V ---
    out << "V = {";
    auto sortedV = V;
    std::sort(sortedV.begin(), sortedV.end(), [](const std::vector<std::string> &a, const std::vector<std::string> &b) {
        for (size_t i = 0; i < std::min(a.size(), b.size()); ++i) {
            if (a[i] != b[i])
                return a[i] < b[i]; // ASCII-vergelijking
        }
        return a.size() < b.size();
    });
    for (size_t i = 0; i < sortedV.size(); i++) {
        const auto &var = sortedV[i];
        if (var.size() == 1) {
            out << var[0];
        } else {
            out << "[";
            for (size_t j = 0; j < var.size(); ++j) {
                out << var[j];
                if (j + 1 != var.size()) out << ",";
            }
            out << "]";
        }
        if (i + 1 != sortedV.size()) out << ", ";
    }
    out << "}\n";

    // --- T ---
    out << "T = {";
    auto sortedT = T;
    std::sort(sortedT.begin(), sortedT.end());
    for (size_t i = 0; i < sortedT.size(); i++) {
        out << sortedT[i];
        if (i + 1 != sortedT.size()) out << ", ";
    }
    out << "}\n";

    // --- P ---
    out << "P = {\n";

    // Sort keys ASCII-style
    std::vector<std::vector<std::string>> sortedKeys;
    for (const auto &pair : P) sortedKeys.push_back(pair.first);
    std::sort(sortedKeys.begin(), sortedKeys.end(), [](const std::vector<std::string> &a, const std::vector<std::string> &b) {
        for (size_t i = 0; i < std::min(a.size(), b.size()); ++i) {
            if (a[i] != b[i])
                return a[i] < b[i]; // ASCII compare
        }
        return a.size() < b.size();
    });

    // Print each production
    for (const auto &key : sortedKeys) {
        auto it = P.find(key);
        if (it == P.end()) continue;
        const auto &productions = it->second;

        // sort bodies inside each key (optional but consistent)
        auto sortedProductions = productions;
        std::sort(sortedProductions.begin(), sortedProductions.end(), [](const std::vector<std::vector<std::string>> &a,
                                                                         const std::vector<std::vector<std::string>> &b) {
            if (a.empty() || b.empty()) return a.size() < b.size();
            for (size_t i = 0; i < std::min(a[0].size(), b[0].size()); ++i) {
                if (a[0][i] != b[0][i])
                    return a[0][i] < b[0][i];
            }
            return a.size() < b.size();
        });

        for (const auto &bodySet : sortedProductions) {
            // left-hand side
            out << "    ";
            if (key.size() == 1) {
                out << key[0];
                if (key[0].size() < 3) out << "   ";
                else out << " ";
            } else {
                out << "[";
                for (size_t j = 0; j < key.size(); ++j) {
                    out << key[j];
                    if (j + 1 != key.size()) out << ",";
                }
                out << "]  ";
                if (key.size() == 3) out << " ";
            }

            out << "-> `";

            // right-hand side
            for (size_t j = 0; j < bodySet.size(); ++j) {
                const auto &part = bodySet[j];
                if (j > 0) out << " ";
                if (part.size() == 1) {
                    out << part[0];
                } else {
                    out << "[";
                    for (size_t k = 0; k < part.size(); ++k) {
                        out << part[k];
                        if (k + 1 != part.size()) out << ",";
                    }
                    out << "]";
                }
            }
            out << "`\n";
        }
    }

    out << "}\n";

    // --- S ---
    out << "S = " << S;

    std::cout << out.str() << std::endl;
}

void CFG::accepts(string input) {
    // splits input up letter per letter
    vector<string> inputChar; // Declare the vector of strings
    for (char c : input) {
        inputChar.emplace_back(1, c); // Create a string with one character and push it back
    }

    // Maak een nieuwe vector voor de eerste lijn van de CYK en reserveer ruimte
    vector<vector<string>> CYK_line1(inputChar.size()); // Reserve space for each input character

    // Nu kun je veilig elementen toevoegen
    for (int _i = 0; _i < inputChar.size(); ++_i) {
        for (const auto& transities : this->P) {
            for (auto& trans : transities.second) {
                if (trans[0][0] == inputChar[_i]) {  // Vergelijk de eerste letter van de transitie
                    CYK_line1[_i].push_back(transities.first[0]);  // Voeg de transitie toe
                }
            }
        }
    }

    // De 2de lijn van de CYK
    vector<vector<string>> CYK_line2(inputChar.size() ); // Reserve space for each input character
    vector <vector<vector<string>>> SuperCombinaties;

    for (int _i = 0; _i < CYK_line2.size(); ++_i) {
        vector <vector<string>> Combinaties;
        for (int i = 0; i < CYK_line1[_i].size(); ++i) {
            for (int j = 0; j < CYK_line1[_i + 1].size(); ++j) {
                vector <string> temp;
                temp.push_back(CYK_line1[_i][i]);
                temp.push_back(CYK_line1[_i + 1][j]);
                Combinaties.emplace_back(temp);
            }
        }
        SuperCombinaties.push_back(Combinaties);
    }

    // De 3de lijn van de CYK
    vector<vector<string>> CYK_line3(inputChar.size() - 1); // Reserve space for each input character

    // Nu gaan we door de combinaties om te kijken of er matches zijn met de transities
    for (int __i = 0; __i < CYK_line2.size(); ++__i) {
        for (const auto& transities : this->P) {
            for (auto& trans : transities.second) {
                for (int c = 0; c < SuperCombinaties.size(); ++c) {
                    // Loop door de transities
                    for (int i = 0; i < trans.size(); ++i) {
                        for (int j = 0; j < trans[i].size() - 1; ++j) {
                            // Vergelijk alle combinaties met alle transities
                            for (int _i = 0; _i < SuperCombinaties[c].size(); ++_i) {
                                for (int _j = 0; _j < SuperCombinaties[c][_i].size() - 1; ++_j) {
                                    if (trans[i][j] == SuperCombinaties[c][_i][_j] &&
                                        trans[i][j + 1] == SuperCombinaties[c][_i][_j + 1]) {
                                        CYK_line3[c].push_back(transities.first[__i]);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return;

}
