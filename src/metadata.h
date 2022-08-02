//
// Created by dekel on 5/20/22.
//

#ifndef BTOR_ANALYZER_METADATA_H
#define BTOR_ANALYZER_METADATA_H

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <btor2parser.h>
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

using namespace std;
using namespace abc;

using namespace std;

struct meta {
    int cnt_app;
    meta() : cnt_app(0){}
};

class MetaData
{
    Btor2Parser *parser;
    const char *model_path;
    map <int64_t, meta*> btor_conds = {};
    map <int64_t, meta*> gia_conds = {};

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

        fclose(model_file);
    }

    void add_ite_conditions () {
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
                    btor_conds.insert(pair<int64_t, meta*>(cond_line->id, md));
                    btor_conds.at(cond_line->id)->cnt_app++;
                }
            }
        }
        cout << "done adding ite conditions" << endl;
    }

    static bool copyFile(const char * SRC, const char * DEST)
    {
        ifstream src(SRC, ios::binary);
        ofstream dest(DEST, ios::binary);
        dest << src.rdbuf();
        return src && dest;
    }

    void add_conditions_states (const char * modified_model_path) {
        copyFile (model_path, modified_model_path);
        ofstream outFile(modified_model_path, std::ios_base::app);

        int64_t line_id = btor2parser_max_id(parser);
        for (auto & cond : btor_conds) {
            // 6 state 4 counter
            // 5 zero 4
            // 7 init 4 6 5
            // 12 next 4 6 11
            string sort = to_string(btor2parser_get_line_by_id(parser, cond.first)->sort.id);
            string cond_id = to_string(cond.first);
            string zero_id = to_string(++line_id);
            string state_id = to_string(++line_id).append(" ");

            outFile << zero_id  << " zero " << sort << endl;
            outFile << state_id << "state " << sort << " " << cond_prefix << cond_id << endl;
            outFile << to_string(++line_id) << " init " << sort << " " << state_id << zero_id << endl;
            outFile << to_string(++line_id) << " next " << sort << " " << state_id << cond_id << endl;
        }
        cout << "done adding condition states" << endl;
    }

    map <int64_t, meta*> get_map() {
        return btor_conds;
    }

    void print_conditions () {
        cout << endl << "state conditions: " << endl;
        for (auto & cond : btor_conds) {
            cout << cond.first << " => appearances: " << cond.second->cnt_app << '\n';
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
