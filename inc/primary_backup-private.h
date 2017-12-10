#ifndef _PRIMARY_BACKUP_PRIVATE_H
#define _PRIMARY_BACKUP_PRIVATE_H

#include "primary_backup.h"
#include "network_client.h"
#include "table-private.h"
#include <stdlib.h>
#include "inet.h"
#include "table.h"
#include <signal.h>
#include "table_skel-private.h"

#define ALIVE 1
#define DEAD 0

struct server_t{
  int socket;
  struct sockaddr_in addr;
  struct server_t *secondary;  //NULL => servidor e secundario
  struct server_t *primary;    //NULL => servidor e primario
  int alive;  //Estou vivo?
  int other_alive;  //O vizinho esta vivo?
};

#endif