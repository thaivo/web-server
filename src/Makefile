CC = g++
DEBUG = -g
CFLAGS = -O2
LFLAGS = -Wall 
LIBMEMACHED = -lmemcached 
LIBMICROHTTPD = -lmicrohttpd 
LIBMYSQLCLIENT = -lmysqlclient
all : Httpd.o PingDBServer.o
	$(CC) $(LFLAGS) Httpd.o -o Httpd $(LIBMEMACHED) $(LIBMICROHTTPD) $(LIBMYSQLCLIENT); \
	$(CC) $(LFLAGS) PingDBServer.o -o PingDBServer  $(LIBMYSQLCLIENT)

Httpd.o: Httpd.cpp
	$(CC) $(CFLAGS) -c Httpd.cpp 
	
PingDBServer.o: PingDBServer.cpp
	$(CC) $(CFLAGS) -c PingDBServer.cpp

clean:
	rm -f *.o Httpd PingDBServer
