#Grupo 51: Felipe Heliszkowski 47064
#Gonçalo Cardoso 46784
#Pedro Gama 47081

CC=gcc
INCLUDE = inc/
OBJ = obj/
BIN = binary/
SRC = src/
FLAG = gcc -g -Wall -I inc/ -c

all : data.o entry.o table.o primary_backup.o message.o table_skel.o client_stub.o test_message.o network_client.o table-client.o table-server.o test_message table-client table-server

data.o: $(INCLUDE)data.h
	$(FLAG) $(SRC)data.c -o $(OBJ)data.o

entry.o: $(INCLUDE)entry.h $(INCLUDE)data.h
	$(FLAG) $(SRC)entry.c -o $(OBJ)entry.o

table.o: $(INCLUDE)table-private.h $(INCLUDE)table.h
	$(FLAG) $(SRC)table.c -o $(OBJ)table.o

message.o: $(INCLUDE)message.h $(INCLUDE)table.h
	$(FLAG) $(SRC)message.c -o $(OBJ)message.o

table_skel.o: $(INCLUDE)message.h $(INCLUDE)table_skel.h 
	$(FLAG) $(SRC)table_skel.c -o $(OBJ)table_skel.o

client_stub.o: $(INCLUDE)data.h $(INCLUDE)message.h $(INCLUDE)network_client.h $(INCLUDE)client_stub-private.h $(INCLUDE)client_stub.h
	$(FLAG) $(SRC)client_stub.c -o $(OBJ)client_stub.o	

network_client.o: $(INCLUDE)message.h $(INCLUDE)inet.h
	$(FLAG) $(SRC)network_client.c -o $(OBJ)network_client.o

primary_backup.o: $(INCLUDE)primary_backup.h $(INCLUDE)primary_backup-private.h $(INCLUDE)table-private.h $(INCLUDE)table.h
	$(FLAG) $(SRC)primary_backup.c -o $(OBJ)primary_backup.o

table-client.o: $(INCLUDE)network_client-private.h $(INCLUDE)inet.h
	$(FLAG) $(SRC)table-client.c -o $(OBJ)table-client.o

table-server.o: $(INCLUDE)inet.h $(INCLUDE)table-private.h $(INCLUDE)primary_backup.h 
	$(FLAG) $(SRC)table-server.c -o $(OBJ)table-server.o

test_message.o: $(INCLUDE)message.h
	$(FLAG) $(SRC)test_message.c -o $(OBJ)test_message.o

test_message: $(OBJ)test_message.o $(OBJ)message.o $(OBJ)table.o $(OBJ)entry.o $(OBJ)data.o
	$(CC) $(OBJ)test_message.o $(OBJ)message.o $(OBJ)table.o $(OBJ)entry.o $(OBJ)data.o -o binary/test_message

table-client: $(OBJ)table-client.o $(OBJ)client_stub.o $(OBJ)network_client.o $(OBJ)message.o $(OBJ)data.o $(OBJ)entry.o $(OBJ)table.o
	$(CC) $(OBJ)table-client.o $(OBJ)client_stub.o $(OBJ)network_client.o $(OBJ)message.o $(OBJ)data.o $(OBJ)entry.o $(OBJ)table.o -o binary/table-client

table-server: $(OBJ)table-server.o  $(OBJ)client_stub.o $(OBJ)table_skel.o $(OBJ)network_client.o $(OBJ)message.o $(OBJ)data.o $(OBJ)entry.o $(OBJ)table.o $(OBJ)primary_backup.o
	$(CC) -pthread $(OBJ)table-server.o $(OBJ)client_stub.o $(OBJ)network_client.o  $(OBJ)table_skel.o $(OBJ)message.o $(OBJ)data.o $(OBJ)primary_backup.o $(OBJ)entry.o $(OBJ)table.o -o binary/table-server

clean:
	rm $(OBJ)*.o
	rm binary/*
