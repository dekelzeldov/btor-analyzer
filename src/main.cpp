#include <iostream>
#include "btor2parser.h"
#include "metadata.h"
#include <string>
#include <vector>
#include <cassert>

#include "aig/aig/aig.h"
#include "aig/gia/gia.h"
#include "aig/ioa/ioa.h"
#include "base/main/main.h"

using namespace std;
using namespace abc;

namespace ABC_NAMESPACE {
    extern Aig_Man_t *Abc_NtkToDar(Abc_Ntk_t *pNtk, int fExors, int fRegisters);
}

static Aig_Man_t *loadAig(std::string fname) {
    Abc_Frame_t *pFrame = Abc_FrameGetGlobalFrame();

    //VERBOSE(2, vcut() << "\tReading AIG from '" << fname << "'\n";);
    string cmd = "read " + fname;
    Cmd_CommandExecute(pFrame, cmd.c_str());

    Abc_Ntk_t *pNtk = Abc_FrameReadNtk(pFrame);

    return Abc_NtkToDar(pNtk, 0, 1);
}

int main() {
    string btor2_path = "/home/dekel/CLionProjects/btor-analyzer/hwmcc20/btor2/bv/2020/mann/simple_alu.btor";
    string modified_btor2_path = "/home/dekel/workspace/btor2tools/examples/btorsim/mdf.btor2";
    string out_path = "/home/dekel/workspace/btor2tools/examples/btorsim/out.aig";
    string btor2aiger_cmd = "/home/dekel/workspace/btor2tools/cmake-build-debug/bin/btor2aiger -a ";

    MetaData md(btor2_path.c_str());
    md.add_ite_conditions();
    md.add_conditions_states(modified_btor2_path.c_str());

    string run_cmd = btor2aiger_cmd + modified_btor2_path + " > " + out_path;
    system(run_cmd.c_str());
    md.print_conditions();
}
