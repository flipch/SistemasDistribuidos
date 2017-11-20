//Grupo 51: Felipe Heliszkowski 47064
//Gonçalo Cardoso 46784
//Pedro Gama 47081
/*
	Programa cliente para manipular tabela de hash remota.
	Os comandos introduzido no programa não deverão exceder
	80 carateres.

	Uso: table-client <ip servidor>:<porta servidor>
	Exemplo de uso: ./table_client 10.101.148.144:54321
*/

#include "network_client-private.h"
#include "message.h"
#include "client_stub.h"

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
        if (msg->content.data != NULL)
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
    char input[81];
    struct rtables_t *rtables;

    rtables = (struct rtables_t *)malloc(sizeof(struct rtables_t));

    /* Testar os argumentos de entrada */
    if (argc < 2)
    {
        printf("Escreva a porta de conexão\n");
        return -1;
    }

    /* Usar network_connect para estabelcer ligação ao servidor */
    rtables = rtables_bind(argv[1]);

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
        if (strcmp(input, "") <= 0)
        {
            printf("Escreva um comando no terminal\n");
        }

        /* Verificar se o comando foi "quit". Em caso afirmativo
		   não há mais nada a fazer a não ser terminar decentemente.
		*/

        token = strtok(input, " ");
        if (strcmp(token, "quit") == 0)
        {
            network_close(rtables->server);
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
            int fail = 0; //BOOLEAN VALUE FOR CHECK

            if (strcmp(token, "put") == 0)
            {
                token = strtok(NULL, " ");
                short tableNum = -1;
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
                    {
                        data = data_create2(sizeof(token), token);
                    }
                    else if (count == 2)
                    {
                        tableNum = atoi(token);
                    }
                    count++;
                    token = strtok(NULL, " ");
                }
                if (count != 3) // Um dos tokens nao foi inserido
                {
                    printf("Insira todos os argumentos necessários\n");
                    fail = 1;
                }
                else
                {
                    rtables->currentTable = tableNum;
                    fail = rtables_put(rtables, key, data);
                }
            }
            else if (strcmp(token, "get") == 0)
            {
                short tableNum = -1;
                token = strtok(NULL, " ");
                char *key;
                key = malloc(strlen(token));
                strcpy(key, token);
                token = strtok(NULL, " ");
                tableNum = atoi(token);
                if (tableNum == -1)
                {
                    printf("Insira todos os argumentos necessários\n");
                    fail = 1;
                }
                else
                {
                    rtables->currentTable = tableNum;
                    rtables_get(rtables, key);
                    fail = 0;
                }
            }
            else if (strcmp(token, "update") == 0)
            {
                char *key;
                short tableNum = -1;
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
                    else if (count == 2)
                        tableNum = atoi(token);
                    token = strtok(NULL, " ");
                    count++;
                }
                if (count != 3) // Um dos tokens nao foi inserido
                {
                    printf("Insira todos os argumentos necessários\n");
                    fail = 1;
                }
                else
                {
                    rtables->currentTable = tableNum;
                    fail = rtables_update(rtables, key, data);
                }
            }
            else if (strcmp(token, "colls") == 0)
            {
                short tableNum = -1;
                token = strtok(NULL, " ");
                tableNum = atoi(token);
                if (tableNum == -1)
                {
                    printf("Insira todos os argumentos necessários\n");
                    fail = 1;
                }
                else
                { 
                     rtables->currentTable = tableNum;
                     fail = rtables_collisions(rtables);
                }
            }
            else if (strcmp(token, "size") == 0)
            {
                short tableNum = -1;
                token = strtok(NULL, " ");
                tableNum = atoi(token);
                if (tableNum == -1)
                {
                    printf("Insira todos os argumentos necessários\n");
                    fail = 1;
                }
                else
                {
                    rtables->currentTable = tableNum;
                    fail = rtables_size(rtables);
                }
            }
            else
            {
                fail = 1;
            }
            if (fail == 1)
            {
                printf("--------------------------\n");
                printf("Introduza um comando certo\n");
                printf("put <key> <data> <table>\n");
                printf("get <key> <table>\n");
                printf("colls <table>\n");
                printf("update <key> <data> <table>\n");
                printf("size <table>\n");
                printf("quit\n");
                printf("--------------------------\n");
            }
            token = (char *)malloc(80);
        }
    }
    free(rtables->server);
    free(rtables);
    return network_close(rtables->server);
}
