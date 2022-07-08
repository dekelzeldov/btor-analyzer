//
// Created by dekel on 7/7/22.
//

#ifndef BTOR_ANALYZER_AIGUTILS_H
#define BTOR_ANALYZER_AIGUTILS_H

#include <iostream>
#include "btor2parser.h"
#include "metadata.h"
#include <string>
#include <vector>
#include <cassert>

#include "aig/aig/aig.h"
#include "aig/gia/gia.h"
#include "aig/gia/giaAig.h"
#include "aig/ioa/ioa.h"
#include "base/main/main.h"

using namespace std;
using namespace abc;

namespace ABC_NAMESPACE {
    extern Aig_Man_t *Abc_NtkToDar(Abc_Ntk_t *pNtk, int fExors, int fRegisters);
}


static Aig_Man_t *loadAig(const std::string& fname) {
    Abc_Frame_t *pFrame = Abc_FrameGetGlobalFrame();

    //VERBOSE(2, vcut() << "\tReading AIG from '" << fname << "'\n";);
    string cmd = "read " + fname;
    Cmd_CommandExecute(pFrame, cmd.c_str());

    Abc_Ntk_t *pNtk = Abc_FrameReadNtk(pFrame);

    return Abc_NtkToDar(pNtk, 0, 1);
}


Aig_Man_t *Aig_remove_condStates(Aig_Man_t *p) {

    // create the new manager
    Aig_Man_t *pNew = Aig_ManStart(Aig_ManObjNumMax(p));
    pNew->pName = Abc_UtilStrsav(p->pName);
    pNew->pSpec = Abc_UtilStrsav(p->pSpec);
    pNew->nTruePis = p->nTruePis;
    pNew->nTruePos = p->nConstrs;  //OK?
    pNew->nRegs = p->nRegs;  //OK?

    // -- move nodes
    Aig_ManConst1(p)->pData = Aig_ManConst1(pNew);

    // -- inputs
    int i;
    Aig_Obj_t *pObj;
    Aig_ManForEachCi(p, pObj, i) {
        pObj->pData = Aig_ObjCreateCi(pNew);
    }

    // duplicate internal nodes
    Aig_ManForEachNode(p, pObj, i) {
            pObj->pData =
                    Aig_And(pNew, Aig_ObjChild0Copy(pObj), Aig_ObjChild1Copy(pObj));
        }
/*
    // create constraint outputs
    Saig_ManForEachPo(p, pObj, i) {
        if (i < Saig_ManPoNum(p) - Saig_ManConstrNum(p))
            continue;
        Aig_ObjCreateCo(pNew, Aig_Not(Aig_ObjChild0Copy(pObj)));
    }

    if (fKeepRegs) {
        // -- registers
        Saig_ManForEachLi(p, pObj, i)
        Aig_ObjCreateCo(pNew, Aig_ObjChild0Copy(pObj));
    }

    if (nPo >= 0) {
        AVY_ASSERT(Aig_ObjChild0Copy(Aig_ManCo(p, nPo)) != NULL);
        Aig_ObjCreateCo(pNew, Aig_ObjChild0Copy(Aig_ManCo(p, nPo)));
    }
*/
    Aig_ManCleanData(p);
    Aig_ManCleanup(pNew);
    return pNew;
}

Gia_Man_t * gia_no_condStates(const string& aig_path) {
    Aig_Man_t *aig_mng = loadAig(aig_path);
    Aig_Man_t *aig_new_mng = Aig_remove_condStates(aig_mng);
    Gia_Man_t * gia_mng = Gia_ManFromAigSimple(aig_new_mng);
    Aig_ManStop(aig_mng);
    Aig_ManStop(aig_new_mng);
    return gia_mng;
}



#endif //BTOR_ANALYZER_AIGUTILS_H
