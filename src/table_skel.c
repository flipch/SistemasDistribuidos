#include "table_skel.h"

struct table_t *table;
struct table_t *tables;
/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_tables define o número e dimensão das
 * tabelas a serem mantidas no servidor.
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */

int table_skel_init(char **n_tables)
{
	int count = atoi(n_tables[1]); // Na realidade tem mais 2 do que o numero de tabelas supostas

	int i = 2, index = -1;
	tables = (struct table_t *)malloc(sizeof(struct table_t) * count);
	table = (struct table_t *)malloc(sizeof(struct table_t));
	do
	{
		int size = atoi(n_tables[i]);
		table = table_create(size);
		if (table == NULL)
		{
			free(tables);
			free(table);
			return -1;
		}
		tables[++index] = *table;
		i++;
	} while (n_tables[i] != NULL);

	return 0;
}

/* Libertar toda a memória e recursos alocados pela função anterior.
 */
int table_skel_destroy()
{

	if (tables == NULL)
		return -1;
	free(tables);

	if (tables != NULL)
		return -1;

	if (table == NULL)
		return -1;
	free(tables);

	if (table != NULL)
		return -1;

	return 0;
}

/* Executa uma operação numa tabela (indicada pelo opcode e table_num
 * na msg_in) e retorna o resultado numa mensagem de resposta ou NULL
 * em caso de erro.
 */
struct message_t *invoke(struct message_t *msg_in)
{

	struct message_t *msg_resposta;

	if (msg_in == NULL)
	{
		return NULL;
	}

	if (msg_in->opcode < 10 || msg_in->c_type < 10 || msg_in->opcode > 99 || msg_in->opcode > 50)
	{
		return NULL;
	}

	msg_resposta = (struct message_t *)malloc(sizeof(struct message_t));
	if (msg_resposta == NULL)
	{
		free(msg_resposta);
		return NULL;
	}

	int r;
	if (msg_in->opcode == OC_SIZE)
	{
		msg_resposta->opcode = OC_SIZE + 1;
		msg_resposta->c_type = CT_RESULT;
		msg_resposta->table_num = msg_in->table_num;
		msg_resposta->content.result = table_size(&tables[msg_in->table_num]);
		return msg_resposta;
	}
	else if (msg_in->opcode == OC_UPDATE)
	{

		msg_resposta->opcode = OC_UPDATE + 1;
		msg_resposta->c_type = CT_RESULT;
		msg_resposta->content.result = table_update(&tables[msg_in->table_num], msg_in->content.entry->key, msg_in->content.entry->value);
		return msg_resposta;
	}
	else if (msg_in->opcode == OC_GET)
	{

		msg_resposta->opcode = OC_GET + 1;
		if (strcmp("*", msg_in->content.key) == 0)
		{
			msg_resposta->table_num = msg_in->table_num;
			msg_resposta->c_type = CT_KEYS;
			msg_resposta->content.keys = table_get_keys(&tables[msg_in->table_num]);
			return msg_resposta;
		}
		msg_resposta->c_type = CT_VALUE;
		msg_resposta->table_num = msg_in->table_num;
		msg_resposta->content.data = table_get(&tables[msg_in->table_num], msg_in->content.key);
		if (msg_resposta->content.data == NULL)
		{
			msg_resposta->content.data = data_create(0); //Cria um data vazio, tamanho 0
		}
		return msg_resposta;
	}
	else if (msg_in->opcode == OC_PUT)
	{

		msg_resposta->opcode = OC_PUT + 1;
		msg_resposta->c_type = CT_RESULT;
		r = table_put(&tables[msg_in->table_num], msg_in->content.entry->key, msg_in->content.entry->value);
		msg_resposta->content.result = r;
		return msg_resposta;
	}
	else if (msg_in->opcode == OC_COLLS)
	{

		msg_resposta->opcode = OC_COLLS + 1;
		msg_resposta->c_type = CT_RESULT;
		msg_resposta->content.result = table_colls(&tables[msg_in->table_num]);

		return msg_resposta;
	}
	free(msg_resposta);
	return NULL; // se falhar todos retorna NULL RIGHT???
}
