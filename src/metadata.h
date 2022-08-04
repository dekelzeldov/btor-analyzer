//
// Created by dekel on 5/20/22.
//

#ifndef BTOR_ANALYZER_METADATA_H
#define BTOR_ANALYZER_METADATA_H

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <cassert>
#include <fstream>
#include <set>
#include <string>
#include <sstream>
#include <map>

#include "base/abc/abc.h"
#include "aig/aig/aig.h"
#include "aig/gia/gia.h"
#include "aig/gia/giaAig.h"
#include "aig/ioa/ioa.h"
#include "base/main/main.h"

#include <btor2parser.h>
#include <btor2aiger.h>

using namespace std;
using namespace abc;

struct meta {
    int cnt_app;
    meta() : cnt_app(0){}
};

class MetaData
{
    Btor2Parser *parser;
    Btor2Model m_btor2_model;
    const char *model_path;
    std::map <int64_t, meta*> btor_conds = {};
    std::map <int64_t, meta*> gia_conds = {};

    const char * cond_prefix = "__cond_line_";

public:
    explicit MetaData (const char *model_path) : model_path(model_path){
        FILE *model_file;
        if (!(model_file = fopen (model_path, "r"))) {
            std::cout << "fopen failed" << std::endl;
            printf("failed to open BTOR model file '%s' for reading", "\n");
            exit(EXIT_FAILURE);
        }

        parser = btor2parser_new ();
        if (!btor2parser_read_lines (parser, model_file)) {
            std::cout << "read_lines failed" << std::endl;
            const char *err = btor2parser_error (parser);
            std::cout << err << std::endl;
            fclose(model_file);
            exit(EXIT_FAILURE);
        }

        rewind(model_file);
        parse_btor2(model_file, m_btor2_model);

        fclose(model_file);
    }

    Btor2Model& givemeBtor2Model() { return m_btor2_model; }

    void collect_ite_conditions () {
        Btor2LineIterator it = btor2parser_iter_init (parser);
        Btor2Line *l;
        while ((l = btor2parser_iter_next (&it))) {
            if (l->tag == BTOR2_TAG_ite) {
                assert (l->nargs == 3);
                Btor2Line* cond_line = btor2parser_get_line_by_id(parser, l->args[0]);
                if (btor_conds.count(cond_line->id)){
                    btor_conds.at(cond_line->id)->cnt_app++;
                } else {
                    meta * md = new meta();
                    btor_conds.insert(std::pair<int64_t, meta*>(cond_line->id, md));
                    btor_conds.at(cond_line->id)->cnt_app++;
                }
            }
        }
        std::cout << "done collecting ite conditions" << std::endl;
    }

    static bool copyFile(const char * SRC, const char * DEST)
    {
        std::ifstream src(SRC, std::ios::binary);
        std::ofstream dest(DEST, std::ios::binary);
        dest << src.rdbuf();
        return src && dest;
    }

    void add_conditions_states (const char * modified_model_path) {
        copyFile (model_path, modified_model_path);
        std::ofstream outFile(modified_model_path, std::ios_base::app);

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

    std::map <int64_t, meta*> get_map() {
        return btor_conds;
    }

    void print_conditions () {
        std::cout << std::endl << "state conditions: " << std::endl;
        for (auto & cond : btor_conds) {
            std::cout << cond.first << " => appearances: " << cond.second->cnt_app << '\n';
        }
    }

    long num_cond(){
        assert (btor_conds.size() == gia_conds.size());
        return (int) btor_conds.size();
    }

    //aigUtils
    Gia_Man_t * Gia_remove_condStates(Gia_Man_t *p);
    Gia_Man_t * Gia_no_condStates(const string& aig_path);
    Gia_Man_t * Gia_make_condSAT(Gia_Man_t *p);
    Gia_Man_t * Gia_condSAT(const string& aig_path);
};

#endif //BTOR_ANALYZER_METADATA_H
