#ifndef _PRIMARY_BACKUP_PRIVATE_H
#define _PRIMARY_BACKUP_PRIVATE_H

#include "inet.h"


struct server_t{
	struct sockaddr_in addr;
	int socket;
	struct table_t* tables;
	int state; 
};
#endif