# SistemasDistribuidos
## Projeto de SD 
### Grupo51:

* fc47064- Felipe Heliszkowski
* fc46784- Gonçalo Cardoso
* fc47081- Pedro Gama

Limitações na implementação:


Compilação: Sem problemas na compilação

Executáveis: Sem problemas


TO-DO SHIT :
    0- Mutex TOP PRIORITY
    1- Done
        DONE
    2- DONE
    3-
        a- done
        b- criar uma quarta thread que age como um cliente
    4-
        a-done
        b- marcar dead no array de estados
            -usar update_State(primary_backup) - Nao usamos isto ainda
            -done.
            -marcar alive no array de estados.
---------#---------
    1-
        a-(table-client, usar o update_state do primary_backup)
        1- done

    2-
    switch dentro de um while , modos como 1 ou 2 para identificar primary ou sec
        1-operacao == write replicar para  o server em modo 2.(sec)

        2- aplicar a operacao recebido , sozinho.

    3- o "antigo" primario faz hello(primary_backup) ao "novo" primario(Sec) e muda para modo 2.
        a-durante esta operacao mudamos o state dos servers para maintenance.

