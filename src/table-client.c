/*
	Programa cliente para manipular tabela de hash remota.
	Os comandos introduzido no programa não deverão exceder
	80 carateres.

	Uso: table-client <ip servidor>:<porta servidor>
	Exemplo de uso: ./table_client 10.101.148.144:54321
*/

#include "network_client-private.h"
#include "message.h"

void print_message(struct message_t *msg)
{
    int count;

    printf("##### Message #####\n");
    printf("opcode: %d, c_type:%d\n", msg->opcode, msg->c_type);
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
        for (count = 0; msg->content.keys[count] != NULL; count++)
        {
            printf("key[%d]: %s\n", count, msg->content.keys[count]);
        }
    }
    break;
    case CT_VALUE:
    {
        printf("datasize: %d\n", msg->content.data->datasize);
    }
    break;
    case CT_RESULT:
    {
        printf("result: %d\n", msg->content.result);
    };
    }
    printf("####################\n");
}

int main(int argc, char **argv)
{
    struct server_t *server;
    char input[81];
    struct message_t *msg_out, *msg_resposta;

    /* Testar os argumentos de entrada */
    if (argc < 2)
    {
        printf("Escreva a porta de conexão\n");
        return -1;
    }

    /* Usar network_connect para estabelcer ligação ao servidor */
    server = network_connect(argv[1]);

    char *token = (char *)malloc(80);
    /* Fazer ciclo até que o utilizador resolva fazer "quit" */
    while (strcmp(token, "quit") != 0)
    {
        printf(">>> "); // Mostrar a prompt para inserção de comando

        /* Receber o comando introduzido pelo utilizador
		   Sugestão: usar fgets de stdio.h
		   Quando pressionamos enter para finalizar a entrada no
		   comando fgets, o carater \n é incluido antes do \0.
		   Convém retirar o \n substituindo-o por \0.
		*/

        fgets(input, 80, stdin);
        if (input[strlen(input) - 1] == '\n')
            input[strlen(input) - 1] = '\0';
        if (strcmp(input, "")
        {
            printf("Escreva um comando no terminal");
            exit(0);
        }

        /* Verificar se o comando foi "quit". Em caso afirmativo
		   não há mais nada a fazer a não ser terminar decentemente.
		*/

        token = strtok(input, " ");
        if (strcmp(token, "quit") == 0)
        {
            network_close(server);
            exit(0);
        }
        /* Caso contrário:

			Verificar qual o comando;

			Preparar msg_out;

			Usar network_send_receive para enviar msg_out para
			o server e receber msg_resposta.
		*/

        else
        {
            int count = 0;
            msg_out = (struct message_t *)malloc(sizeof(struct message_t));
            int fail = 0; //BOOLEAN VALUE FOR CHECK

            if (strcmp(token, "put") == 0)
            {
                token = strtok(NULL, " ");
                char *key;
                struct data_t *data;
                while (token != NULL)
                {
                    if (count == 0)
                    {
                        key = malloc(strlen(token));
                        strcpy(key, token);
                        strcat(key, "");
                    }
                    else if (count == 1)
                        data = data_create2(sizeof(token), token);
                    count++;
                    token = strtok(NULL, " ");
                }
                msg_out->opcode = OC_PUT;
                msg_out->c_type = CT_ENTRY;
                struct entry_t *entry = (struct entry_t *)malloc(sizeof(struct entry_t));
                if (entry == NULL)
                { //Malloc failed?
                    free(entry);
                    return -1;
                }
                entry->key = key;
                entry->value = data;
                msg_out->content.entry = entry;
            }
            else if (strcmp(token, "get") == 0)
            {
                token = strtok(NULL, " ");
                msg_out->content.key = (char *)malloc(strlen(token));
                strcpy(msg_out->content.key, token);
                msg_out->opcode = OC_GET;
                msg_out->c_type = CT_KEY;
            }
            else if (strcmp(token, "update") == 0)
            {
                char *key;
                struct data_t *data;
                token = strtok(NULL, " ");
                while (token != NULL)
                {
                    if (count == 0)
                    {
                        key = malloc(strlen(token));
                        strcpy(key, token);
                    }
                    else if (count == 1)
                        data = data_create2(sizeof(token), token);
                    count++;
                    token = strtok(NULL, " ");
                }
                msg_out->opcode = OC_UPDATE;
                msg_out->c_type = CT_ENTRY;
                struct entry_t *entry = (struct entry_t *)malloc(sizeof(struct entry_t));
                if (entry == NULL)
                { //Malloc failed?
                    free(entry);
                    return -1;
                }
                entry->key = key;
                entry->value = data;
                msg_out->content.entry = entry;
            }
            else if (strcmp(token, "cols") == 0)
            {
                msg_out->opcode = OC_COLLS;
                msg_out->c_type = CT_RESULT;
            }
            else if (strcmp(token, "size") == 0)
            {
                msg_out->opcode = OC_SIZE;
                msg_out->c_type = CT_RESULT;
            }
            else
            {
                fail = 1;
                printf("Introduza um command certo\n");
                printf("put <key> <data>\n");
                printf("get <key>\n");
                printf("colls \n");
                printf("update <key> <data>\n");
                printf("size\n");
                printf("quit\n");
            }
            if (fail == 0)
            {
                msg_resposta = (struct message_t *)malloc(sizeof(struct message_t));
                if ((msg_resposta = network_send_receive(server, msg_out)) == NULL)
                {
                    free(msg_resposta);
                    exit(0);
                }
                count = 0;
                print_message(msg_resposta);
                free_message(msg_out);
            }
            token = (char *)malloc(80);
        }
    }
    return network_close(server);
}
