#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//#include <arpa/inet.h> //Where's the lib boy ayyy lmao

#include "message.h"

void free_message(struct message_t *msg)
{

  /* Verificar se msg é NULL */
  if (msg == NULL)
    return;

  /* Se msg->c_type for:
      VALOR, libertar msg->content.data
      ENTRY, libertar msg->content.entry
      CHAVES, libertar msg->content.keys
      CHAVE, libertar msg->content.key
  */
  int i = 0;
  switch (msg->c_type)
  {
  case CT_VALUE:
    data_destroy(msg->content.data);
    free(msg->content.data);
    break;
  case CT_ENTRY:
    data_destroy(msg->content.entry->value);
    free(msg->content.entry->key);
    // Free do content.entry->next ??
    free(msg->content.entry);
    break;
  case CT_KEYS:

    do
    {
      if (msg->content.keys[i] == NULL)
        break;
      free(msg->content.keys[i]);
      i++;
    } while (msg->content.keys[i] != NULL);
  case CT_KEY:
    free(msg->content.key);
    break;
  }

  /* libertar msg */
  free(msg);
  return;
}

int message_to_buffer(struct message_t *msg, char **msg_buf)
{

  /* Verificar se msg é NULL */
  if (msg == NULL || msg_buf == NULL)
    return -1;

  /* Consoante o msg->c_type, determinar o tamanho do vetor de bytes
     que tem de ser alocado antes de serializar msg
  */
  int size = -1;
  int nkeys = 0;
  switch (msg->c_type)
  {
  case CT_ENTRY:
    size = sizeof(short) * 3 + 2 + strlen(msg->content.entry->key) + 4 + msg->content.entry->value->datasize;
    break;
  case CT_KEY:
    size = sizeof(short) * 3 + 2 + strlen(msg->content.key);
    break;
  case CT_KEYS:
    size = sizeof(short) * 3 + 4;
    while (msg->content.keys[nkeys] != NULL)
    {
      size += 2 + strlen(msg->content.keys[nkeys]); // Ask prof
      nkeys++;
    }
    break;
  case CT_VALUE:
    size = sizeof(short) * 3 + 4 + msg->content.data->datasize;
    break;
  case CT_RESULT:
    size = sizeof(short) * 3 + 4;
    break;
  }

  /* Alocar quantidade de memória determinada antes 
     *msg_buf = ....
  */
  *msg_buf = malloc(size);

  /* Inicializar ponteiro auxiliar com o endereço da memória alocada */
  /*ptr = *msg_buf;
  
  short_value = htons(msg->opcode);
  memcpy(ptr, &short_v, _SHORT);
  ptr += _SHORT;

  short_value = htons(msg->c_type);
  memcpy(ptr, &short_v, _SHORT);
  ptr += _SHORT;*/

  char **ptr = *msg_buf;
  int short_value, short_v;

  short_value = htons(msg->opcode);
  memcpy(ptr, &short_v, _SHORT);
  ptr += _SHORT;

  short_value = htons(msg->c_type);
  memcpy(ptr, &short_v, _SHORT);
  ptr += _SHORT;

  /* Serializar número da tabela */

  /* Consoante o conteúdo da mensagem, continuar a serialização da mesma */

  //return buffer_size;
  return 0;
}

struct message_t *buffer_to_message(char *msg_buf, int msg_size)
{

  /* Verificar se msg_buf é NULL */
  if (msg_buf == NULL)
    return -1;
  /* msg_size tem tamanho mínimo ? */
  if (msg_size < 1)
    return -1;
  /* Alocar memória para uma struct message_t */
  struct message_t *msg = (struct message_t *) malloc( sizeof (struct message_t));
  /* Recuperar o opcode e c_type */
  /*memcpy(&short_aux, msg_buf, _SHORT);
  msg->opcode = ntohs(short_aux);
  msg_buf += _SHORT;

  memcpy(&short_aux, msg_buf, _SHORT);
  msg->c_type = ntohs(short_aux);
  msg_buf += _SHORT;*/

  /* Recuperar table_num */

  /* Exedmplo da mesma coisa que em cima mas de forma compacta, ao estilo C! 

  msg->opcode = ntohs(*(short *) msg_buf++);
  msg->c_type = ntohs(*(short *) ++msg_buf);
  msg_buf += _SHORT;

  */

  /* O opcode e c_type são válidos? */

  /* Consoante o c_type, continuar a recuperação da mensagem original */

  //return msg;
  return 0;
}
