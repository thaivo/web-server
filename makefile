CC = g++
DEBUG = -g
CFLAGS = -Wall -c 
LFLAGS = -Wall 
LIBS = -lmemcached -lmicrohttpd -lmysqlclient
all :
	cd src; echo "I'm in src_dir"; \
	$(CC) $(LFLAGS) httpd.cpp -o httpd $(LIBS)

clean:
	cd src/; echo "I'm in src_dir"; \
	rm *.o *~ httpd
