#include "metadata.h"

#include <string>
#include <stdio.h>

#include "aig/aig/aig.h"
#include "aigUtils.h"
#include "aig/gia/gia.h"
#include "sat/cnf/cnf.h"

#include "Glucose.h"

#include "btor2aiger.h"

using namespace std;
using namespace abc;


int main(int argc, char* argv[]) {
    if (argc != 2) return -1;

    string btor2_path = argv[1];
    string modified_btor2_path = "/tmp/mdf.btor2";
    string aig_path = "/tmp/out.aig";
    string aig_sat_path = "/tmp/out_sat.aig";

    MetaData md(btor2_path.c_str());
    md.collect_ite_conditions();
    md.add_conditions_states(modified_btor2_path.c_str());
    md.print_conditions();

    /// XXX BTOR -> AIGER
    FILE *infile = fopen(modified_btor2_path.c_str(), "r");
    Btor2Model model;
    parse_btor2 (infile, model);
    fclose (infile);
    aiger * aig = generate_aiger (md.givemeBtor2Model(), false);
    FILE *aig_file = fopen(aig_path.c_str(), "w");
    aiger_write_to_file (aig, aiger_binary_mode, aig_file);
    fclose(aig_file);
    ///

    Gia_Man_t * gia_mng_condSAT = md.Gia_condSAT(aig_path);
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
