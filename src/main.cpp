#include <iostream>
#include "btor2parser.h"
#include "metadata.h"
#include <string>
#include <vector>
#include <cassert>

using namespace std;

int main() {
    string btor2aiger_cmd = "/home/dekel/workspace/btor2tools/cmake-build-debug/bin/btor2aiger -a ";
    string btor2_path = "/home/dekel/workspace/btor2tools/examples/btorsim/twocount32.btor2";
    string modified_btor2_path = "/home/dekel/workspace/btor2tools/examples/btorsim/mdf.btor2";
    string out_path = "/home/dekel/workspace/btor2tools/examples/btorsim/out.aig";

    MetaData md(btor2_path.c_str());
    md.add_ite_conditions();
    md.add_conditions_states(modified_btor2_path.c_str());

    string run_cmd = btor2aiger_cmd + modified_btor2_path + " > " + out_path;
    system(run_cmd.c_str());
    md.print_conditions();
}
