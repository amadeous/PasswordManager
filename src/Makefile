CC=gcc
CLIBS=-lsodium
CFLAGS=-Wall -pedantic
HEADERS=menuOptions.h files.h crypto.h base64.h utils.h const.h
OBJECTS=menuOptions.o files.o crypto.o base64.o utils.o

default: passwordManager

%.o: %.c %.h

passwordManager: passwordManager.c $(OBJECTS) $(HEADERS)
	$(CC) -o $@ passwordManager.c $(OBJECTS) $(CFLAGS) $(CLIBS)
	rm *.o

passwordManager-debug: passwordManager.c $(OBJECTS) $(HEADERS)
	$(CC) -o $@ passwordManager.c $(OBJECTS) $(CFLAGS) $(CLIBS) -fsanitize=address -O1 -fno-omit-frame-pointer -g
	rm *.o
	
clean:
	rm passwordManager
	rm passwordManager-debug