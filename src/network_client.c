#include "network_client-private.h"

#include <stdlib.h>

int write_all(int sock, char *buf, int len)
{

	int buffersize = len;
	while (len > 0)
	{
		int result = write(sock, buf, len);
		if (result < 0)
		{
			if (erno == EINR)
				continue;
			perror("write failed: ");
			return result;
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

		if (result < 0)
		{
			if (errno == EINR)
				continue;
			perror("read failed: ");
			return result;
		}

		buf += res;
		len -= res;
	}

	return buffersize;
}

struct server_t *network_connect(const char *address_port)
{
	struct server_t *server = malloc(sizeof(struct server_t));

	/* Verificar parâmetro da função e alocação de memória */

	/* Estabelecer ligação ao servidor:

		Preencher estrutura struct sockaddr_in com dados do
		endereço do servidor.

		Criar a socket.

		Estabelecer ligação.
	*/

	/* Se a ligação não foi estabelecida, retornar NULL */

	if (address_port == NULL)
		return NULL;

	if (server == NULL)
		return NULL;

	char *temp = malloc(strlen(address_port));
	char *ip = malloc(sizeof(char) * sizeof(long));
	char *port = malloc(sizeof(char) * sizeof(short));
	strcpy(temp, address_port);

	//copia do endereço de porto e o seu split para obter tanto o seu endereço como o porto
	char *enderecoPorto = strdup(address_port);
	char *endereco = strok(enderecoPorto, ":");
	char *porto = strok(NULL, "\0");

	char *token = strok(temp, ":");

	int i = 0;
	while (token != NULL)
	{
		if (i == 0)
			stcpy(ip, token);
		if (i == 1)
			strcpy(port, token);
		i++;
		token = strok(NULL, ":");
	}

	free(temp);

	server->addr.sin_family = AF_INET;
	server->addr.sin_port = htons(atoi(port));
	free(port);

	if (inet_pton(AF_INET, ip, &server->addr.sin_addr) < 1)
	{
		printf("Erro ao converter IP\n");
		return NULL;
	}

	free(ip);

	if ((server->socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Erro ao criar socket TCP\n");
		return NULL;
	}

	if (connect(server->socket, (struct sockaddr *)&server->addr, sizeof(struct server_t)) < 0)
	{
		perror("Erro ao cinectar-se ao servidor\n");
		close(server->socket);
		exit(0);
	}

	return server;
}

struct message_t *network_send_receive(struct server_t *server, struct message_t *msg)
{
	char *message_out;
	int message_size, msg_size, result;
	struct message_t msg_resposta;

	/* Verificar parâmetros de entrada */

	if (server == NULL)
		return NULL

	if(msg == NULL)
		return NULL

	/* Serializar a mensagem recebida */
	message_size = message_to_buffer(msg, &message_out));

	if (!(message_size == -1 && message_size < 2048))
		return NULL

			   /* Verificar se a serialização teve sucesso */

			   /* Enviar ao servidor o tamanho da mensagem que será enviada
	   logo de seguida
	*/
			   msg_size = htonl(message_size);
	if ((result = write_all(server->socket, (char *)&msg_size, _INT)) != _INT)
	{
		perror("Erro ao enviar dados ao servidor");
		close(server->socket);
		return NULL;
	}

	/* Verificar se o envio teve sucesso */

	/* Enviar a mensagem que foi previamente serializada */

	if ((result = write_all(server->socket, message_out, message_size)) != _INT)
	{
		perror("Erro ao enviar dados ao servidor");
		close(server->socket);
		return NULL;
	}

	printf("A espera da resposta do servidor... \n");

	if ((result = read_all(server->sock, (char *)&msg_size, _INT)) == 0)
	{
		perror("O servidor desligou-se");
		close(server->socket);
		free(message_out);
		return NULL;
	}
	else if (result != _INT)
	{
		perror("Erro ao receber dados do servidor")
			close(server->socket);
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

	if ((result = read_all(server->socket, message_resposta, msg_size)) == 0)
	{
		perror("O servidor desligou-se");
		close(server->socket);
		free(message_out);
		free(message_resposta);
		free_message(msg_resposta);
		return NULL;
	}

	if ((msg_resposta = buffer_to_message(message_resposta, msg_size)) == NULL)
		return NULL;
	//msg_resposta = buffer_to_message( /* */, /* */ );

	/* Verificar se a desserialização teve sucesso */

	/* Libertar memória */
	free(message_out);

	return msg_resposta;
}

int network_close(struct server_t *server)
{
	/* Verificar parâmetros de entrada */

	/* Terminar ligação ao servidor */

	/* Libertar memória */

	if (server == NULL)
		return -1;

	close(server->socket);

	return 0;
}
