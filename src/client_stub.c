#include "client_stub.h"

/* Remote table. A definir pelo grupo em client_stub-private.h 
 */
struct rtables_t;

/* Função para estabelecer uma associação entre o cliente e um conjunto de
 * tabelas remotas num servidor.
 * Os alunos deverão implementar uma forma de descobrir quantas tabelas
 * existem no servidor.
 * address_port é uma string no formato <hostname>:<port>.
 * retorna NULL em caso de erro .
 */
struct rtables_t *rtables_bind(const char *address_port)
{

	int numero_tabelas = 0;
	int maisTabelas = 1;

	if (address_port == NULL)
		return NULL;

	struct rtables_t *res = (struct rtables_t *)malloc(sizeof(struct rtables_t));
	if (res == NULL)
		return NULL;
	res->print = 0;
	res->server = network_connect(address_port);
	if (res->server == NULL)
	{
		free(res);
		return NULL;
	}
	else
	{
		res->currentTable = 0;
		do
		{
			if (rtables_size(res) == -1)
			{

				if (res->currentTable == 0)
				{
					//Se a primeira casa possivel nao tem tamanho, o servidor nao tem tabelas sequer
					free(res);
					return NULL;
				}
				else
				{
					//Tabela não existente, fim do numero de tabelas
					maisTabelas = 0;
				}
			}
			else
			{
				//Tem algum tamanho, nao nos interessa qual
				numero_tabelas++;
				res->currentTable++;
			}
		} while (maisTabelas == 1);
	}
	res->print = 1;
	return res;
}

/* Termina a associação entre o cliente e um conjunto de tabelas remotas, e
 * liberta toda a memória local. 
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtables_unbind(struct rtables_t *rtables)
{
	if (rtables == NULL)
		return -1;
	if ((network_close(rtables->server)) == 0)
	{
		free(rtables);
		return 0;
	}
	free(rtables);
	return -1;
}

/* Função para adicionar um par chave valor numa tabela remota.
 * Devolve 0 (ok) ou -1 (problemas).
 */
int rtables_put(struct rtables_t *rtables, char *key, struct data_t *value)
{

	if (key == NULL)
		return -1;
	if (value == NULL)
		return -1;
	if (rtables == NULL)
		return -1;

	struct message_t *msg_2;
	struct message_t *msg = (struct message_t *)malloc(sizeof(struct message_t));
	if (msg == NULL)
		return -1;

	msg->opcode = OC_PUT;
	msg->c_type = CT_ENTRY;
	msg->table_num = rtables->currentTable;

	msg->content.entry = (struct entry_t *)malloc(sizeof(struct entry_t));
	if ((msg->content.entry) == NULL)
	{
		free(msg);
		return -1;
	}

	msg->content.entry->value = data_dup(value);
	if (msg->content.entry->value == NULL)
	{
		free(msg);
		return -1;
	}

	msg->content.entry->key = strdup(key);
	if (msg->content.entry->key == NULL)
	{
		free(msg);
		return -1;
	}

	msg_2 = malloc(sizeof(struct message_t));
	if (msg_2 == NULL)
	{
		free(msg);
		return -1;
	}

	msg_2 = network_send_receive(rtables->server, msg);
	if (msg_2 == NULL)
	{
		msg_2->opcode = OC_RT_ERROR;
		msg_2->c_type = CT_RESULT;
		msg_2->content.result = -1;
	}

	if (msg_2->opcode != OC_PUT + 1 && msg_2->c_type != CT_RESULT)
	{
		msg_2->opcode = OC_RT_ERROR;
		msg_2->c_type = CT_RESULT;
		msg_2->content.result = -1;
	}

	//print_message(msg);   //needed???
	if (rtables->print == 1)
		print_message(msg_2);
	free(msg);
	free(msg_2);
	return 0;
}

/* Função para substituir na tabela remota, o valor associado à chave key.
 * Devolve 0 (OK) ou -1 em caso de erros.
 */
int rtables_update(struct rtables_t *rtables, char *key, struct data_t *value)
{

	if (key == NULL)
		return -1;
	if (value == NULL)
		return -1;
	if (rtables == NULL)
		return -1;

	struct message_t *msg_2;
	struct message_t *msg = (struct message_t *)malloc(sizeof(struct message_t));
	if (msg == NULL)
		return -1;

	msg->opcode = OC_UPDATE;
	msg->c_type = CT_ENTRY;
	msg->table_num = rtables->currentTable;

	msg->content.entry = (struct entry_t *)malloc(sizeof(struct entry_t));
	if ((msg->content.entry) == NULL)
	{
		free(msg);
		return -1;
	}

	msg->content.entry->key = strdup(key);
	if (msg->content.entry->key == NULL)
	{
		free(msg);
		return -1;
	}

	msg->content.entry->value = data_dup(value);
	if (msg->content.entry->value == NULL)
	{
		free(msg);
		return -1;
	}

	msg_2 = network_send_receive(rtables->server, msg);
	if (msg_2 == NULL)
	{
		msg_2 = malloc(sizeof(struct message_t));
		if (msg_2 == NULL)
		{
			free(msg);
			return -1;
		}
		msg_2->opcode = OC_RT_ERROR;
		msg_2->c_type = CT_RESULT;
		msg_2->content.result = -1;
	}

	if (msg_2->opcode != OC_UPDATE + 1 && msg_2->c_type != CT_RESULT)
	{
		msg_2->opcode = OC_RT_ERROR;
		msg_2->c_type = CT_RESULT;
		msg_2->content.result = -1;
	}
	//print_message(msg);
	if (rtables->print == 1)
		print_message(msg_2);
	free(msg);
	free(msg_2);
	return 0;
}

/* Função para obter da tabela remota o valor associado à chave key.
 * Devolve NULL em caso de erro.
 */
struct data_t *rtables_get(struct rtables_t *tables, char *key)
{
	struct data_t *result;
	if (key == NULL)
		return NULL;
	if (tables == NULL)
		return NULL;

	struct message_t *msg_2;
	struct message_t *msg = (struct message_t *)malloc(sizeof(struct message_t));
	if (msg == NULL)
		return NULL;

	msg->opcode = OC_GET;
	msg->c_type = CT_KEY;
	msg->table_num = tables->currentTable;

	msg->content.key = strdup(key);
	if (msg->content.key == NULL)
	{
		free(msg);
		return NULL;
	}

	msg_2 = malloc(sizeof(struct message_t));
	if (msg_2 == NULL)
	{
		free(msg);
		return NULL;
	}

	msg_2 = network_send_receive(tables->server, msg);

	if (msg_2 == NULL)
	{
		if (tables->print == 1)
			print_message(msg_2);
		return NULL;
	}

	if (msg_2->opcode != OC_GET + 1 && msg_2->c_type != CT_RESULT)
	{
		msg_2->opcode = OC_RT_ERROR;
		msg_2->c_type = CT_RESULT;
		msg_2->content.result = -1;
	}
	//print_message(msg);
	if (tables->print == 1)
		print_message(msg_2);

	if (msg_2->content.result == -1)
	{
		free(msg);
		free(msg_2);
		return NULL;
	}
	else
	{
		return msg_2->content.data;
	}
}

/* Devolve número de pares chave/valor na tabela remota.
 */
int rtables_size(struct rtables_t *rtables)
{
	int result;
	if (rtables == NULL)
		return -1;

	struct message_t *msg_2;
	struct message_t *msg = (struct message_t *)malloc(sizeof(struct message_t));
	if (msg == NULL)
		return -1;

	msg->opcode = OC_SIZE;
	msg->c_type = CT_RESULT;
	msg->table_num = rtables->currentTable;
	msg->content.result = 0;

	msg_2 = malloc(sizeof(struct message_t));

	if (msg_2 == NULL)
	{
		free(msg);
		return -1;
	}

	msg_2 = network_send_receive(rtables->server, msg);

	if (msg_2 == NULL)
	{
		msg_2->opcode = OC_RT_ERROR;
		msg_2->c_type = CT_RESULT;
		msg_2->content.result = -1;
	}

	if (msg_2->opcode != OC_SIZE + 1 && msg_2->c_type != CT_RESULT)
	{
		msg_2->opcode = OC_RT_ERROR;
		msg_2->c_type = CT_RESULT;
		msg_2->content.result = -1;
	}
	result = msg_2->content.result;
	//print_message(msg);
	if (rtables->print == 1)
		print_message(msg_2);
	free(msg);
	free(msg_2);
	return result;
}

/* Devolve número de colisões na tabela remota.
 */
int rtables_collisions(struct rtables_t *rtables)
{
	int result;
	if (rtables == NULL)
		return -1;

	struct message_t *msg_2;
	struct message_t *msg = (struct message_t *)malloc(sizeof(struct message_t));
	if (msg == NULL)
		return -1;

	msg->opcode = OC_COLLS;
	msg->c_type = CT_RESULT;
	msg->table_num = rtables->currentTable;
	msg->content.result = 0;

	msg_2 = malloc(sizeof(struct message_t));

	if (msg_2 == NULL)
	{
		free(msg);
		return -1;
	}

	msg_2 = network_send_receive(rtables->server, msg);

	if (msg_2 == NULL)
	{
		msg_2->opcode = OC_RT_ERROR;
		msg_2->c_type = CT_RESULT;
		msg_2->content.result = -1;
	}

	if (msg_2->opcode != OC_COLLS + 1 && msg_2->c_type != CT_RESULT)
	{
		msg_2->opcode = OC_RT_ERROR;
		msg_2->c_type = CT_RESULT;
		msg_2->content.result = -1;
	}
	result = msg_2->content.result;
	//print_message(msg);
	if (rtables->print == 1)
		print_message(msg_2);
	free(msg);
	free(msg_2);
	return result;
}

/* Devolve um array de char * com a cópia de todas as keys da
 * tabela remota, e um último elemento a NULL.
 */
char **rtables_get_keys(struct rtables_t *rtables)
{
	if (rtables == NULL)
		return NULL;

	struct message_t *msg_2;
	struct message_t *msg = (struct message_t *)malloc(sizeof(struct message_t));
	if (msg == NULL)
		return NULL;

	msg->opcode = OC_GET;
	msg->c_type = CT_ENTRY;
	msg->table_num = rtables->server->tableNum;

	msg->content.key = "*";
	if (msg->content.key == NULL)
	{
		free(msg);
		return NULL;
	}

	msg_2 = malloc(sizeof(struct message_t));
	if (msg_2 == NULL)
	{
		free(msg);
		return NULL;
	}

	msg_2 = network_send_receive(rtables->server, msg);

	if (msg_2 == NULL)
	{
		msg_2->opcode = OC_RT_ERROR;
		msg_2->c_type = CT_RESULT;
		msg_2->content.result = -1;
	}

	if (msg_2->opcode != OC_GET + 1 && msg_2->c_type != CT_RESULT)
	{
		msg_2->opcode = OC_RT_ERROR;
		msg_2->c_type = CT_RESULT;
		msg_2->content.result = -1;
	}
	//print_message(msg);
	if (rtables->print == 1)
		print_message(msg_2);
	free(msg);
	free(msg_2);
	return 0;
}

/* Liberta a memória alocada por rtables_get_keys().
 */
void rtables_free_keys(char **keys)
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
