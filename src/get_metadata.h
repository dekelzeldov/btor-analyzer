//
// Created by dekel on 5/20/22.
//

#ifndef BTOR_ANALYZER_GET_METADATA_H
#define BTOR_ANALYZER_GET_METADATA_H

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <btor2parser.h>
#include <iostream>
#include <cassert>

class getmetadata
{
public:
    void print_ite (char *model_path) {
        FILE *model_file;
        Btor2Parser *model;
        int64_t num_format_lines;
        std::vector<Btor2Line *> inits;
        std::vector<Btor2Line *> nexts;
        std::vector<Btor2Line *> lines;

        if (!(model_file = fopen (model_path, "r"))) {
            std::cout << "fopen failed" << std::endl;
            printf("failed to open BTOR model file '%s' for reading", "\n");
            exit(EXIT_FAILURE);
        }
        model = btor2parser_new ();
        if (!btor2parser_read_lines (model, model_file)) {
            std::cout << "read_lines failed" << std::endl;
            //die ("parse error in '%s' at %s", model_path, btor2parser_error (model));
            exit(EXIT_FAILURE);
        }
        num_format_lines = btor2parser_max_id (model);
        inits.resize (num_format_lines, nullptr);
        nexts.resize (num_format_lines, nullptr);
        Btor2LineIterator it = btor2parser_iter_init (model);
        Btor2Line *l;

        while ((l = btor2parser_iter_next (&it))) {
            line[l->id]=l;
            if (l->tag == BTOR2_TAG_ite) {
                assert (l->nargs == 3);
//                BtorSimState args[3];
//                for (uint32_t i = 0; i < l->nargs; i++) args[i] = simulate (l->args[i]);
                assert (l->args[0].type == BtorSimState::Type::BITVEC);
//                if (res.type == BtorSimState::Type::ARRAY)
//                {
//                    assert ((l->args[1]).type == BtorSimState::Type::ARRAY);
//                    assert (l->args[2].type == BtorSimState::Type::ARRAY);
//                    res.array_state = btorsim_am_ite (
//                            args[0].bv_state, args[1].array_state, args[2].array_state);
//                }
//                else
//                {
//                    assert (args[1].type == BtorSimState::Type::BITVEC);
//                    assert (args[2].type == BtorSimState::Type::BITVEC);
//                    res.bv_state = btorsim_bv_ite (
//                            args[0].bv_state, args[1].bv_state, args[2].bv_state);
//                }
            }
        }


        fclose (model_file);
        std::cout << "so far, so good!" << std::endl;

    }
};

#endif //BTOR_ANALYZER_GET_METADATA_H
