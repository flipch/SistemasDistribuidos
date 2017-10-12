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

int message_to_buffer(struct message_t *msg, char **msg_buf)//endereço de um char *
{

  /* Verificar se msg é NULL */
  if (msg == NULL)// || msg_buf == NULL NAO E NECESSARIO PORQUE VAI ESTAR A NULL NO INICIO
    return -1;

  /* Consoante o msg->c_type, determinar o tamanho do vetor de bytes
     que tem de ser alocado antes de serializar msg
  */
  int size = 0;
  int allkeys = 0;

  short opcode, c_type,table_num;
  int buffer_size = 0;

  switch (msg->c_type)
  {
  case CT_RESULT:
    
    int result = htons(msg -> content.result);
    buffer_size = (_SHORT * 3) + _INT;
    
    *msg_buf = (char*) malloc(buffer_size);
    
    if(msg_buf == NULL)
      return -1;

      memcpy(*msg_buf,opcode,_SHORT);
      size = size + _SHORT;

      memcpy(*msg_buf + size,c_type,_SHORT);
      size = size + _SHORT;

      memcpy(*msg_buf + size,table_num,_SHORT);
      size = size + _SHORT;

      memcpy(*msg_buf + size,&result,_INT);
      size = size + _INT;
    break;
  case CT_ENTRY:
    
    int lenkey = strlen(msg->content.entry->key);
    int value = msg->content.entry->value->datasize;

    short key = htons(msg->content.entry->key);

    short value = htons(msg->content.entry->value->datasize);


    buffer_size = (_SHORT * 3 )+ 2 + strlen(msg->content.entry->key) + 4 + msg->content.entry->value->datasize;
    *msg_buf = (char*) malloc(buffer_size);

    if(msg_buf == NULL)
          return -1;

    memcpy(*msg_buf,opcode,_SHORT);
      size = size + _SHORT;

    memcpy(*msg_buf,c_type,_SHORT);
      size = size + _SHORT;

    memcpy(*msg_buf + size,table_num,_SHORT);
      size = size + _SHORT;

    memcpy(*msg_buf + size,&key,_SHORT);
      size = size + _SHORT;

    memcpy(*msg_buf + size,msg->content.entry->key,lenkey);
      size = size + lenkey;

    memcpy(*msg_buf + size,&value,_INT);
      size = size + _INT;

    memcpy(*msg_buf + size,msg->content.entry->value->datasize,value);
      size = size + value;

    break;
  case CT_KEY:


    //acho que temos que retirar o '/0' ultima posição da string


      
      short keylen = (short)strlen(msg->content.key);
      short key = htons(keylen);

      buffer_size = _SHORT * 3 + 2 + strlen(msg->content.key);

      *msg_buf = (char*) malloc(buffer_size); 

      if(msg_buf == NULL)
          return -1;

      memcpy(*msg_buf,opcode,_SHORT);
        size = size + _SHORT;

      memcpy(*msg_buf,c_type,_SHORT);
        size = size + _SHORT;

      memcpy(*msg_buf + size,table_num,_SHORT);
        size = size + _SHORT;

      memcpy(*msg_buf + size,&key,_SHORT);
      size = size + _SHORT;

      memcpy(*msg_buf + size,msg->content.key,keylen);
      size = size + keylen;


    break;
  case CT_KEYS:
    
    buffer_size = (_SHORT * 3) + 4;
    

    while (msg->content.keys[nkeys] != NULL)
    {
      buffer_size += 2 + strlen(msg->content.keys[allkeys]);
      allkeys++;
    }

    int numkeys = htons(allkeys);
     *msg_buf = (char*) malloc(buffer_size + 1 ); // contem mais uma posição '/0'

      if(msg_buf == NULL)
          return -1;



      memcpy(*msg_buf,opcode,_SHORT);
        size = size + _SHORT;

      memcpy(*msg_buf,c_type,_SHORT);
        size = size + _SHORT;

      memcpy(*msg_buf + size,table_num,_SHORT);
        size = size + _SHORT;

      memcpy(*msg_buf + size,&numkeys,_INT);
        size = size + _INT;

      int i;
      for(i = 0;i < allkeys ;i++){

        short hkeys = htons((short)strlen(msg->content.keys[i]));


        memcpy(*msg_buf + size,&hkeys,_SHORT);
        size = size + _SHORT;


        memcpy(*msg_buf + size,msg->content.keys[i],strlen(msg->content.keys[i]));
        size = size + strlen(msg->content.keys[i]);

      }

    break;
  case CT_VALUE:
    

    int sizeValue = msg->content.data->datasize;
    int hvalue = htons(sizeValue);

    buffer_size = _SHORT * 3 + 4 + msg->content.data->datasize;
    *msg_buf = (char*) malloc(buffer_size);

    if(msg_buf == NULL)
          return -1;


      memcpy(*msg_buf,opcode,_SHORT);
        size = size + _SHORT;

      memcpy(*msg_buf,c_type,_SHORT);
        size = size + _SHORT;

      memcpy(*msg_buf + size,table_num,_SHORT);
        size = size + _SHORT;

      memcpy(*msg_buf + size,&hvalue,_INT);
        size = size + _INT;

      memcpy(*msg_buf + size,msg->content.data->datasize,sizeValue);
        size = size + sizeValue;




    break;
  
  }

  
 
  
  
  /* Alocar quantidade de memória determinada antes 
   *msg_buf = ....
   */
 
  if(msg_buf ==NULL)
    return -1;
  /* Inicializar ponteiro auxiliar com o endereço da memória alocada */
  char *ptr = *msg_buf;
  

  /* Serializar número da tabela */
  
  opcode  = htons(msg->opcode);
  memcpy(ptr, &short_v, _SHORT);
  ptr += _SHORT;
  
  c_type = htons(msg->c_type);
  memcpy(ptr, &short_v, _SHORT);
  ptr += _SHORT;
  
  table_num = htons(msg->table_num);
  memcpy(ptr, &short_v, _SHORT);
  ptr += _SHORT;
  
  short_value = htons(msg->content_u);
  memcpy(ptr, &short_v, _SHORT);
  ptr += _SHORT;

    
    /* Consoante o conteúdo da mensagem, continuar a serialização da mesma */
/*
    switch(msg->c_type){
      
      case CT_RESULT :

      case CT_VALUE:

      case CT_KEY : 

      case CT_ENTRY :

      case CT_KEYS : 

      case 0:

      default : 
  	
  */  
    
    

  //return 
  return buffer_size;
}

struct message_t *buffer_to_message(char *msg_buf, int msg_size)
{
  
  
  
  //IGUAL A SERIALIZAR MAS OPERACAO CONTRARIA NTONS
  

  /* Verificar se msg_buf é NULL */
  if (msg_buf == NULL ||msg_size < 1 )
    return NULL;
  /* msg_size tem tamanho mínimo ? */

  /* Alocar memória para uma struct message_t */
  struct message_t *msg = (struct message_t *) malloc( sizeof (struct message_t));
  
  if(msg == NULL)
    return NULL;
  
  
  /* Recuperar o opcode e c_type */
  
  
  memcpy(&short_aux, msg_buf, _SHORT);
  msg->opcode = ntohs(short_aux);
  msg_buf += _SHORT;

  memcpy(&short_aux, msg_buf, _SHORT);
  msg->c_type = ntohs(short_aux);
  msg_buf += _SHORT;

  /* Recuperar table_num */

  /* Exedmplo da mesma coisa que em cima mas de forma compacta, ao estilo C! 

  msg->opcode = ntohs(*(short *) msg_buf++);
  msg->c_type = ntohs(*(short *) ++msg_buf);
  msg_buf += _SHORT;

  */

  /* O opcode e c_type são válidos? */

  /* Consoante o c_type, continuar a recuperação da mensagem original */
      
  switch(msg->c_type){
      
      case CT_RESULT :
        
      case CT_VALUE:

      case CT_KEY : 

      case CT_ENTRY :

      case CT_KEYS : 

      case 0:

      default :       
      
        free(msg);
      

  //return msg;
  return msg;
}
