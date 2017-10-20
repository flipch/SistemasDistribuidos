#Grupo 51: Felipe Heliszkowski 47064
#Gonçalo Cardoso 46784
#Pedro Gama 47081

CC=gcc
INCLUDE = inc/
OBJ = obj/
SRC = src/
FLAG = gcc -g -Wall -Iinc/ -c

all : data.o entry.o table.o message.o test_message.o test_message

data.o: $(INCLUDE)data.h
	$(FLAG) $(SRC)data.c -o $(OBJ)data.o

entry.o: $(INCLUDE)entry.h $(INCLUDE)data.h
	$(FLAG) $(SRC)entry.c -o $(OBJ)entry.o

table.o: $(INCLUDE)table-private.h $(INCLUDE)table.h $(INCLUDE)entry.h $(INCLUDE)data.h
	$(FLAG) $(SRC)table.c -o $(OBJ)table.o

#test_data.o: $(INCLUDE)data.h
#	$(FLAG) $(SRC)test_data.c -o $(OBJ)test_data.o

#test_entry.o: $(INCLUDE)data.h $(INCLUDE)entry.h
#	$(FLAG) $(SRC)test_entry.c -o $(OBJ)test_entry.o

#test_table.o: $(INCLUDE)table-private.h $(INCLUDE)table.h $(INCLUDE)data.h $(INCLUDE)entry.h 
#	$(FLAG) $(SRC)test_table.c -o $(OBJ)test_table.o

#test_data: $(OBJ)test_data.o $(OBJ)data.o
#	$(CC) $(OBJ)test_data.o $(OBJ)data.o -o bin/test_data

#test_entry: $(OBJ)test_entry.o $(OBJ)data.o $(OBJ)entry.o
#	$(CC) $(OBJ)test_entry.o $(OBJ)data.o $(OBJ)entry.o -o bin/test_entry

#test_table: $(OBJ)test_table.o $(OBJ)data.o $(OBJ)entry.o $(OBJ)table.o
#	$(CC) $(OBJ)test_table.o $(OBJ)data.o $(OBJ)entry.o $(OBJ)table.o -o bin/test_table

message.o: $(INCLUDE)message.h $(INCLUDE)entry.h
	$(FLAG) $(SRC)message.c -o $(OBJ)message.o

test_message.o: $(INCLUDE)message.h $(INCLUDE)data.h $(INCLUDE)entry.h
	$(FLAG) $(SRC)test_message.c -o $(OBJ)test_message.o

test_message: $(OBJ)test_message.o $(OBJ)message.o $(OBJ)data.o $(OBJ)entry.o
	$(CC) $(OBJ)test_message.o $(OBJ)message.o $(OBJ)data.o $(OBJ)entry.o $(OBJ)table.o -o bin/test_message

test_1.o: $(INCLUDE)message.h $(INCLUDE)data.h $(INCLUDE)entry.h
	$(FLAG) $(SRC)test_1.c -o $(OBJ)test1.o

test_1: $(OBJ)test_message.o $(OBJ)message.o $(OBJ)data.o $(OBJ)entry.o
	$(CC) $(OBJ)test_1.o $(OBJ)message.o $(OBJ)data.o $(OBJ)entry.o $(OBJ)table.o -o bin/test_1	

clean:
	rm $(OBJ)*.o
	rm bin/*