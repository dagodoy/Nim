#include "Chat.h"

int main(int argc, char **argv)
{
    P2PServer es(argv[1], argv[2]);
    es.do_messages();

    return 0;
}
