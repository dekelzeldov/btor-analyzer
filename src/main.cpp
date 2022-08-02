
#include "metadata.h"
#include <string>

#include "aig/aig/aig.h"
#include "aigUtils.h"
#include "aig/gia/gia.h"
#include "sat/cnf/cnf.h"

using namespace std;
using namespace abc;


int main() {
    string btor2_path = "/home/dekel/CLionProjects/btor-analyzer/hwmcc20/btor2/bv/2020/mann/simple_alu.btor";
    string modified_btor2_path = "/home/dekel/CLionProjects/btor-analyzer/src/mdf.btor2";
    string aig_path = "/home/dekel/CLionProjects/btor-analyzer/src/out.aig";
    string aig_sat_path = "/home/dekel/CLionProjects/btor-analyzer/src/out_sat.aig";
    string btor2aiger_cmd = "/home/dekel/workspace/btor2tools/cmake-build-debug/bin/btor2aiger " + modified_btor2_path + " > " + aig_path;
    string aiger2aiger_cmd = "/home/dekel/workspace/aiger/aigtoaig " + aig_sat_path + " -a";

    MetaData md(btor2_path.c_str());
    md.add_ite_conditions();
    md.add_conditions_states(modified_btor2_path.c_str());
    md.print_conditions();

    system(btor2aiger_cmd.c_str());

    Gia_Man_t * gia_mng_condSAT = md.Gia_condSAT(aig_path);
    system(aiger2aiger_cmd.c_str());
    Cnf_Dat_t * pCnf = (Cnf_Dat_t *) Mf_ManGenerateCnf( gia_mng_condSAT, 8, 1, 0, 0, 0 );
    Gia_ManStop(gia_mng_condSAT);

    // SAT solver
    //(A==(a&b||c))&(B==(x||y))&(C==...).... += &!(A&B&C)&!(A&!B&C)

    Gia_Man_t * gia_mng_no_condStates = md.Gia_no_condStates(aig_path);

    Gia_ManStop(gia_mng_no_condStates);
}
