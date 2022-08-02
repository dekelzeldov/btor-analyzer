
#include "metadata.h"
#include <string>

#include "aig/aig/aig.h"
#include "aigUtils.h"
#include "aig/gia/gia.h"
#include "sat/cnf/cnf.h"

#include "Glucose.h"

using namespace std;
using namespace abc;


int main(int argc, char* argv[]) {
    string btor2_path = "/home/yvizel/workspace/btor-analyzer/hwmcc20/btor2/bv/2020/mann/simple_alu.btor";
    string modified_btor2_path = "/home/yvizel/workspace/btor-analyzer/src/mdf.btor2";
    string aig_path = "/home/yvizel/workspace/btor-analyzer/src/out.aig";
    string aig_sat_path = "/home/yvizel/workspace/btor-analyzer/src/out_sat.aig";
    string btor2aiger_cmd = "/home/yvizel/workspace/btor2tools/build/bin/btor2aiger " + modified_btor2_path + " > " + aig_path;
    string aiger2aiger_cmd = "/home/yvizel/workspace/aiger/aigtoaig " + aig_sat_path + " -a";

    MetaData md(btor2_path.c_str());
    md.add_ite_conditions();
    md.add_conditions_states(modified_btor2_path.c_str());
    md.print_conditions();

    system(btor2aiger_cmd.c_str());

    Gia_Man_t * gia_mng_condSAT = md.Gia_condSAT(aig_path);
    // system(aiger2aiger_cmd.c_str());
    Cnf_Dat_t * pCnf = (Cnf_Dat_t *) Mf_ManGenerateCnf( gia_mng_condSAT, 8, 0, 0, 0, 0 );

    avy::Glucose g_sat(pCnf->nVars, false, true);
    for (unsigned i=0; i < pCnf->nClauses; i++) {
        g_sat.addClause(pCnf->pClauses[i], pCnf->pClauses[i + 1]);
    }

    Gia_Obj_t* pObj;
    unsigned obj_idx;
    vector<int> condVars;
    Gia_ManForEachCo(gia_mng_condSAT, pObj, obj_idx) {
        int var = pCnf->pVarNums[(Gia_ObjId(gia_mng_condSAT, pObj))];
        condVars.push_back(var);
    }

    while (g_sat.solve()) {
        std::cout << "Found an assignment..." << std::endl;
        vector<int> block;
        for (int var : condVars) {
            block.push_back(toLitCond(var, g_sat.getVarVal(var)));
        }
        g_sat.addClause(block.data(), block.data()+block.size());
    }

    Gia_ManStop(gia_mng_condSAT);

    Gia_Man_t * gia_mng_no_condStates = md.Gia_no_condStates(aig_path);

    Gia_ManStop(gia_mng_no_condStates);
}
