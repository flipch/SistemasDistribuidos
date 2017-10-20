#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <netinet/in.h>

#include "message.h"

int testResult()
{
    // TO-DO
}

int main()
{
    int score = 0;

    printf("\nIniciando o teste do módulo message\n");

    score += testResult();

    printf("Resultados do teste do módulo message: %d em 6\n\n", score);
}