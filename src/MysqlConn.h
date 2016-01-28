/*
 * mysqlconn.h
 *
 *  Created on: Jan 15, 2016
 *      Author: INI\thai.vo
 */

#ifndef MYSQLCONN_H_
#define MYSQLCONN_H_
#include <mysql/mysql.h>
#include <string>
#include <stdlib.h>
#include <vector>

#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "common.h"
using namespace std;

pthread_mutex_t mutex_connection = PTHREAD_MUTEX_INITIALIZER;

class MYSQLCONN {
	static MYSQLCONN* MyConnect;
	MYSQL* connect;
//	vector<MYSQL*> array_connection;
	//change
//	int index_of_current_connection_inuse;
	//end change
	MYSQLCONN(MYSQL* conn = 0) :
			connect(conn) {
		initConn();
	}
	void initConn() {//enhance : load config file for database connection server
		connect->host="127.0.0.1";
		string hostName = (const char*) getenv("MYSQL_SERVER_ADDRESS");
		printf("2 initConn\n");
		string userId = (const char*) getenv("MYSQL_USERNAME");
		string password = (const char*) getenv("MYSQL_PASSWORD");
		string DB = (const char*) getenv("MYSQL_DATABASE_NAME");

		int port = atoi((const char*) getenv("MYSQL_PORT"));
		MYSQL *MySQLConnectionRet;
		connect = mysql_init( NULL);

		try {
			MySQLConnectionRet = mysql_real_connect(connect, hostName.c_str(),
					userId.c_str(), password.c_str(), DB.c_str(), port,
					NULL, 0);
			if(connect)printf("connect != NULL\n");
			if (MySQLConnectionRet == NULL)
				throw FFError((char*) mysql_error(connect));

			printf("MySQL Connection Info: %s \n",
					mysql_get_host_info(connect));
			printf("MySQL Client Info: %s \n", mysql_get_client_info());
			printf("MySQL Server Info: %s \n", mysql_get_server_info(connect));

		} catch (FFError& e) {
			printf("%s\n", e.Label.c_str());
		}
	}
public:
	static MYSQLCONN* getInstance() {
		printf("static MYSQLCONN* getInstance()\n");

		if (!MyConnect) {
				printf("MyConnect = NULL\n");
				MyConnect = new MYSQLCONN;
//			}
//			pthread_mutex_unlock(&mutex_connection);
			return MyConnect;
		} else {
			printf("MyConnect NOT NULL\n");
			return MyConnect;
		}
	}

	MYSQL* getConnection() {
		printf("MYSQL* getConnection()\n");
		if(connect) printf("connect NOT NULL\n");
		return connect;
	}

	bool DBBeginTransaction(){
		return mysql_query(connect,"START TRANSACTION");
	}

	bool DBEndTransaction(){
		return mysql_commit(connect);
	}

	bool DBRollback(){
		return mysql_rollback(connect);
	}
};

/*
 * check current connection alive or not
 */
int SwitchToNewConnectionIfCurrentConnectionDied()
{
	void *file_memory;
	int semid, rc;
	key_t semkey;

		/* Generate an IPC key for the semaphore set and the shared      */
		/* memory segment.  Typically, an application specific path and  */
		/* id would be used to generate the IPC key.                     */
	semkey = ftok(SEMKEYPATH, SEMKEYID);
	if (semkey == (key_t) -1) {
		printf("main: ftok() for sem failed\n");
		return -1;
	}

		/* Get the already created semaphore ID associated with key.     */
		/* If the semaphore set does not exist, then it will not be      */
		/* created, and an error will occur.                             */
	semid = semget(semkey, 1, 0666);
	printf("semid : %d\n", semid);
	if (semid == -1) {
		printf("main: semget() failed\n");
		return -1;
	}
		/* Attach the shared memory segment to the client process.       */
	int fd;
	int integer;
	/* Open the file.  */
	fd = open("test.txt", O_RDWR, S_IRUSR | S_IWUSR);
	/* Create the memory-mapping.  */
	file_memory = mmap(0, FILE_LENGTH, PROT_READ | PROT_WRITE,
	MAP_SHARED, fd, 0);
	close(fd);
	printf("wait semaphore\n");
	rc = binary_semaphore_wait(semid);
	if (rc == -1) {
		printf("main: semop() failed\n");
		return -1;
	}
	sleep(1);
	sscanf((const char*) file_memory, "%d", &integer);
	printf("current port in use: %d\n", MYSQLCONN::getInstance()->getConnection()->port);
	printf("port alive: %d\n", integer);
	if(MYSQLCONN::getInstance()->getConnection()->port != integer){//check whether current port is alive or not
		MYSQLCONN::getInstance()->getConnection()->port = integer;
		MYSQLCONN::getInstance()->getConnection()->reconnect = true;
		rc =mysql_ping(MYSQLCONN::getInstance()->getConnection());//connect to new instance database server
	}

	printf("post semaphore\n");
	rc = binary_semaphore_post(semid);
	if (rc == -1) {
		printf("main: semop() failed\n");
		return -1;
	}
	munmap (file_memory, FILE_LENGTH);
}
map<string,string> parseStringWithAssignmentDelimiter(vector<string> s);
void parseUploadData(string uploadData, DATA& data);
void handleDataIntoDb(DATA& data);

void parseStringWithAndDelimiter(vector<string>& output, string input) {
	std::string token;
	size_t pos = 0;
	while ((pos = input.find(AND_DELIMITER)) != std::string::npos) {
		token = input.substr(0, pos);
		output.push_back(token);
		input.erase(0, pos + AND_DELIMITER.length());
	}
	output.push_back(input);
}

map<string, string> parseStringWithAssignmentDelimiter(vector<string> s) {
	string delimiter = "=";
	size_t pos = 0;
	std::string token;
	map<string, string> ret;
	for(size_t i= 0; i<s.size(); ++i) {
		while ((pos = s[i].find(ASSIGNMENT_DELIMITER)) != std::string::npos) {
			token = s[i].substr(0, pos);
			s[i].erase(0, pos + delimiter.length());
		}
		ret[token] = s[i];
	}
	return ret;
}

void parseUploadData(string uploadData, DATA& data) {
	vector<string> expressionAssignment;
	parseStringWithAndDelimiter(expressionAssignment, uploadData);
	map<string, string> ret = parseStringWithAssignmentDelimiter(
			expressionAssignment);
	data.op = ret["op"];
	data.id = ret["id"];
	data.value = ret["name"];
}
#endif /* MYSQLCONN_H_ */
