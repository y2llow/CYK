#include <iostream>
// #include "CFG.h"
#include "include/PDA.h"
using namespace std;

int main() {
    PDA pda("/home/y2llow/CLionProjects/MB_ProgrammeerOpdrachten/input/input-pda2cfg1.json");
    pda.toCFG().print();
    return 0;
}

// using namespace std;
// int main() {
//     CFG cfg("/home/y2llow/CLionProjects/MB_ProgrammeerOpdrachten/CFG.json");
//     cfg.print();
//     return 0;
// }