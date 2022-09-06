#include "metadata.h"

#include <string>
#include <stdio.h>

#include "aig/aig/aig.h"
#include "aigUtils.h"
#include "aig/gia/gia.h"

#include "btor2aiger.h"

using namespace std;
using namespace abc;


int main(int argc, char* argv[]) {

    if (argc != 2) return -1;

    string btor2_path = argv[1];

    // Create the MetaData object and read the btor file
    MetaData md(btor2_path.c_str());

    // Collect selected metadata
    md.collect_ite_conditions();
    md.print_btor_conditions();

    // Generate an AIG with metadata embedded into it
    Gia_Man_t *pAig = md.givemeAigWithMeta();

    // Generate a clean AIG
    Gia_Man_t * gia_mng_no_condStates = md.Gia_remove_condStates(pAig);
    md.print_gia_conditions();

    // Is this right? The function gets the AIG with metadata?
    Gia_Man_t * gia_mng_condSAT = md.Gia_make_condSAT(pAig);
    md.find_assignments(gia_mng_condSAT);
    md.print_assignments();

    // prove per assignment

    Gia_ManStop(pAig);
    Gia_ManStop(gia_mng_condSAT);
    Gia_ManStop(gia_mng_no_condStates);
}
