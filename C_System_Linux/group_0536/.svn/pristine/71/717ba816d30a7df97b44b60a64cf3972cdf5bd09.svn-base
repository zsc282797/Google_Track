PORT = 57759
CFLAGS = -DPORT=$(PORT) -g -Wall -Werror -std=c99

all: mismatch_server

mismatch_server: mismatch_server.o
	gcc ${CFLAGS}  mismatch_server.c utils.c qtree.c questions.c -o mismatch_server
