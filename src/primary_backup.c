#include "primary_backup.h"

/* Função usada para um servidor avisar o servidor “server” de que
* já acordou. Retorna 0 em caso de sucesso, -1 em caso de insucesso
*/
int hello(struct server_t *server)
{
  int result;
  //Enviar mensagem a server com o nosso heartbeat
  //Se server esta a null nem nos preocupamos
  if (server == NULL)
  {
    return -1;
  }
  struct message_t *message_p = (struct message_t *)malloc(sizeof(struct message_t));
  //Se a mensagem esta a null houve falha no malloc
  //Possivel falta de memoria?
  if (message_p == NULL)
  {
    perror("Malloc da mensagem durante o hello\n");
    return -1;
  }

  //Init das variaveis
  message_p->opcode = OC_HEARTHBEAT;
  message_p->c_type = CT_RESULT;
  message_p->content.result = 0;
  message_p->table_num = 0;

  struct message_t *message_r;
  message_r = network_send_receive(server, message_p);
  if (message_p == NULL)
  {
    perror("Network send receive falhou, durante o hello\n");
    free_message(message_p);
    return -1;
  }
  // Please no leakerino
  result = message_r->content.result;
  free_message(message_p);
  free_message(message_r);
  return result;
}

/* Pede atualizacao de estado ao server.
* Retorna 0 em caso de sucesso e -1 em caso de insucesso.
*/
int update_state(struct server_t *server)
{
  int result;
  //Enviar mensagem a server com o nosso heartbeat
  //Se server esta a null nem nos preocupamos
  if (server == NULL)
  {
    return -1;
  }

  //Referencia das tabelas antes dos conteudos
  struct table_t *table;
  struct table_t *tables;
  int n_tabelas;

  char **sizes_tabelas = malloc(n_tabelas * sizeof(char *));
  //Table_skell_init recebe o argv puro, ou seja, ainda tem
  //Na casa 1 o nome do binario, e 2 a porta

  sprintf(sizes_tabelas[0], "%d", -1);
  sprintf(sizes_tabelas[1], "%d", -1);
  do
  {
    //Pega as tabelas ate retornar uma tabela 0x0
    //Talvez usar este metodo no rtables_bind em vez daquilo que esta atualmente
    table = get_table(n_tabelas);
    if (table != NULL)
    {
      sprintf(sizes_tabelas[n_tabelas], "%d", table->MAX_SIZE);
    }
    n_tabelas++;
  } while (table != NULL);

  //Apagar o quer que esteja ainda ali
  table_skel_destroy();
  //Reinicializar as tabelas com os sizes
  table_skel_init(sizes_tabelas);

  int index;
  struct message_t *msg_r;
  struct message_t *msg_p;
  char **keys;
  for (index = 0; index <= n_tabelas; index++)
  {

    //Pegar todas as keys por tabela ao servidor pretendido
    msg_p = malloc(sizeof(struct message_t));
    if (msg_p == NULL)
    {
      //Ai os leaks
      perror("Malloc falhou, falta de memoria?\n");
      return -1;
    }

    msg_p->opcode = OC_GET;
    msg_p->table_num = index;
    msg_p->c_type = CT_KEY;
    msg_p->content.key = "*";

    msg_r = network_send_receive(server, msg_p);
    if (msg_r == NULL)
    {
      free_message(msg_p);
      perror("Servidor nao respondeu ao pedido de keys * durante o update_state\n");
      return -1;
    }
    if (msg_r->opcode != OC_GET + 1 && (msg_r->c_type != CT_VALUE || msg_r->c_type != CT_KEYS))
    {
      free_message(msg_p);
      free_message(msg_r);
      perror("Bug no servidor, OC errados para OC_GET de Keys *\n");
      return -1;
    }
    keys = msg_r->content.keys;

    //No longer needed for now
    free_message(msg_p);
    free_message(msg_r);

    int indexK; //index keys

    while (keys[indexK] != NULL)
    {
      //Pegar o valor da chave
      msg_p = malloc(sizeof(struct message_t));
      if (msg_p == NULL)
      {
        perror("Falha no malloc, falta de memoria?\n");
        return -1;
      }
      msg_p->opcode = OC_GET;
      msg_p->table_num = index;
      msg_p->c_type = CT_KEY;
      msg_p->content.key = strdup(keys[indexK++]);

      msg_r = network_send_receive(server, msg_p);
      if (msg_r == NULL)
      {
        perror("Servidor nao respondeu durante o update_state\n");
        free_message(msg_p);
        return -1;
      }
      if (msg_r->opcode != OC_GET + 1 && (msg_r->c_type != CT_VALUE || msg_r->c_type != CT_KEYS))
      {
        perror("OC errados para GET da key\n");
        free_message(msg_p);
        free_message(msg_r);
        return -1;
      }

      struct data_t *data;
      data = data_dup(msg_r->content.data);
      free_message(msg_r);
      free_message(msg_p);

      //Fazer o put na tabela local
      msg_p = malloc(sizeof(struct message_t));
      if (msg_p == NULL)
      {
        return -1;
      }
      msg_p->opcode = OC_PUT;
      msg_p->table_num = index;
      msg_p->c_type = CT_ENTRY;

      msg_p->content.entry = (struct entry_t *)malloc(sizeof(struct entry_t));
      if (msg_p->content.entry == NULL)
      {
        perror("Falha no malloc, falta de memoria?\n");
        free_message(msg_p);
        return -1;
      }

      msg_p->content.entry->value = data_dup(data);
      if (msg_p->content.entry->value == NULL)
      {
        free_message(msg_p);
        return -1;
      }

      msg_r = invoke(msg_p);
      if ((msg_r->opcode != OC_PUT + 1 && msg_r->c_type != CT_RESULT) || (msg_r->content.result == -1))
      {
        free_message(msg_p);
        free_message(msg_r);
        return -1;
      }
      free_message(msg_p);
      free_message(msg_r);
    }

    //All gut
    return 0;
  }
}