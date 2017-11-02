#Grupo 51: Felipe Heliszkowski 47064
#Gonçalo Cardoso 46784
#Pedro Gama 47081

CC=gcc
INCLUDE = inc/
OBJ = obj/
BIN = binary/
SRC = src/
FLAG = gcc -g -Wall -I inc/ -c

all : data.o entry.o table.o  message.o test_message.o network_client.o table-client.o table-server.o test_message table-client table-server

data.o: $(INCLUDE)data.h
	$(FLAG) $(SRC)data.c -o $(OBJ)data.o

entry.o: $(INCLUDE)entry.h $(INCLUDE)data.h
	$(FLAG) $(SRC)entry.c -o $(OBJ)entry.o

table.o: $(INCLUDE)table-private.h $(INCLUDE)table.h
	$(FLAG) $(SRC)table.c -o $(OBJ)table.o

message.o: $(INCLUDE)message.h $(INCLUDE)table.h
	$(FLAG) $(SRC)message.c -o $(OBJ)message.o

network_client.o: $(INCLUDE)message.h $(INCLUDE)inet.h
	$(FLAG) $(SRC)network_client.c -o $(OBJ)network_client.o

table-client.o: $(INCLUDE)network_client-private.h $(INCLUDE)inet.h
	$(FLAG) $(SRC)table-client.c -o $(OBJ)table-client.o

table-server.o: $(INCLUDE)inet.h $(INCLUDE)table-private.h
	$(FLAG) $(SRC)table-server.c -o $(OBJ)table-server.o

test_message.o: $(INCLUDE)message.h
	$(FLAG) -c $(SRC)test_message.c -o $(OBJ)test_message.o

test_message: $(OBJ)test_message.o $(OBJ)message.o $(OBJ)table.o $(OBJ)entry.o $(OBJ)data.o
	$(CC) $(OBJ)test_message.o $(OBJ)message.o $(OBJ)table.o $(OBJ)entry.o $(OBJ)data.o -o binary/test_message

table-client: $(OBJ)table-client.o $(OBJ)network_client.o $(OBJ)message.o $(OBJ)data.o $(OBJ)entry.o $(OBJ)table.o
	$(CC) $(OBJ)table-client.o $(OBJ)network_client.o $(OBJ)message.o $(OBJ)data.o $(OBJ)entry.o $(OBJ)table.o -o binary/table-client

table-server: $(OBJ)table-server.o $(OBJ)network_client.o $(OBJ)message.o $(OBJ)data.o $(OBJ)entry.o $(OBJ)table.o
	$(CC) $(OBJ)table-server.o $(OBJ)network_client.o $(OBJ)message.o $(OBJ)data.o $(OBJ)entry.o $(OBJ)table.o -o binary/table-server

clean:
	rm $(OBJ)*.o
	rm binary/*
