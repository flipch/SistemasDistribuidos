//Grupo 51: Felipe Heliszkowski 47064
//Gon�alo Cardoso 46784
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

#define PRIMARY		1
#define SECONDARY  	2

/*****************************************/
struct thread_parameters
{
	char **argv;
	int argc;
	int type;
	int id;
};

typedef struct ServerController
{
	int server_id;
	int state;
	struct server_t server;
} ServerController;

ServerController **sc;
int unique_id = 1;

/***************************************************************/
int idPrimary, idSecondary;

pthread_mutex_t dados = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t dados_disponiveis = PTHREAD_COND_INITIALIZER;

/***********************************************************************************/

void print_message(struct message_t *msg)
{
	int i;

	printf("\n----- MESSAGE -----\n");
	printf("Tabela n�mero: %d\n", msg->table_num);
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

/* Fun��o para preparar uma socket de rece��o de pedidos de liga��o.
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

/* Fun��o que recebe uma tabela e uma mensagem de pedido e:
	- aplica a opera��o na mensagem de pedido na tabela;
	- devolve uma mensagem de resposta com oresultado.
*/
struct message_t *process_message(struct message_t *msg_pedido, struct table_t *tabela)
{
	struct message_t *msg_resposta;

	/* Verificar par�metros de entrada */

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

	/* Aplicar opera��o na tabela */
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

/* Fun��o "inversa" da fun��o network_send_receive usada no table-client.
   Neste caso a fun��o implementa um ciclo receive/send:

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

	/* Verificar par�metros de entrada */
	if (sockfd < 0)
	{
		return -1;
	}

	if (tables == NULL)
	{
		free(tables);
		return -1;
	}
	/* Com a fun��o read_all, receber num inteiro o tamanho da 
	   mensagem de pedido que ser� recebida de seguida.*/
	/* Verificar se a rece��o teve sucesso */
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

	/* Alocar mem�ria para receber o n�mero de bytes da
	   mensagem de pedido. */
	msg_size = ntohl(msg_size);
	msg_pedido = (struct message_t *)malloc(msg_size);
	message_p = (char *)malloc(msg_size);

	/* Com a fun��o read_all, receber a mensagem de resposta. */
	/* Verificar se a rece��o teve sucesso */
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

	/* Verificar se a desserializa��o teve sucesso */
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

	/* Verificar se a serializa��o teve sucesso */
	if (message_size <= 0) // Condicao hmmmm
	{
		free_message(msg_pedido);
		free_message(msg_resposta);
		free(message_p);
		return -1;
	}
	/* Enviar ao cliente o tamanho da mensagem que ser� enviada
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
	/* Libertar mem�ria */
	free_message(msg_pedido);
	free_message(msg_resposta);
	free(message_p);
	free(message_r);

	return 0;
}
/*
	Procura por um id no controlador de servers
*/
int indexOf(int id)
{
	int count = 0;
	ServerController *serverC = sc[0];
	while (serverC != NULL)
	{
		if (serverC->server_id == id)
		{
			//Found
			return count;
		}
		count++;
	}
	return -1;
}

/* 
Thread onde corre a l�gica do servidor prim�rio
*/
void *thread_server(void *params)
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

	if (tp->type == 1)
	{
		if ((listening_socket = make_server_socket(atoi(tp->argv[1]))) < 0)
			return -1;
	}
	else if (tp->type == 2)
	{
		int porta = atoi(tp->argv[1]) + 1;
		if ((listening_socket = make_server_socket(porta) < 0))
			return -1;
	}

	/*********************************************************/
	/* Criar as tabelas de acordo com linha de comandos dada */
	/*********************************************************/
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

	int indexServer = indexOf(tp->id);
	if (indexServer == -1)
	{
		return -1;
	}
	sc[indexServer]->state = 1;
	sc[indexServer]->server.addr = client;
	sc[indexServer]->server.socket = listening_socket;
	while ((res = poll(polls, nfds, TIMEOUT)) >= 0)
	{ //Hearthbeat
		sc[indexServer]->state = 1;

		if ((polls[0].revents & POLLIN) && (nfds < NFDESC))
		{
			if ((polls[nfds].fd = accept(polls[0].fd, (struct sockaddr *)&client, &size_client)) > 0)
			{ // Liga��o feita
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

					if (msg_pedido->opcode == OC_PUT || msg_pedido->opcode == OC_UPDATE) //Redundancia
					{
						int backup = -1;
						//backup = backupToServer(msg_pedido);
						if (backup == -1)
						{
							//Marcar o secundario como down
						}
					}

					msg_resposta = invoke(msg_pedido);

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
	struct thread_parameters thread_params;

	if (argc < 3)
	{
		printf("Uso: ./server <porta TCP> <port> <table1_size> [<table2_size> ...]\n");
		printf("Exemplo de uso: ./table-server 54321 10 15 20 25\n");
		return -1;
	}

	thread_params.argv = argv;
	thread_params.argc = argc;
	thread_params.type = 1;
	thread_params.id = unique_id;

	sc = malloc( sizeof(ServerController*) * 20);

	sc[0]->server_id = unique_id;
	unique_id++;
	sc[0]->state = 0;

	if (pthread_create(&primary_server, NULL, &thread_server, (void *)&thread_params) != 0)
	{
		perror("\nThread do Servidor Prim�rio n�o criada.\n");
		exit(EXIT_FAILURE);
	}
	/* Par�metros para o servidor secundario */
	thread_params.argv = argv;
	thread_params.argc = argc;
	thread_params.type = 2;
	thread_params.id = unique_id;

	sc[1]->server_id = unique_id;
	unique_id++;
	sc[1]->state = 0;

	if (pthread_create(&secondary_server, NULL, &thread_server, (void *)&thread_params) != 0)
	{
		perror("\nThread do Servidor Secund�rio n�o criada.\n");
		exit(EXIT_FAILURE);
	}
	clock_t t_inicio;
	int startCounting = 0;
	double time_taken = -1; // in seconds
	while (1)
	{
		//Optimiza��o
		sleep(1);

		/*
			Procura o hearthbeat e trata da logica entre controlador-servidor primario
		*/ 
		int indexPrimary = indexOf(idPrimary);
		if (indexPrimary == -1)
		{
			printf("Index do servidor primario nao foi encontrado\n");
		}
		else
		{
			if (sc[indexPrimary]->state == 1)
			{
				sc[indexPrimary]->state = 0;
			}
			else if (sc[indexPrimary]->state == 0)
			{
				if (startCounting == 0)
				{
					t_inicio = clock();
					startCounting = 1;
				}
				if (startCounting == 1)
				{
					clock_t t_atual = clock() - t_inicio;
					time_taken = ((double)t_atual) / CLOCKS_PER_SEC;
				}
				if (time_taken > 5)
				{
					startCounting = 0;
					//criar novo
					sc[1]->server.type = SECONDARY;
				}
			}
		}
		
		/*
			Procura o hearthbeat e trata da logica entre controlador-servidor secundario
		*/ /*
		int indexSecondary = indexOf(idSecondary);
		if (indexSecondary == -1)
		{
			printf("Index do servidor primario nao foi encontrado\n");
		}
		else
		{
			if (sc[indexSecondary]->state == 1)
			{
				sc[indexSecondary]->state = 0;
			}
			else if (sc[indexSecondary]->state == 0)
			{
				if (startCounting == 0)
				{
					t_inicio = clock();
					startCounting = 1;
				}
				if (startCounting == 1)
				{
					clock_t t_atual = clock() - t_inicio;
					time_taken = ((double)t_atual) / CLOCKS_PER_SEC;
				}
				if (time_taken > 5)
				{
					startCounting = 0;
					//criar novo
				}
			}
		}*/
	}

	return 0;
}
