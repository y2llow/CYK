#include "../include/CFG.h"
#include "../json.hpp"
#include <fstream>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <set>

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

// void CFG::accepts(string input) {
//     int n = input.length();
//
//     // CYK tabel: table[i][j] bevat de set van variabelen die substring vanaf i met lengte j+1 kunnen afleiden
//     vector table(n, vector<set<string>>(n));
//
//     // Stap 1: Vul de eerste rij (substrings van lengte 1)
//     for (int i = 0; i < n; i++) {
//         string symbol(1, input[i]);
//
//         // Zoek alle variabelen die deze terminal kunnen afleiden
//         for (const auto& production : P) {
//             string var = production.first[0];
//             for (const auto& body : production.second) {
//                 // Check voor unit productie: A -> a
//                 if (body.size() == 1 && body[0].size() == 1 && body[0][0] == symbol) {
//                     table[i][0].insert(var);
//                 }
//             }
//         }
//     }
//
//     // Stap 2: Vul de rest van de tabel (substrings van lengte 2 tot n)
//     for (int length = 2; length <= n; length++) {
//         for (int i = 0; i <= n - length; i++) {
//             int j = length - 1;
//
//             // Splits de substring op alle mogelijke manieren
//             for (int k = 0; k < length - 1; k++) {
//                 // Linker deel: table[i][k]
//                 // Rechter deel: table[i+k+1][j-k-1]
//
//                 set<string>& left = table[i][k];
//                 set<string>& right = table[i + k + 1][j - k - 1];
//
//                 // Zoek producties A -> BC waar B in left en C in right
//                 for (const auto& production : P) {
//                     string var = production.first[0];
//                     for (const auto& body : production.second) {
//                         // Check voor binaire productie: A -> B C
//                         if (body.size() == 1 && body[0].size() == 2) {
//                             string B = body[0][0];
//                             string C = body[0][1];
//
//                             if (left.count(B) && right.count(C)) {
//                                 table[i][j].insert(var);
//                             }
//                         }
//                     }
//                 }
//             }
//         }
//     }
//
//     // Print de tabel
//     for (int j = n - 1; j >= 0; j--) {
//         for (int i = 0; i <= n - j - 1; i++) {
//             cout << "| {";
//             bool first = true;
//             vector<string> sorted_vars(table[i][j].begin(), table[i][j].end());
//             sort(sorted_vars.begin(), sorted_vars.end());
//             for (const string& var : sorted_vars) {
//                 if (!first) cout << ", ";
//                 cout << var;
//                 first = false;
//             }
//             cout << "}  ";
//         }
//         cout << "|\n";
//     }
//
//     // Check of het startsymbool in de top cel zit
//     bool accepted = table[0][n - 1].count(S) > 0;
//     cout << (accepted ? "true" : "false") << endl;
// }

void CFG::accepts(string input) {
    // splits input up letter per letter
    vector<string> inputChar;
    for (char c : input) {
        inputChar.emplace_back(1, c);
    }

    int n = inputChar.size();

    // Maak de CYK tabel: CYK_table[length][start_pos]
    vector<vector<vector<string>>> CYK_table(n);
    for (int i = 0; i < n; ++i) {
        CYK_table[i].resize(n - i);
    }

    // Lijn 1 van de CYK: vul de basis (lengte 1)
    for (int _i = 0; _i < inputChar.size(); ++_i) {
        for (const auto& transities : this->P) {
            for (auto& trans : transities.second) {
                // Check voor unit productie A -> a
                if (trans.size() == 1 && trans[0].size() == 1 && trans[0][0] == inputChar[_i]) {
                    CYK_table[0][_i].push_back(transities.first[0]);
                }
            }
        }
    }

    // Vul de rest van de CYK tabel (lengtes 2 tot n)
    for (int length = 2; length <= n; ++length) {
        for (int start = 0; start <= n - length; ++start) {
            // Voor elke mogelijke split van de substring
            for (int split = 1; split < length; ++split) {
                int left_length = split;
                int right_length = length - split;
                int right_start = start + split;

                // Haal de linker en rechter delen op
                vector<string>& left = CYK_table[left_length - 1][start];
                vector<string>& right = CYK_table[right_length - 1][right_start];

                // Maak combinaties van linker en rechter variabelen
                vector<vector<string>> Combinaties;
                for (int i = 0; i < left.size(); ++i) {
                    for (int j = 0; j < right.size(); ++j) {
                        vector<string> temp;
                        temp.push_back(left[i]);
                        temp.push_back(right[j]);
                        Combinaties.push_back(temp);
                    }
                }

                // Check welke producties deze combinaties kunnen maken
                for (const auto& transities : this->P) {
                    for (auto& trans : transities.second) {
                        // Check voor binaire productie A -> BC
                        if (trans.size() == 1 && trans[0].size() == 2) {
                            string B = trans[0][0];
                            string C = trans[0][1];

                            // Vergelijk met alle combinaties
                            for (int _i = 0; _i < Combinaties.size(); ++_i) {
                                if (Combinaties[_i][0] == B && Combinaties[_i][1] == C) {
                                    CYK_table[length - 1][start].push_back(transities.first[0]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Print de CYK tabel
    for (int length = n; length >= 1; --length) {
        for (int start = 0; start <= n - length; ++start) {
            cout << "| {";

            // Sorteer en verwijder duplicaten
            vector<string> vars = CYK_table[length - 1][start];
            sort(vars.begin(), vars.end());
            vars.erase(unique(vars.begin(), vars.end()), vars.end());

            for (int i = 0; i < vars.size(); ++i) {
                cout << vars[i];
                if (i + 1 < vars.size()) cout << ", ";
            }
            cout << "}  ";
        }
        cout << "|\n";
    }

    // Check of het startsymbool in de top cel zit
    vector<string>& top = CYK_table[n - 1][0];
    bool accepted = find(top.begin(), top.end(), S) != top.end();
    cout << (accepted ? "true" : "false") << endl;
}
