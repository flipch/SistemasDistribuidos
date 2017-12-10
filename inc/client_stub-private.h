#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H
#include "inet.h"
#include "client_stub.h"

struct rtables_t{
	struct server_t*server;
    int numeroTabelas;
    int currentTable;
    int print;
};
#endif
