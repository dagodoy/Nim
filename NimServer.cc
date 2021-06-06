#include "Nim.h"

int main(int argc, char **argv)
{
    //./ns direccion puerto
    P2PServer es(argv[1], argv[2]);
    es.do_messages();

    return 0;
}
