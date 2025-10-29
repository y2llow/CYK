#ifndef TEST_CFG_H
#define TEST_CFG_H

#include <utility>
#include <vector>
#include <map>
#include <string>

using namespace std;

class CFG {
public:
    vector<vector<string>> V;
    vector<string> T;
    map<vector<string>, vector<vector<vector<string>>>> P;
    string S;

    explicit CFG(const string &filename);
    CFG () = default;

    void print() const;

    void accepts(string input);
};

#endif