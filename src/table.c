//Grupo 51: Felipe Heliszkowski 47064
//Gonçalo Cardoso 46784
//Pedro Gama 47081

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table.h"
#include "table-private.h"

/* A definir pelo grupo em table-private.h */
struct table_t;

int hash(char *key, int size)
{
  int sum = 0;
  int tamanho = strlen(key);

  if (size == 0)
    return 0;

  if (tamanho < 5)
  {
    for (int i = 0; i < tamanho; i++)
    {
      sum = sum + key[i];
    }
  }
  else
  {
    sum = key[0] + key[1] + key[tamanho - 2] + key[tamanho - 1];
  }
  return sum % size;
}

/* Função para criar/inicializar uma nova tabela hash, com n
 * linhas(n = módulo da função hash) 
 */
struct table_t *table_create(int n)
{ //5

  struct table_t *table;
  if (n < 1)
    return NULL;
  table = (struct table_t *)malloc(sizeof(struct table_t));

  if (table == NULL)
    return NULL;

  table->entry = (struct entry_t *)malloc(n * sizeof(struct entry_t)); //TA AQUI

  if (table->entry == NULL)
  {
    free(table);
    return NULL;
  }

  table->size = 0;
  table->MAX_SIZE = n;
  table->colls = 0;

  int i;
  for (i = 0; i < n; i++)
  {
    entry_initialize(&(table->entry[i]));
  }
  return table;
}

/* Libertar toda a memória ocupada por uma tabela.
 */
void table_destroy(struct table_t *table)
{

  if (table != NULL)
  {
    int i;
    for (i = 0; i < table->MAX_SIZE; i++)
    {
      data_destroy(table->entry[i].value);
      free(table->entry[i].key);
    }
    free(table->entry);
  }
  free(table);
}

/* Função para adicionar um par chave-valor na tabela.
 * Os dados de entrada desta função deverão ser copiados.
 * Devolve 0 (ok) ou -1 (out of memory, outros erros)
 */
int table_put(struct table_t *table, char *key, struct data_t *value)
{

  if (table == NULL || key == NULL || value == NULL)
    return -1;

  if (table->size == table->MAX_SIZE)
    return -1;

  int hashKey = hash(key, table->MAX_SIZE);

  struct entry_t *a = &(table->entry[hashKey]);

  if (a->key == NULL)
  { //no collision
    a->key = strdup(key);
    a->value = data_dup(value);
  }
  else
  {
    table->colls++;
    if (strcmp(table->entry[hashKey].key, key) == 0)
      return -1; //keys iguais a serem put return -1
    while (a->next != NULL)
    {
      a = a->next;
      if (strcmp(a->key, key) == 0) //a->key
        return -1;
    }
    int i = table->MAX_SIZE - 1;
    while (i >= 0 && table->entry[i].key != NULL)
      i--;
    table->entry[i].key = strdup(key);
    table->entry[i].value = data_dup(value);
    a->next = &(table->entry[i]);
  }
  table->size++;
  return 0;
}

/* Função para substituir na tabela, o valor associado à chave key.
 * Os dados de entrada desta função deverão ser copiados.
 * Devolve 0 (OK) ou -1 (out of memory, outros erros)
 */

int table_update(struct table_t *table, char *key, struct data_t *value)
{

  if (table == NULL || key == NULL || value == NULL)
    return -1;

  int hk = hash(key, table->MAX_SIZE);

  if (table->entry[hk].key == NULL) // Casa vazia nao ha nada para dar update
    return -1;

  struct entry_t *a = &(table->entry[hk]);

  while (a != NULL && strcmp(a->key, key) != 0)
  {
    a = a->next;
  }
  if (a == NULL)
    return -1;
  data_destroy(a->value);
  a->value = data_dup(value);
  return 0;

  return -1;
}

/* Função para obter da tabela o valor associado à chave key.
 * A função deve devolver uma cópia dos dados que terão de
 * ser libertados no contexto da função que chamou table_get.
 * Devolve NULL em caso de erro.
 */

struct data_t *table_get(struct table_t *table, char *key)
{

  int hk = hash(key, table->MAX_SIZE);

  struct entry_t *e = &table->entry[hk];
  if (e->value == NULL)
    return NULL;
  struct data_t *valor = NULL;

  while (e != NULL && strcmp(key, e->key) != 0)
  {
    e = e->next;
  }

  if (e != NULL)
  {
    valor = data_dup(e->value);
  }

  return valor;
}

/* Devolve o número de elementos na tabela.
 */
int table_size(struct table_t *table)
{
  //criamos um elemento = 0 sempre que ha um elemento damos update com o table put
  if (table->MAX_SIZE == 0)
    return -1;
  return table->size;
}

/* Devolve o número de colisões na tabela.
 */
int table_colls(struct table_t *table)
{
  //criamos um elemento = 0 sempre que ha um elemento damos update com o table put
  if (table == NULL)
    return -1;
  return table->colls;
}

/* Devolve um array de char * com a cópia de todas as keys da
 * tabela, e um último elemento a NULL.
 */
char **table_get_keys(struct table_t *table)
{

  if (table == NULL)
    return NULL;

  char **list_keys = (char **)malloc(sizeof(char *) * (table->size + 1));

  if (list_keys == NULL)
    return NULL;

  int i;
  int j = 0;

  for (i = 0; i < table->MAX_SIZE; i++)
  {
    if (table->entry[i].key != NULL)
    {
      list_keys[j] = strdup(table->entry[i].key);
      j++;
    }
  }
  list_keys[j] = NULL;
  return list_keys;
}

/* Liberta a memória alocada por table_get_keys().
 */
void table_free_keys(char **keys)
{
  int count = 0;
  if (keys != NULL)
  {
    while (keys[count] != NULL)
    {
      free(keys[count]);
      count++;
    }
    free(keys);
  }
}
