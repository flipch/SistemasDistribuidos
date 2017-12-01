//Grupo 51: Felipe Heliszkowski 47064
//Gonçalo Cardoso 46784
//Pedro Gama 47081

/*
   Programa que implementa um servidor de uma tabela hash com chainning.
   Uso: table-server <port> <table1_size> [<table2_size> ...]
   Exemplo de uso: ./table_server 54321 10 15 20 25
*/
#define NFDESC 5
#define TIMEOUT 50
#include <error.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "inet.h"
#include "table_skel.h"
#include "table-private.h"
#include "message.h"
#include "primary_backup.h"
#include "network_client-private.h"

/*****************************************/
struct thread_parameters
{
	char **argv;
	int argc;
};

/***************************************************************/

int counter = 0, number = 0;

pthread_mutex_t dados = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t dados_disponiveis = PTHREAD_COND_INITIALIZER;

/***********************************************************************************/

void print_message(struct message_t *msg)
{
	int i;

	printf("\n----- MESSAGE -----\n");
	printf("Tabela número: %d\n", msg->table_num);
	printf("opcode: %d, c_type: %d\n", msg->opcode, msg->c_type);
	switch (msg->c_type)
	{
	case CT_ENTRY:
	{
		printf("key: %s\n", msg->content.entry->key);
		printf("datasize: %d\n", msg->content.entry->value->datasize);
	}
	break;
	case CT_KEY:
	{
		printf("key: %s\n", msg->content.key);
	}
	break;
	case CT_KEYS:
	{
		for (i = 0; msg->content.keys[i] != NULL; i++)
		{
			printf("key[%d]: %s\n", i, msg->content.keys[i]);
		}
	}
	break;
	case CT_VALUE:
	{
		if (msg->content.data != NULL)
			printf("datasize: %d\n", msg->content.data->datasize);
	}
	break;
	case CT_RESULT:
	{
		printf("result: %d\n", msg->content.result);
	}
	break;
	case OC_RT_ERROR:
	{
		printf("result: %d\n", msg->content.result);
	};
	}
	printf("-------------------\n");
}

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

	if (listen(socket_fd, 1) < 0)
	{
		perror("Erro ao executar listen");
		close(socket_fd);
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

	if (msg_pedido == NULL)
		return NULL;
	if (tabela == NULL)
		return NULL;

	/* Verificar opcode e c_type na mensagem de pedido */
	if (!(msg_pedido->opcode == OC_SIZE || msg_pedido->opcode == OC_COLLS || msg_pedido->opcode == OC_UPDATE || msg_pedido->opcode == OC_GET || msg_pedido->opcode == OC_PUT) ||
		!(msg_pedido->c_type == CT_RESULT || msg_pedido->c_type == CT_VALUE || msg_pedido->c_type == CT_KEY || msg_pedido->c_type == CT_KEYS || msg_pedido->c_type == CT_ENTRY))
		return NULL;

	int result = 0;
	struct data_t *data = (struct data_t *)malloc(sizeof(struct data_t));
	char **keys;
	int dataOrkeys = 0;

	/* Aplicar operação na tabela */
	if (msg_pedido->opcode == OC_PUT)
	{
		result = table_put(tabela, msg_pedido->content.entry->key, msg_pedido->content.entry->value);
	}
	else if (msg_pedido->opcode == OC_GET)
	{
		if (strcmp(msg_pedido->content.key, "*") != 0) //Caso normal
			data = table_get(tabela, msg_pedido->content.key);
		else
		{ // Pega tudo
			keys = table_get_keys(tabela);
			dataOrkeys = 1;
		}
	}
	else if (msg_pedido->opcode == OC_COLLS)
	{
		result = table_colls(tabela);
	}
	else if (msg_pedido->opcode == OC_UPDATE)
	{
		result = table_update(tabela, msg_pedido->content.entry->key, msg_pedido->content.entry->value);
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
	else if (msg_pedido->opcode == OC_COLLS)
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
/* 
Thread onde corre a lógica do servidor primário
*/
void *thread_primary(void *params)
{
	struct thread_parameters *tp = (struct thread_parameters *)params;
	int *result = 0;
	int listening_socket;
	struct sockaddr_in client;
	socklen_t size_client;
	struct pollfd polls[NFDESC];
	int res, msg_size, nfds;
	char *message_p, *message_r;
	struct message_t *msg_pedido, *msg_resposta;

	if ((listening_socket = make_server_socket(atoi(tp->argv[1]))) < 0)
		return -1;

	/*********************************************************/
	/* Criar as tabelas de acordo com linha de comandos dada */
	/*********************************************************/
	sprintf(tp->argv[1], "%d", argc);
	table_skel_init(tp->argv);

	size_client = sizeof(struct sockaddr_in);

	int i = 0;
	for (i = 0; i < NFDESC; i++)
	{
		polls[i].fd = -1; // ignore < 0
	}
	polls[0].fd = listening_socket;
	polls[0].events = POLLIN;

	nfds = 1;

	while ((res = poll(polls, nfds, TIMEOUT)) >= 0)
	{
		if ((polls[0].revents & POLLIN) && (nfds < NFDESC))
		{
			if ((polls[nfds].fd = accept(polls[0].fd, (struct sockaddr *)&client, &size_client)) > 0)
			{ // Ligação feita
				printf("Cliente{%d} connectado\n", nfds);
				polls[nfds].events = POLLIN;
				nfds++;
			}
		}

		for (i = 1; i < nfds; i++)
		{
			if (polls[i].revents & POLLIN)
			{
				if ((result = read_all(polls[i].fd, (char *)&msg_size, _INT)) == 0)
				{
					printf("O cliente desligou-se\n");
					close(polls[i].fd);
					nfds--;
					polls[i].fd = -1;
					continue;
				}
				else if (result != _INT)
				{
					printf("Erro ao receber dados do cliente");
					close(polls[i].fd);
					polls[i].fd = -1;
					continue;
				}

				msg_size = ntohl(msg_size);
				msg_pedido = (struct message_t *)malloc(msg_size);
				message_p = (char *)malloc(msg_size);

				if ((result = read_all(polls[i].fd, message_p, msg_size)) == 0)
				{
					printf("O cliente desligou-se\n");
					close(polls[i].fd);
					nfds--;
					polls[i].fd = -1;
					continue;
				}
				else if (result != msg_size)
				{
					printf("Erro ao receber dados do cliente");
					close(polls[i].fd);
					polls[i].fd = -1;
					continue;
				}

				else
				{
					msg_pedido = buffer_to_message(message_p, msg_size);

					if (msg_pedido == NULL)
					{
						free_message(msg_pedido);
						free(message_p);
						return -1;
					}
					//printf("Recebido do cliente:");
					//print_message(msg_pedido);
					msg_resposta = invoke(msg_pedido);
					//printf("Enviado para o cliente:");
					//print_message(msg_resposta);

					msg_size = message_to_buffer(msg_resposta, &message_r);

					if (msg_size <= 0)
					{
						free_message(msg_pedido);
						free_message(msg_resposta);
						free(message_p);
						return -1;
					}

					int message_size = msg_size;
					msg_size = htonl(message_size);
					if ((result = write_all(polls[i].fd, (char *)&msg_size, _INT)) != _INT)
					{
						perror("Erro ao receber dados do cliente");
						close(polls[i].fd);
						free_message(msg_pedido);
						free_message(msg_resposta);
						free(message_r);
						free(message_p);
						return -1;
					}

					if ((result = write_all(polls[i].fd, message_r, message_size)) != message_size)
					{
						perror("Erro ao receber dados do cliente");
						close(polls[i].fd);
						free_message(msg_pedido);
						free_message(msg_resposta);
						free(message_p);
						free(message_r);
						return -1;
					}
				}
			}

			if (polls[i].revents & POLLHUP)
			{
				close(polls[i].fd);
				polls[i].fd = -1;
			}
		}
	}

	if (table_skel_destroy() == -1)
		return -1;

	for (i = 0; i < nfds; i++)
		close(polls[i].fd);
	return 0;
}

void *thread_secondary(void *params)
{
	struct thread_parameters *tp = (struct thread_parameters *)params;
	int *result = 0;
	int listening_socket;
	struct sockaddr_in client;
	socklen_t size_client;
	struct pollfd polls[NFDESC];
	int res, msg_size, nfds;
	char *message_p, *message_r;
	struct message_t *msg_pedido, *msg_resposta;
	int sec_port = atoi(tp->argv[1]) + 1; //E.g. 1337 => 1338

	if ((listening_socket = make_server_socket()) < 0)
		return -1;

	/*********************************************************/
	/* Criar as tabelas de acordo com linha de comandos dada */
	/*********************************************************/
	sprintf(tp->argv[1], "%d", argc);
	table_skel_init(tp->argv);

	size_client = sizeof(struct sockaddr_in);

	int i = 0;
	for (i = 0; i < NFDESC; i++)
	{
		polls[i].fd = -1; // ignore < 0
	}
	polls[0].fd = listening_socket;
	polls[0].events = POLLIN;

	nfds = 1;

	while ((res = poll(polls, nfds, TIMEOUT)) >= 0)
	{
		if ((polls[0].revents & POLLIN) && (nfds < NFDESC))
		{
			if ((polls[nfds].fd = accept(polls[0].fd, (struct sockaddr *)&client, &size_client)) > 0)
			{ // Ligação feita
				printf("Cliente{%d} connectado\n", nfds);
				polls[nfds].events = POLLIN;
				nfds++;
			}
		}

		for (i = 1; i < nfds; i++)
		{
			if (polls[i].revents & POLLIN)
			{
				if ((result = read_all(polls[i].fd, (char *)&msg_size, _INT)) == 0)
				{
					printf("O cliente desligou-se\n");
					close(polls[i].fd);
					nfds--;
					polls[i].fd = -1;
					continue;
				}
				else if (result != _INT)
				{
					printf("Erro ao receber dados do cliente");
					close(polls[i].fd);
					polls[i].fd = -1;
					continue;
				}

				msg_size = ntohl(msg_size);
				msg_pedido = (struct message_t *)malloc(msg_size);
				message_p = (char *)malloc(msg_size);

				if ((result = read_all(polls[i].fd, message_p, msg_size)) == 0)
				{
					printf("O cliente desligou-se\n");
					close(polls[i].fd);
					nfds--;
					polls[i].fd = -1;
					continue;
				}
				else if (result != msg_size)
				{
					printf("Erro ao receber dados do cliente");
					close(polls[i].fd);
					polls[i].fd = -1;
					continue;
				}

				else
				{
					msg_pedido = buffer_to_message(message_p, msg_size);

					if (msg_pedido == NULL)
					{
						free_message(msg_pedido);
						free(message_p);
						return -1;
					}
					//printf("Recebido do cliente:");
					//print_message(msg_pedido);
					msg_resposta = invoke(msg_pedido);
					//printf("Enviado para o cliente:");
					//print_message(msg_resposta);

					msg_size = message_to_buffer(msg_resposta, &message_r);

					if (msg_size <= 0)
					{
						free_message(msg_pedido);
						free_message(msg_resposta);
						free(message_p);
						return -1;
					}

					int message_size = msg_size;
					msg_size = htonl(message_size);
					if ((result = write_all(polls[i].fd, (char *)&msg_size, _INT)) != _INT)
					{
						perror("Erro ao receber dados do cliente");
						close(polls[i].fd);
						free_message(msg_pedido);
						free_message(msg_resposta);
						free(message_r);
						free(message_p);
						return -1;
					}

					if ((result = write_all(polls[i].fd, message_r, message_size)) != message_size)
					{
						perror("Erro ao receber dados do cliente");
						close(polls[i].fd);
						free_message(msg_pedido);
						free_message(msg_resposta);
						free(message_p);
						free(message_r);
						return -1;
					}
				}
			}

			if (polls[i].revents & POLLHUP)
			{
				close(polls[i].fd);
				polls[i].fd = -1;
			}
		}
	}

	if (table_skel_destroy() == -1)
		return -1;

	for (i = 0; i < nfds; i++)
		close(polls[i].fd);
	return 0;
}

int main(int argc, char **argv)
{
	int result, *r;
	pthread_t primary_server, secondary_server;
	struct thread_parameters thread_p, thread_s;
	struct server_t **servers;

	if (argc < 3)
	{
		printf("Uso: ./server <porta TCP> <port> <table1_size> [<table2_size> ...]\n");
		printf("Exemplo de uso: ./table-server 54321 10 15 20 25\n");
		return -1;
	}

	/* Parâmetros para o servidor primário */
	thread_p.argv = argv;
	thread_p.argc = argc;

	/* Parâmetros para o servidor secundario */
	thread_s.argv = argv;
	thread_s.argc = argc;

	/* Criação das threads de cada servidor */
	if (pthread_create(&primary_server, NULL, &thread_primary, (void *)&thread_p) != 0)
	{
		perror("\nThread do Servidor Primário não criada.\n");
		exit(EXIT_FAILURE);
	}
	if (pthread_create(&secondary_server, NULL, &thread_secondary, (void *)&thread_s) != 0)
	{
		perror("\nThread do Servidor Secundário não criada.\n");
		exit(EXIT_FAILURE);
	}

	while (1)
	{
		//Optimização
		sleep(5);
		int i;
		/*for (i = 0; i < 2; i++ 0)
		{
			int state = update_state(servers[i]);
			if (state == -1) //Servidor dead
			{
				//Criar novo servidor dependendo do caso
				if (i == 0)
				{
					//Primary server morreu, secundario agora é primário e criar novo secundário
					primary_server = secondary_server;
					//New secondary
					if (pthread_create(&secondary_server, NULL, &thread_secondary, (void *)&thread_s) != 0)
					{
						perror("\nThread do Servidor Secundário não criada.\n");
						exit(EXIT_FAILURE);
					}
				}
			}
		}
		*/
	}

	return 0;
}
