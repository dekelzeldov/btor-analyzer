
#include "metadata.h"
#include <string>

#include "aig/aig/aig.h"
#include "aigUtils.h"

using namespace std;
using namespace abc;


int main() {
    string btor2_path = "/home/dekel/CLionProjects/btor-analyzer/hwmcc20/btor2/bv/2020/mann/simple_alu.btor";
    string modified_btor2_path = "/home/dekel/CLionProjects/btor-analyzer/src/mdf.btor2";
    string aig_path = "/home/dekel/CLionProjects/btor-analyzer/src/out.aig";
    string btor2aiger_cmd = "/home/dekel/workspace/btor2tools/cmake-build-debug/bin/btor2aiger -a ";

    MetaData md(btor2_path.c_str());
    md.add_ite_conditions();
    md.add_conditions_states(modified_btor2_path.c_str());

    string run_cmd = btor2aiger_cmd + modified_btor2_path + " > " + aig_path;
    system(run_cmd.c_str());
    md.print_conditions();

    Gia_Man_t * gia_mng = gia_no_condStates(aig_path);

    Gia_ManStop(gia_mng);
}
