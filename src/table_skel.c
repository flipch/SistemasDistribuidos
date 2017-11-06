#ifndef _TABLE_SKEL_H
#define _TABLE_SKEL_H

#include "table_skel.h"
struct table_t *table;
struct table_t *tables;
/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_tables define o número e dimensão das
 * tabelas a serem mantidas no servidor.
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */

int table_skel_init(char **n_tables){
	int count = 0;

	while(n_tables[count] != NULL){
		count ++;
	}

	int i;				
	tables = (struct table_t *)malloc(sizeof(struct table_t) * count); 
	table = (struct table_t *)malloc(sizeof(struct table_t));
	for (i = 0; i < count; i++)
	{
		int size = atoi(n_tables[i]); 
		table = table_create(size);
		if (table == NULL) 
		{
			result = close(listening_socket);
			free(tables);
			free(table);
			return -1;
		}
		tables[i] = *table;
	}
return 0;
}

/* Libertar toda a memória e recursos alocados pela função anterior.
 */
int table_skel_destroy(){
	
	if(tables == NULL)
		return -1;
	free(tables);

	if(tables != NULL)
		return -1;	

	if(table == NULL)
		return -1;
	free(tables);

	if(table != NULL)
		return -1;

return 0;
}

/* Executa uma operação numa tabela (indicada pelo opcode e table_num
 * na msg_in) e retorna o resultado numa mensagem de resposta ou NULL
 * em caso de erro.
 */
struct message_t *invoke(struct message_t *msg_in){
		char *message_r, *message_p;
	//int msg_length;
	int message_size, msg_size, result;
	struct message_t *msg_pedido, *msg_resposta;

	/* Verificar parâmetros de entrada */
	if (sockfd < 0)
	{
		return -1;
	}

	if (tables == NULL)
	{
		free(tables);
		return -1;
	}
	/* Com a função read_all, receber num inteiro o tamanho da 
	   mensagem de pedido que será recebida de seguida.*/
	/* Verificar se a receção teve sucesso */
	if ((result = read_all(sockfd, (char *)&msg_size, _INT)) == 0)
	{
		perror("O cliente desligou-se");
		close(sockfd);
		return 0;
	}
	else if (result != _INT)
	{
		perror("Erro ao receber dados do cliente");
		close(sockfd);
		return -1;
	}

	/* Alocar memória para receber o número de bytes da
	   mensagem de pedido. */
	msg_size = ntohl(msg_size);
	msg_pedido = (struct message_t *)malloc(msg_size);
	message_p = (char *)malloc(msg_size);

	/* Com a função read_all, receber a mensagem de resposta. */
	/* Verificar se a receção teve sucesso */
	if ((result = read_all(sockfd, message_p, msg_size)) == 0)
	{
		perror("O cliente desligou-se");
		close(sockfd);
		free_message(msg_pedido);
		free(message_p);
		return 0;
	}
	else if (result != msg_size)
	{
		perror("Erro ao receber dados do cliente");
		close(sockfd);
		free_message(msg_pedido);
		free(message_p);
		return -1;
	}

	/* Desserializar a mensagem do pedido */
	msg_pedido = buffer_to_message(message_p, msg_size);

	/* Verificar se a desserialização teve sucesso */
	if (msg_pedido == NULL)
	{
		free_message(msg_pedido);
		free(message_p);
		return -1;
	}
	/* Processar a mensagem */
	msg_resposta = process_message(msg_pedido, &tables[msg_pedido->table_num]);

	/* Serializar a mensagem recebida */
	message_size = message_to_buffer(msg_resposta, &message_r);

	/* Verificar se a serialização teve sucesso */
	if (message_size <= 0) // Condicao hmmmm
	{
		free_message(msg_pedido);
		free_message(msg_resposta);
		free(message_p);
		return -1;
	}
	/* Enviar ao cliente o tamanho da mensagem que será enviada
	   logo de seguida
	*/
	/* Verificar se o envio teve sucesso */
	msg_size = htonl(message_size);
	if ((result = write_all(sockfd, (char *)&msg_size, _INT)) != _INT)
	{
		perror("Erro ao receber dados do cliente");
		close(sockfd);
		free_message(msg_pedido);
		free_message(msg_resposta);
		free(message_r);
		free(message_p);
		return -1;
	}

	/* Enviar a mensagem que foi previamente serializada */

	result = write_all(sockfd, message_r, message_size);

	/* Verificar se o envio teve sucesso */
	if (result != message_size)
	{
		perror("Erro ao receber dados do cliente");
		close(sockfd);
		free_message(msg_pedido);
		free_message(msg_resposta);
		free(message_p);
		free(message_r);
		return -1;
	}
	/* Libertar memória */
	free_message(msg_pedido);
	free_message(msg_resposta);
	free(message_p);
	free(message_r);

	return 0;
}

#endif
