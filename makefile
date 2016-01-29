CC = g++
DEBUG = -g
CFLAGS = -Wall -c 
LFLAGS = -Wall 
LIBMEMACHED = -lmemcached 
LIBMICROHTTPD = -lmicrohttpd 
LIBMYSQLCLIENT = -lmysqlclient
all :
	cd src; echo "I'm in src_dir"; \
	$(CC) $(LFLAGS) httpd.cpp -o httpd $(LIBMEMACHED) $(LIBMICROHTTPD) $(LIBMYSQLCLIENT); \
	$(CC) $(LFLAGS) mmapServerSem.cpp -o mmapServerSem $(LIBMYSQLCLIENT)

clean:
	cd src/; echo "I'm in src_dir"; \
	rm httpd mmapServerSem
