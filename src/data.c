//Grupo 51: Felipe Heliszkowski 47064
//Gonçalo Cardoso 46784
//Pedro Gama 47081

#include "data.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Função que cria um novo elemento de dados data_t e reserva a memória
 * necessária, especificada pelo parâmetro size
 */
struct data_t *data_create(int size){
  
  if(size < 1)
    return NULL;
  
  struct data_t *st;
  
  st = (struct data_t *) malloc(sizeof(struct data_t));
  
  if(st == NULL)
    return NULL;
  
  st->datasize = size;
  st->data = malloc(size);// sem type cast porque e um ponteiro generico
  
  if(st->data == NULL){
    free(st); //se for NULL da free ao espaço alocado
    return NULL;
  }
  
  return st;
}



/* Função idêntica à anterior, mas que inicializa os dados de acordo com
 * o parâmetro data.
 */
struct data_t *data_create2(int size, void * data){
  
  
  if(size < 1 || data == NULL)
    return NULL;
  
  struct data_t *st;
  
  st = data_create(size);
  
  if(st == NULL)
    return NULL;
  
  memcpy(st->data,data,size);
  
  st->datasize = size;
  
  return st;
}



/* Função que destrói um bloco de dados e liberta toda a memória.
 */
void data_destroy(struct data_t *data){
  if(data != NULL){
    if(data->data !=NULL)
      free(data->data);
    free(data);
  }
}



/* Função que duplica uma estrutura data_t.
 */
struct data_t *data_dup(struct data_t *data){
  if(data != NULL)
    return data_create2(data->datasize,data->data);
  return NULL;
}
