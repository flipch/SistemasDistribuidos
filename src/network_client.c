//Grupo 51: Felipe Heliszkowski 47064
//Gonçalo Cardoso 46784
//Pedro Gama 47081

#include "network_client-private.h"
#include "inet.h"
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <message.h>

int write_all(int sock, char *buf, int len)
{

	int buffersize = len;
	while (len > 0)
	{
		int res = write(sock, buf, len);
		if (res < 0)
		{
			if (errno == EINTR)
				continue;
			perror("write failed: ");
			return res;
		}
		buf += res;
		len -= res;
	}

	return buffersize;
}

int read_all(int sock, char *buf, int len)
{

	int buffersize = len;
	while (len > 0)
	{
		int result = read(sock, buf, len);

		if (result <= 0)
		{
			if (errno == EINTR)
				continue;
			perror("read failed: ");
			return result;
		}

		buf += result;
		len -= result;
	}

	return buffersize;
}

struct server_t *network_connect(const char *address_port)
{
	/* Verificar parâmetro da função e alocação de memória */
	if (address_port == NULL)
		return NULL;

	char *cAddress_port = strdup(address_port);

	char *separator = strchr(cAddress_port, ':');
	*separator = '\0';

	char *address = cAddress_port;
	char *port = strdup(separator + 1);

	struct server_t *server = malloc(sizeof(struct server_t));
	if (server == NULL)
	{
		free(cAddress_port);
		free(port);
		return NULL;
	}

	// Cria socket TCP
	if ((server->socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Erro ao criar socket");
		free(server);
		free(cAddress_port);
		free(port);
		return NULL;
	}
	// Preenche estrutura server para estabelecer conexão
	server->addr.sin_family = AF_INET;
	server->addr.sin_port = htons(atoi(port));
	if (inet_pton(AF_INET, address, &server->addr.sin_addr) < 1)
	{
		printf("Erro ao converter IP\n");
		free(cAddress_port);
		free(port);
		close(server->socket);
		free(server);
		return NULL;
	}
	// Estabelece conexão com o servidor definido em server
	if (connect(server->socket, (struct sockaddr *)&server->addr, sizeof(server->addr)) < 0)
	{
		perror("Erro ao conectar-se ao servidor");
		free(cAddress_port);
		free(port);
		close(server->socket);
		free(server);
		return NULL;
	}
	free(port);
	free(cAddress_port);
	return server;
}

struct message_t *network_send_receive(struct server_t *server, struct message_t *msg)
{
	char *message_out;
	int message_size, msg_size, result;
	struct message_t *msg_resposta;

	/* Verificar parâmetros de entrada */

	if (server == NULL)
		return NULL;

	if (msg == NULL)
		return NULL;

	/* Serializar a mensagem recebida */
	message_size = message_to_buffer(msg, &message_out);

	/* Verificar se a serialização teve sucesso */
	//TO-DO

	/* Enviar ao servidor o tamanho da mensagem que será enviada
	   logo de seguida
	*/
	msg_size = htonl(message_size);
	if ((result = write_all(server->socket, (char *)&msg_size, _INT)) != _INT)
	{
		perror("Erro ao enviar dados ao servidor");
		network_close(server);
		return NULL;
	}

	/* Verificar se o envio teve sucesso */

	/* Enviar a mensagem que foi previamente serializada */

	write_all(server->socket, message_out, message_size);	

	//printf("A  da resposta do servidor... \n");

	if ((result = read_all(server->socket, (char *)&msg_size, _INT)) == 0)
	{
		perror("O servidor desligou-se");
		network_close(server);
		free(message_out);
		return NULL;
	}
	else if (result != _INT)
	{
		perror("Erro ao receber dados do servidor");
		network_close(server);
		free(message_out);
		return NULL;
	}

	/* Verificar se o envio teve sucesso */

	/* De seguida vamos receber a resposta do servidor:

		Com a função read_all, receber num inteiro o tamanho da 
		mensagem de resposta.

		Alocar memória para receber o número de bytes da
		mensagem de resposta.

		Com a função read_all, receber a mensagem de resposta.
		
	*/

	message_size = ntohl(msg_size);
	/* Desserializar a mensagem de resposta */
	msg_resposta = (struct message_t *)malloc(msg_size);
	char *message_resposta = malloc(sizeof(char *));

	result = read_all(server->socket, message_resposta, message_size);
	
	msg_resposta = buffer_to_message(message_resposta, message_size);
	/* Verificar se a desserialização teve sucesso */

	/* Libertar memória */
	free(message_out);
	free(message_resposta);

	return msg_resposta;
}

int network_close(struct server_t *server)
{
	/* Verificar parâmetros de entrada */
	int retorna = 0;
	if (server == NULL)
	{
		free(server);
		return retorna;
	}
	/* Terminar ligação ao servidor */
	if (close(server->socket) < 0)
	{
		retorna = 1;
	}

	/* Libertar memória */
	free(server);
	return retorna;
}
