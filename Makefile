CC = gcc
CFLAGS = -c
EXECUTABLE = main
SRC = main.c
OBJ = functions.o 

$(EXECUTABLE): $(OBJ) $(SRC)
	$(CC) $(SRC) $(OBJ) -o $(EXECUTABLE) 

functions.o: functions.c functions.h
	$(CC) $(CFLAGS) functions.c 

clean:
	rm -f $(EXECUTABLE) *.o
