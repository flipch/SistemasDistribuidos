#Grupo 51: Felipe Heliszkowski 47064
#Gonçalo Cardoso 46784
#Pedro Gama 47081

CC=gcc
INCLUDE = inc/
OBJ = obj/
SRC = src/
FLAG = gcc -g -Wall -Iinc/ -c

all : table-client.o table-server.o table-client table-server test_message.o test_message

data.o: $(INCLUDE)data.h
	$(FLAG) $(SRC)data.c -o $(OBJ)data.o

entry.o: $(INCLUDE)entry.h $(INCLUDE)data.h
	$(FLAG) $(SRC)entry.c -o $(OBJ)entry.o

table.o: $(INCLUDE)table-private.h $(INCLUDE)table.h $(INCLUDE)entry.h $(INCLUDE)data.h
	$(FLAG) $(SRC)table.c -o $(OBJ)table.o

message.o: $(INCLUDE)message.h $(INCLUDE)entry.h
	$(FLAG) $(SRC)message.c -o $(OBJ)message.o

table-server.o: $(INCLUDE)inet.h $(INCLUDE)table-private.h
	$(FLAG) $(SRC)table-server.c -o $(OBJ)table-server.o

table-client.o: $(INCLUDE)network_client-private.h
	$(FLAG) $(SRC)table-client.c -o $(OBJ)table-client.o

table-server: $(OBJ)table-server.o $(OBJ)network_client.o $(OBJ)message.o $(OBJ)table.o $(OBJ)table.o $(OBJ)entry.o $(OBJ)data.o
	$(FLAG) $(OBJ)table-server.o $(OBJ)network_client.o $(OBJ)message.o $(OBJ)table.o $(OBJ)table.o $(OBJ)entry.o $(OBJ)data.o -o bin/table-client

table-client: $(OBJ)table-client.o $(OBJ)network_client.o $(OBJ)message.o $(OBJ)table.o $(OBJ)table.o $(OBJ)entry.o $(OBJ)data.o
	$(FLAG) $(OBJ)table-client.o $(OBJ)network_client.o $(OBJ)message.o $(OBJ)table.o $(OBJ)table.o $(OBJ)entry.o $(OBJ)data.o -o bin/table-client

test_message.o: $(INCLUDE)message.h $(INCLUDE)data.h $(INCLUDE)entry.h
	$(FLAG) $(SRC)test_message.c -o $(OBJ)test_message.o

test_message: $(OBJ)test_message.o $(OBJ)message.o $(OBJ)data.o $(OBJ)entry.o
	$(CC) $(OBJ)test_message.o $(OBJ)message.o $(OBJ)data.o $(OBJ)entry.o $(OBJ)table.o -o bin/test_message

clean:
	rm $(OBJ)*.o
	rm bin/*