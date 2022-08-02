//
// Created by dekel on 7/7/22.
//

#ifndef BTOR_ANALYZER_AIGUTILS_H
#define BTOR_ANALYZER_AIGUTILS_H

#include "metadata.h"
#include <regex>


static Gia_Man_t *loadAig(const std::string& fname) {
    Abc_Frame_t *pFrame = Abc_FrameGetGlobalFrame();

    string cmd = "read " + fname + " ; zero; &get -n";
    Cmd_CommandExecute(pFrame, cmd.c_str());

    Gia_Man_t * pAig = Abc_FrameReadGia(pFrame);
    return pAig;
}
/*
void convert_to_ascii(const std::string& fin, const std::string& fout){
    Abc_Frame_t *pFrame = Abc_FrameGetGlobalFrame();
    string rcmd = "read " + fin + " ; zero; &get -n";
    Cmd_CommandExecute(pFrame, rcmd.c_str());
    string wcmd = "write_aiger" + fout;
    Cmd_CommandExecute(pFrame, wcmd.c_str());
}
*/
Gia_Man_t * MetaData::Gia_remove_condStates(Gia_Man_t *p) {
    Gia_Man_t *pNew;

    pNew = Gia_ManStart(Gia_ManObjNum(p) - 2*btor_conds.size()); // - key*2
    Gia_ManHashStart(pNew);
    pNew->pName = Abc_UtilStrsav(p->pName);
    pNew->pSpec = Abc_UtilStrsav(p->pSpec);
    Gia_ManConst0(p)->Value = 0;

    Gia_Obj_t *pObj;
    int i;
    char * pObjName;

    int numCond = 0;

    Gia_ManForEachCi(p, pObj, i) {
        pObjName = Gia_ObjCiName(p, i);
        if (pObjName == strstr(pObjName, cond_prefix)) {
            numCond++;
        } else {
            pObj->Value = Gia_ManAppendCi(pNew);
        }
    }

    Gia_ManForEachAnd(p, pObj, i) {
            pObj->Value = Gia_ManHashAnd(pNew, Gia_ObjFanin0Copy(pObj), Gia_ObjFanin1Copy(pObj));
        }

    int numCondCo = 0;
    Gia_ManForEachCo(p, pObj, i) {
        pObjName = Gia_ObjCoName(p, i);
        if (pObjName == strstr(pObjName, cond_prefix)) {
            numCondCo++;
            int cond_num = stoi(regex_replace(pObjName, regex("[^0-9]*([0-9]+).*"), string("$1")));
            assert(btor_conds.count(cond_num));
            gia_conds.insert(pair <int64_t, meta*> (Gia_ObjFanin0Copy(pObj), btor_conds.at(cond_num)));
        } else {
            pObj->Value = Gia_ManAppendCo(pNew, Gia_ObjFanin0Copy(pObj));
        }
    }

    assert(numCond == numCondCo);
    assert(numCond == btor_conds.size());
    assert(gia_conds.size() == btor_conds.size());

    Gia_ManSetRegNum(pNew, Gia_ManRegNum(p) - numCond);

    Gia_ManHashStop(pNew);
    assert(Gia_ManIsNormalized(pNew));

    Gia_Man_t *pClean = Gia_ManCleanup(pNew);
    Gia_ManStop(pNew);

    assert(Gia_ManIsNormalized(pClean));

    return pClean;
}

void Gia_clearMark0ForFaninRec(Gia_Obj_t *pObj){
    if(!pObj->fMark0){
        return;
    }
    pObj->fMark0=0;
    if(Gia_ObjIsCo(pObj)){
        Gia_clearMark0ForFaninRec(Gia_ObjFanin0(pObj));
    } else if(Gia_ObjIsAnd(pObj)){
        Gia_clearMark0ForFaninRec(Gia_ObjFanin0(pObj));
        Gia_clearMark0ForFaninRec(Gia_ObjFanin1(pObj));
    } else {
        assert(Gia_ObjIsCi(pObj));
    }
}

Gia_Man_t * Gia_DupUnMarked( Gia_Man_t * p )
{
    Gia_Man_t * pNew;
    Gia_Obj_t * pObj;
    int i;
    int CountMarked = 0;
    Gia_ManForEachObj( p, pObj, i )
        CountMarked += pObj->fMark0;
    Gia_ManFillValue( p );
    pNew = Gia_ManStart( Gia_ManObjNum(p) - CountMarked );
    pNew->nConstrs = p->nConstrs;
    pNew->pName = Abc_UtilStrsav( p->pName );
    pNew->pSpec = Abc_UtilStrsav( p->pSpec );
    Gia_ManConst0(p)->Value = 0;
    Gia_ManForEachObj1( p, pObj, i )
    {
        if ( pObj->fMark0 )
        {
            pObj->fMark0 = 0;
        }
        else if ( Gia_ObjIsAnd(pObj) )
        {
            pObj->Value = Gia_ManAppendAnd( pNew, Gia_ObjFanin0Copy(pObj), Gia_ObjFanin1Copy(pObj) );
        }
        else if ( Gia_ObjIsCi(pObj) )
        {
            pObj->Value = Gia_ManAppendCi( pNew );
        }
        else
        {
            assert( Gia_ObjIsCo(pObj) );
            pObj->Value = Gia_ManAppendCo( pNew, Gia_ObjFanin0Copy(pObj) );
        }
    }
    assert( pNew->nObjsAlloc == pNew->nObjs );
    Gia_ManSetRegNum( pNew, 0 );
    return pNew;
}

Gia_Man_t * MetaData::Gia_make_condSAT(Gia_Man_t *p) {
    Gia_Obj_t *pObj;
    int i;
    char * pObjName;
    Gia_ManSetMark0(p);
    Gia_ManConst0(p)->fMark0 = 0;
    Gia_ManForEachCo(p, pObj, i) {
        pObjName = Gia_ObjCoName(p, i);
        if (pObjName == strstr(pObjName, cond_prefix)) {
            Gia_clearMark0ForFaninRec(pObj);
        }
    }
    Gia_Man_t * pNew = Gia_DupUnMarked(p);
    Gia_ManCleanMark0(p);
    assert(Gia_ManCoNum(pNew) == btor_conds.size());
    return pNew;
}

Gia_Man_t * MetaData::Gia_no_condStates(const string& aig_path) {
    Gia_Man_t * gia_mng = loadAig(aig_path);
    Gia_Man_t * gia_no_condSt = Gia_remove_condStates(gia_mng);
    Gia_ManStop(gia_mng);
    return gia_no_condSt;
}

Gia_Man_t * MetaData::Gia_condSAT(const string& aig_path) {
    Gia_Man_t * gia_mng = loadAig(aig_path);
    Gia_Man_t * gia_cond_sat = Gia_make_condSAT(gia_mng);
    Gia_ManStop(gia_mng);
    return gia_cond_sat;
}

#endif //BTOR_ANALYZER_AIGUTILS_H
