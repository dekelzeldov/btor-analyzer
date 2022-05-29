#include <iostream>
#include "btor2parser.h"
#include "metadata.h"

int main() {
    getmetadata metadata;
    metadata.print_ite("/home/dekel/workspace/btor2tools/examples/btorsim/twocount2c.btor2");
}
