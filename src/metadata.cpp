#include "metadata.h"
#include "aigUtils.h"

using namespace std;
using namespace abc;

void MetaData::collect_ite_conditions () {
    Btor2LineIterator it = btor2parser_iter_init (parser);
    Btor2Line *l;
    while ((l = btor2parser_iter_next (&it))) {
        if (l->tag == BTOR2_TAG_ite) {
            assert (l->nargs == 3);
            Btor2Line* cond_line = btor2parser_get_line_by_id(parser, l->args[0]);
            if (btor_conds.count(cond_line->id)){
                btor_conds.at(cond_line->id).cnt_app++;
            } else {
                meta md = meta(cond_line->id);
                btor_conds.insert(pair<int64_t, meta>(cond_line->id, md));
                btor_conds.at(cond_line->id).cnt_app++;
            }
        }
    }
    std::cout << "done collecting ite conditions" << std::endl;

    add_conditions_states();
}

void MetaData::add_conditions_states () {

    int64_t line_id = btor2parser_max_id(parser);
    for (auto & cond : btor_conds) {
        ///////
        BoolectorSort bsort = m_btor2_model.get_sort(btor2parser_get_line_by_id(parser, cond.first)->sort.id);
        BoolectorNode *bcond = m_btor2_model.get_node(cond.first);
        string state_name = cond_prefix;
        const char *cond_symbol = boolector_get_symbol(m_btor2_model.btor, bcond);
        //state_name += (cond_symbol != nullptr) ? cond_symbol : to_string(cond.first);
        state_name += to_string(cond.first);
        BoolectorNode *bzero = boolector_zero(m_btor2_model.btor ,bsort);
        BoolectorNode *bstate = boolector_var(m_btor2_model.btor, bsort, state_name.c_str());
        cout << "Adding " << state_name << endl;

        /// For now, a simple hack, add line_id
        /// The Btor2Model class is based on line numbers instead of
        /// object IDs.
        /// TODO: Extend that class to use an object ID based mapping
        int32_t bzero_id = line_id + boolector_get_node_id(m_btor2_model.btor, bzero);
        int32_t bstate_id = line_id + boolector_get_node_id(m_btor2_model.btor, bstate);
        if (m_btor2_model.nodes.find(bzero_id) == m_btor2_model.nodes.end()) {
            m_btor2_model.add_node(bzero_id, bzero);
        }
        m_btor2_model.add_node(bstate_id, bstate);
        m_btor2_model.states[bstate_id] = bstate;
        m_btor2_model.init[bstate_id] = bzero;
        m_btor2_model.next[bstate_id] = bcond;
    }
    std::cout << "done adding condition states" << std::endl;
}

Gia_Man_t* MetaData::givemeAigWithMeta() {
    string aig_path = "/tmp/out.aig";

    aiger * aig = generate_aiger (givemeBtor2Model(), false);
    FILE *aig_file = fopen(aig_path.c_str(), "w");
    aiger_write_to_file (aig, aiger_binary_mode, aig_file);
    fclose(aig_file);
    ///

    Gia_Man_t * pAig = loadAig(aig_path);
    return pAig;
}

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
        if (pObjName == strstr(pObjName, cond_prefix.c_str())) {
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
        if (pObjName == strstr(pObjName, cond_prefix.c_str())) {
            numCondCo++;
            int cond_num = condNum(pObjName);
            assert(btor_conds.count(cond_num));
            gia_conds.insert(pair <int64_t, meta> (Gia_ObjFanin0Copy(pObj), btor_conds.at(cond_num)));
            stateObj_to_noStateObj.insert(pair <int, int64_t> (Gia_ObjId(p, pObj), Gia_ObjFanin0Copy(pObj)));
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

Gia_Man_t * MetaData::Gia_DupUnMarked( Gia_Man_t * p )
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
        else if ( Gia_ObjIsCo(pObj) )
        {
            pObj->Value = Gia_ManAppendCo( pNew, Gia_ObjFanin0Copy(pObj)  );
            satObj_to_stateObj.insert(pair <int, int> (pObj->Value/2, Gia_ObjId(p, pObj)));
        }
        else if ( Gia_ObjIsAnd(pObj) ) {
            pObj->Value = Gia_ManAppendAnd(pNew, Gia_ObjFanin0Copy(pObj), Gia_ObjFanin1Copy(pObj));
        }
        else
        {
            assert( Gia_ObjIsCi(pObj) );
            pObj->Value = Gia_ManAppendCi( pNew);
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
        if (pObjName == strstr(pObjName, cond_prefix.c_str())) {
            Gia_clearMark0ForFaninRec(pObj);
        }
    }
    Gia_Man_t * pNew = Gia_DupUnMarked(p);
    Gia_ManCheckMark0(p);
    assert(Gia_ManCoNum(pNew) == btor_conds.size());
    return pNew;
}

void MetaData::find_assignments(Gia_Man_t * p) {
    auto pCnf = (Cnf_Dat_t *) Mf_ManGenerateCnf(p, 8, 0, 0, 0, 0);

    avy::Glucose g_sat(pCnf->nVars, false, true);
    for (unsigned i = 0; i < pCnf->nClauses; i++) {
        g_sat.addClause(pCnf->pClauses[i], pCnf->pClauses[i + 1]);
    }

    Gia_Obj_t *pObj;
    int obj_idx;
    Gia_ManForEachCo(p, pObj, obj_idx) {
        int var = pCnf->pVarNums[(Gia_ObjId(p, pObj))];
        var_to_satObj.insert(pair <int, int> (var, Gia_ObjId(p, pObj)));
    }
    assert(var_to_satObj.size() == btor_conds.size());

    while (g_sat.solve()) {
        vector<int> block;
        map<int, bool> ass;
        for (auto const& [var, satCo] : var_to_satObj) {
            block.push_back(toLitCond(var, g_sat.getVarVal(var)));
            ass.insert(pair <int, bool> (satCo_to_noStateObj(satCo), g_sat.getVarVal(var)));
        }
        assert(ass.size() == btor_conds.size());
        g_sat.addClause(block.data(), block.data() + block.size());
        assignments.push_back(ass);
    }
}