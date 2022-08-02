
#include "metadata.h"
#include <string>

#include "aig/aig/aig.h"
#include "aigUtils.h"
#include "aig/gia/gia.h"

using namespace std;
using namespace abc;


int main(int argc, char* argv[]) {
    string btor2_path = "/home/dekel/CLionProjects/btor-analyzer/examples/simple_alu.btor";
    string modified_btor2_path = "/home/dekel/CLionProjects/btor-analyzer/src/mdf.btor2";
    string aig_path = "/home/dekel/CLionProjects/btor-analyzer/src/out.aig";
    string aig_sat_path = "/home/dekel/CLionProjects/btor-analyzer/src/out_sat.aig";
    string btor2aiger_cmd = "/home/dekel/workspace/btor2tools/cmake-build-debug/bin/btor2aiger " + modified_btor2_path + " > " + aig_path;
    string aiger2aiger_cmd = "/home/dekel/workspace/aiger/aigtoaig " + aig_sat_path + " -a";

    MetaData md(btor2_path.c_str());
    md.add_ite_conditions();
    md.add_conditions_states(modified_btor2_path.c_str());
    md.print_btor_conditions();

    system(btor2aiger_cmd.c_str());

    Gia_Man_t * gia_mng_no_condStates = md.Gia_no_condStates(aig_path);
    md.print_gia_conditions();

    Gia_Man_t * gia_mng_condSAT = md.Gia_condSAT(aig_path);    //print with system(aiger2aiger_cmd.c_str());
    md.find_assignments(gia_mng_condSAT);
    md.print_assignments();

    // prove per assignment

    Gia_ManStop(gia_mng_condSAT);
    Gia_ManStop(gia_mng_no_condStates);
}
