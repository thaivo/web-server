CC = g++
DEBUG = -g
CFLAGS = -O2
LFLAGS = -Wall 
LIBMEMACHED = -lmemcached 
LIBMICROHTTPD = -lmicrohttpd 
LIBMYSQLCLIENT = -lmysqlclient
all : httpd.o PingDBServer.o
	$(CC) $(LFLAGS) httpd.o -o httpd $(LIBMEMACHED) $(LIBMICROHTTPD) $(LIBMYSQLCLIENT); \
	$(CC) $(LFLAGS) PingDBServer.o -o PingDBServer  $(LIBMYSQLCLIENT)

httpd.o: httpd.cpp
	$(CC) $(CFLAGS) -c httpd.cpp 
	
PingDBServer.o: PingDBServer.cpp
	$(CC) $(CFLAGS) -c PingDBServer.cpp

clean:
	rm -f *.o httpd PingDBServer
