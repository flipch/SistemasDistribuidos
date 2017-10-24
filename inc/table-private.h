//Grupo 51: Felipe Heliszkowski 47064
//Gonçalo Cardoso 46784
//Pedro Gama 47081

#ifndef _TABLE_PRIVATE_H
#define _TABLE_PRIVATE_H

#include "table.h"
#include "data.h"
struct table_t{
    int MAX_SIZE;//tamanho da tabela
    int size; //Elementos inseridos
    struct entry_t *entry;
    int colls; //Quantas colisoes
};

#endif
