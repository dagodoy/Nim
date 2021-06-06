#include <thread>
#include "Nim.h"

int main(int argc, char **argv)
{
    //./nc direccion puerto nombre oponente
    NimClient ec(argv[1], argv[2], argv[3], argv[4]);
}

