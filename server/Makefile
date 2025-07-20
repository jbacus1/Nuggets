# Makefile for grid, gridtest, and server
# Todd Rosenbaum

CC      = gcc
CFLAGS  = -g -Wall -pedantic -std=c11 -ggdb -I../libcs50 -I../support
LIBS    = ../libcs50/libcs50-given.a ../support/support.a -lm
OBJS    = server.o gridtest.o grid.o

.PHONY: all clean test

all: support gridtest server

support: ../support/support.a

../support/support.a:
	$(MAKE) -C ../support

gridtest: gridtest.o grid.o $(LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

gridtest.o: gridtest.c grid.h
	$(CC) $(CFLAGS) -c $< -o $@

grid.o: grid.c grid.h ../libcs50/hashtable.h ../libcs50/file.h
	$(CC) $(CFLAGS) -c $< -o $@

server: server.o grid.o $(LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

server.o: server.c grid.h
	$(CC) $(CFLAGS) -c $< -o $@

gridTest: gridtest
	./gridtest >> gridTesting.out

gridValgrind: gridtest
	valgrind ./gridtest 2> gridValgrindTest.out

# Clean up
clean:
	rm -f $(OBJS) gridtest server
	rm -f core
	rm -rf *~ *.o *.gch *.dSYM
	$(MAKE) -C ../support clean
