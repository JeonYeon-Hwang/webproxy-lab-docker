# echo 서버 및 클라를 만드는 makerfile 입니다
CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

all: echoserver echoclient

echoserver: echoserver.o ../csapp.o
	$(CC) $(CFLAGS) echoserver.o ../csapp.o -o echoserver $(LDFLAGS)

echoclient: echoclient.o ../csapp.o
	$(CC) $(CFLAGS) echoclient.o ../csapp.o -o echoclient $(LDFLAGS)

echoserver.o: echoserver.c ../csapp.h
	$(CC) $(CFLAGS) -c echoserver.c

echoclient.o: echoclient.c ../csapp.h
	$(CC) $(CFLAGS) -c echoclient.c

../csapp.o: ../csapp.c ../csapp.h
	$(CC) $(CFLAGS) -c ../csapp.c -o ../csapp.o

clean:
	rm -f *~ *.o echoserver echoclient ../csapp.o