#ifndef _CLIENT_STUB_H
#define _CLIENT_STUB_H

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
struct rtables_t *rtables_bind(const char *address_port){
	struct rtable_t *res = (struct rtable_t*) malloc (sizeof(struct rtable_t));
	if(res == NULL){
		return NULL;
	}
	res->server = network_connect(address_port);
	if(res->server == NULL)
		return NULL;

	return res;
}

/* Termina a associação entre o cliente e um conjunto de tabelas remotas, e
 * liberta toda a memória local. 
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtables_unbind(struct rtables_t *rtables){

	if((network_close(rtables->server))== 0){
		free(rtables);
		return 0;
	}
	free(rtables);
	return -1;	
}

/* Função para adicionar um par chave valor numa tabela remota.
 * Devolve 0 (ok) ou -1 (problemas).
 */
int rtables_put(struct rtables_t *rtables, char *key, struct data_t *value){

	struct message_t *msg = (struct message_t*) malloc(sizeof(struct message_t));
	if(msg == NULL) return -1;
	msg->opcode = OC_PUT;
	msg->c_type = CT_ENTRY;
	msg->table_num = tableNum;
	if((msg->content.entry = entry_create(key,value)) == NULL){
		free(msg);
		return -1;
	}
	struct message_t *msg_resposta = (struct message_t*) malloc(sizeof(struct message_t));
	if(msg_resposta == NULL){
		free_message(msg);
		return -1;
	}
	if((msg_resposta = network_send_receive(rtable -> server, msg)) == NULL){
		free_message(msg);
		free(msg_resposta);		 
		return -1;
	}
	free_message(msg_resposta);
	free_message(msg);
				
	return 0;
}

/* Função para substituir na tabela remota, o valor associado à chave key.
 * Devolve 0 (OK) ou -1 em caso de erros.
 */
int rables_update(struct rtables_t *rtables, char *key, struct data_t *value){
	
}

/* Função para obter da tabela remota o valor associado à chave key.
 * Devolve NULL em caso de erro.
 */
struct data_t *rtables_get(struct rtables_t *tables, char *key){
	
}

/* Devolve número de pares chave/valor na tabela remota.
 */
int rtables_size(struct rtables_t *rtables){
	
}

/* Devolve um array de char * com a cópia de todas as keys da
 * tabela remota, e um último elemento a NULL.
 */
char **rtables_get_keys(struct rtables_t *rtables){
	
}

/* Liberta a memória alocada por rtables_get_keys().
 */
void rtables_free_keys(char **keys){
	
}

#endif