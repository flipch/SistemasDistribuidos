/*
   Programa que implementa um servidor de uma tabela hash com chainning.
   Uso: table-server <port> <table1_size> [<table2_size> ...]
   Exemplo de uso: ./table_server 54321 10 15 20 25
*/
#include <error.h>

#include "inet.h"
#include "table-private.h"
#include "message-private.h"

/* Função para preparar uma socket de receção de pedidos de ligação.
*/
int make_server_socket(short port)
{
	int socket_fd;
	struct sockaddr_in server;

	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Erro ao criar socket");
		return -1;
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(socket_fd, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("Erro ao fazer bind");
		close(socket_fd);
		return -1;
	}

	if (listen(socket_fd, 0) < 0)
	{
		perror("Erro ao executar listen");
		close(sfd);
		return -1;
	}
	return socket_fd;
}

/* Função que recebe uma tabela e uma mensagem de pedido e:
	- aplica a operação na mensagem de pedido na tabela;
	- devolve uma mensagem de resposta com oresultado.
*/
struct message_t *process_message(struct message_t *msg_pedido, struct table_t *tabela)
{
	struct message_t *msg_resposta;

	/* Verificar parâmetros de entrada */

	if (msg_pedido == NULL || tabela == NULL)
		return NULL;

	/* Verificar opcode e c_type na mensagem de pedido */
	if (!(msg_pedido->opcode == OC_SIZE || msg_pedido->opcode == OC_DEL || msg_pedido->opcode == OC_UPDATE || msg_pedido->opcode == OC_GET || msg_pedido->opcode == OC_PUT) ||
		!(msg_pedido->c_type == CT_RESULT || msg_pedido->c_type == CT_VALUE || msg_pedido->c_type == CT_KEY || msg_pedido->c_type == CT_KEYS || msg_pedido->c_type == CT_ENTRY))
		return NULL;

	int result;
	struct data_t data;
	char **keys;
	int dataOrkeys = 0;

	/* Aplicar operação na tabela */
	if (msg_pedido->opcode == OC_PUT)
	{
		if (table_get(tabela, msg_pedido->content.entry->key) == NULL)
			result = table_put(tabela, msg_pedido->content.entry->key, msg_pedido->content.entry->value);
		else
			result = -1;
	}
	else if (msg_pedido->opcode == OC_GET)
	{
		if (strcmp(msg_pedido->content.key, "*") != 0)
			data = table_get(tabela, msg_pedido->content.key);
		else
		{
			keys = table_get_keys(tabela);
			dataOrkeys = 1;
		}
	}
	else if (msg_pedido->opcode == OC_DEL)
	{
		result = table_del(tabela, msg_pedido->content.key)
	}
	else if (msg_pedido->opcode == OC_UPDATE)
	{
		if (table_get(tabela, msg_pedido->content.key) != NULL)
			result = table_update(tabela, msg_pedido->content.entry->key, msg_pedido->content.entry->value);
		else
			result = -1;
	}
	else if (msg_pedido->opcode == OC_SIZE)
	{
		result = table_size(tabela);
	}

	msg_resposta = (struct message_t *)malloc(sizeof(struct message_t *));

	if (result == -1)
	{
		msg_resposta->opcode = OC_RT_ERROR;
		msg_resposta->c_type = CT_RESULT;
		msg_resposta->content.result = result;
		return msg_resposta;
	}

	/* Preparar mensagem de resposta */

	msg_resposta->opcode = msg_pedido->opcode + 1;
	if (msg_pedido->opcode == OC_PUT)
	{
		msg_resposta->c_type = CT_RESULT;
		msg_resposta->content.result = result;
	}
	else if (msg_pedido->opcode == OC_GET)
	{
		if (dataOrkeys == 0)
		{
			if (data == NULL)
			{
				struct data_t *not_found = (struct data_t *)malloc(sizeof(struct data_t));
				not_found->data = NULL;
				not_found->datasize = 0;
				msg_resposta->c_type = CT_VALUE;
				msg_resposta->content.data = not_found;
			}
			else
			{
				msg_resposta->c_type = CT_VALUE;
				msg_resposta->content.data = data_create2(data->datasize, data);
			}
		}
		else
		{
			msg_resposta->c_type = CT_KEYS;
			msg_resposta->content.keys = keys;
		}
	}
	else if (msg_pedido->opcode == OC_DEL)
	{
		msg_resposta->c_type = CT_RESULT;
		msg_resposta->content.result = result;
	}
	else if (msg_pedido->opcode == OC_UPDATE)
	{
		msg_resposta->c_type = CT_RESULT;
		msg_resposta->content.result = result;
	}
	else if (msg_pedido->opcode == OC_SIZE)
	{
		msg_resposta->c_type = CT_RESULT;
		msg_resposta->content.result = result;
	}

	return msg_resposta;
}

/* Função "inversa" da função network_send_receive usada no table-client.
   Neste caso a função implementa um ciclo receive/send:

	Recebe um pedido;
	Aplica o pedido na tabela;
	Envia a resposta.
*/
int network_receive_send(int sockfd, struct table_t *tables)
{
	char *message_resposta, *message_pedido;
	int msg_length;
	int message_size, msg_size, result;
	struct message_t *msg_pedido, *msg_resposta;

	if (sockfd < 0)
		return NULL;

	if (tables == NULL)
	{
		free(tables);
		return NULL;
	}

	/* Verificar parâmetros de entrada */

	/* Com a função read_all, receber num inteiro o tamanho da 
	   mensagem de pedido que será recebida de seguida.*/
	result = read_all(sockfd, (char *)&msg_size, _INT);

	/* Verificar se a receção teve sucesso */

	/* Alocar memória para receber o número de bytes da
	   mensagem de pedido. */

	/* Com a função read_all, receber a mensagem de resposta. */
	result = read_all(sockfd, message_pedido, /* tamanho da mensagem */);

	/* Verificar se a receção teve sucesso */

	/* Desserializar a mensagem do pedido */
	msg_pedido = buffer_to_message(message_pedido, /* tamanho da mensagem */);

	/* Verificar se a desserialização teve sucesso */

	/* Processar a mensagem */
	msg_resposta = process_message(msg_pedido, /* tabela do pedido */);

	/* Serializar a mensagem recebida */
	message_size = message_to_buffer(msg_resposta, &message_resposta);

	/* Verificar se a serialização teve sucesso */

	/* Enviar ao cliente o tamanho da mensagem que será enviada
	   logo de seguida
	*/
	msg_size = htonl(message_size);
	result = write_all(server->/*atributo*/, (char *) &msg_size, _INT));

	/* Verificar se o envio teve sucesso */

	/* Enviar a mensagem que foi previamente serializada */

	result = write_all(server->/*atributo*/, message_resposta, message_size));

	/* Verificar se o envio teve sucesso */

	/* Libertar memória */

	return 0;
}

int main(int argc, char **argv)
{
	int listening_socket, connsock, result;
	struct sockaddr_in client;
	socklen_t size_client;
	struct table_t *tables;

	if (argc < 3)
	{
		printf("Uso: ./server <porta TCP> <port> <table1_size> [<table2_size> ...]\n");
		printf("Exemplo de uso: ./table-server 54321 10 15 20 25\n");
		return -1;
	}

	if ((listening_socket = make_server(atoi(argv[1]))) < 0)
		return -1;

	/*********************************************************/
	/* Criar as tabelas de acordo com linha de comandos dada */
	/*********************************************************/

	while ((connsock = accept(listening_socket, (struct sockaddr *)&client, &size_client)) != -1)
	{
		printf(" * Client is connected!\n");

		while (/* condição */)
		{

			/* Fazer ciclo de pedido e resposta */
			network_receive_send(connsock, tables) < 0);

			/* Ciclo feito com sucesso ? Houve erro?
			   Cliente desligou? */
		}
	}
}
