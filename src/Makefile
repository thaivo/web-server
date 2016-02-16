CC = g++
DEBUG = -g
CFLAGS = -O2
LFLAGS = -Wall 
LIBMEMACHED = -lmemcached 
LIBMICROHTTPD = -lmicrohttpd 
LIBMYSQLCLIENT = -lmysqlclient
release : Httpd.o PingDBServer.o
	$(CC) $(LFLAGS) Httpd.o -o Httpd $(LIBMEMACHED) $(LIBMICROHTTPD) $(LIBMYSQLCLIENT); \
	$(CC) $(LFLAGS) PingDBServer.o -o PingDBServer  $(LIBMYSQLCLIENT)

debug : Httpd.o PingDBServer.o
	$(CC) $(DEBUG) $(LFLAGS) Httpd.o -o Httpd $(LIBMEMACHED) $(LIBMICROHTTPD) $(LIBMYSQLCLIENT); \
	$(CC) $(DEBUG) $(LFLAGS) PingDBServer.o -o PingDBServer  $(LIBMYSQLCLIENT)

%.o: %.cpp
	$(CC) $(CFLAGS) -c Httpd.cpp 
	$(CC) $(CFLAGS) -c PingDBServer.cpp

clean:
	rm -f *.o Httpd PingDBServer
