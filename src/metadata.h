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

using namespace std;

class MetaData
{
    friend class Btor2Parser;

    Btor2Parser *parser;
    const char *model_path;
    set <int64_t> input_conditions;
    set <int64_t> to_state_conditions;

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
                if (cond_line->tag == BTOR2_TAG_input) {
                    input_conditions.insert(cond_line->id);
                } else {
                    to_state_conditions.insert(cond_line->id);
                }
            }
        }
        cout << "done adding ite conditions" << endl;
    }

    static bool copyFile(const char *SRC, const char* DEST)
    {
        ifstream src(SRC, ios::binary);
        ofstream dest(DEST, ios::binary);
        dest << src.rdbuf();
        return src && dest;
    }

    void add_conditions_states (const char * modified_model_path) {
        copyFile (model_path, modified_model_path);
        ifstream infile(model_path);
        ofstream outFile(modified_model_path);

        string line;
        while (getline(infile, line))
        {
            int lineid;
            string input;
            int sortid;
            string name;
            string end;
            stringstream sline(line);
            sline >> lineid;
            if (input_conditions.count(lineid)) {
                sline >> input;
                sline >> sortid;
                sline >> name;
                assert (!(sline >> end));
                outFile << lineid << " ";
                outFile << input << " ";
                outFile << sortid << " ";
                outFile << "__cond_input_" << name << "\n";

            } else {
                outFile << line << "\n";
            }
        }

        int64_t line_id = btor2parser_max_id(parser);
        for(const int64_t id: to_state_conditions) {
            // 6 state 4 counter
            // 5 zero 4
            // 7 init 4 6 5
            // 12 next 4 6 11
            string sort = to_string(btor2parser_get_line_by_id(parser, id)->sort.id);
            string cond_id = to_string(id);
            string zero_id = to_string(++line_id);
            string state_id = to_string(++line_id).append(" ");

            outFile << zero_id  << " zero " << sort << endl;
            outFile << state_id << "state " << sort << " __cond_line_" << cond_id << endl;
            outFile << to_string(++line_id) << " init " << sort << " " << state_id << zero_id << endl;
            outFile << to_string(++line_id) << " next " << sort << " " << state_id << cond_id << endl;
        }
        cout << "done adding condition states" << endl;
    }

    void print_conditions () {
        cout << endl << "input conditions: " << endl;
        for(const int64_t id: input_conditions) {
            cout << to_string(id) << " ";
        }
        cout << endl;

        cout << endl << "state conditions: " << endl;
        for(const int64_t id: to_state_conditions) {
            cout << to_string(id) << " ";
        }
        cout << endl;
    }
};

#endif //BTOR_ANALYZER_METADATA_H
