# web-server
implement web-server with micro-httpd to process data into mysql database server

make sure memcached server running

in directory httpd:
run make command to build binary in httpd/src/
release mode: make release
debug mode: make debug

start tool to monitor webserver httpd
./monitor_webserver.sh

start tool to monitor mysql database server
./monitor_database_server.sh

client request webserver (or you can write your own):
./autotest_client_wget.sh



