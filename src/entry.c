//Grupo 51: Felipe Heliszkowski 47064
//Gonçalo Cardoso 46784
//Pedro Gama 47081

#include "entry.h"
//#include "data.h" nao e necessario porque entry.h ja tem include de data.h ??
#include <stdlib.h>
#include <string.h>




/* Função que inicializa os membros de uma entrada na tabela com
 o valor NULL.
 */
void entry_initialize(struct entry_t *entry){
  
  if(entry == NULL)
    return;
  
  entry->key = NULL;
  
  entry->value = NULL;
  
  entry->next = NULL;
  
}

/* Função que duplica um par {chave, valor}.
 */
struct entry_t *entry_dup(struct entry_t *entry){
  
  if(entry == NULL)
    return NULL;
  
  struct entry_t *entryDup = (struct entry_t *) malloc(sizeof(struct entry_t));
  
  if(entryDup == NULL){
    free(entryDup);
    return NULL;
  }
  entryDup -> value = data_dup(entry->value);
  entryDup -> key = strdup(entry->key);
  entryDup -> next = entry->next;
  
  
  return entryDup;
  
}