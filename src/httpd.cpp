//============================================================================
// Name        : httpd.cpp
// Author      : Thai Vo
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

/*
 * simplepost.c
 *
 *  Created on: Jan 14, 2016
 *      Author: INI\thai.vo
 */

/* Feel free to use this example code in any way
 you see fit (Public Domain) */

#include <sys/types.h>
#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#endif
#include <microhttpd.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <map>
#include <mysql/mysql.h>
#include "MysqlConn.h"
#include "memcached.h"
using namespace std;
#define PORT            8000
#define POSTBUFFERSIZE  512
#define MAXNAMESIZE     20
#define MAXANSWERSIZE   512

MYSQLCONN *MYSQLCONN::MyConnect = 0;
MyMemcached *MyMemcached::myMemcached =0;
#define GET             0
#define POST            1

pthread_mutex_t mutex_db =	PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_response = PTHREAD_MUTEX_INITIALIZER;
struct connection_info_struct
{
	int connectiontype;
	char *answerstring;
	struct MHD_PostProcessor *postprocessor;
};

const char *askpage = "<html><body>\
                       What's your name, Sir?<br>\
                       <form action=\"/namepost\" method=\"post\">\
                       <input name=\"name\" type=\"text\">\
                       <input type=\"submit\" value=\" Send \"></form>\
                       </body></html>";

const char *greetingpage = "<html><body><h1>Welcome, %s!</center></h1></body></html>";

const char *errorpage =	"<html><body>This doesn't seem to be right.</body></html>";



void handleDataIntoDb(DATA& data, bool& isKeyExisted) {
	char* temp = (char*) malloc(256 * sizeof(char));

	if (data.op == ACTION[0]) //insert
			{
		sprintf(temp,
				"%s into temp (id,value) VALUES ('%s','%s')",
				"insert", data.id.c_str(), data.value.c_str());
	} else if (data.op == ACTION[1]) //update
			{
		sprintf(temp,
				"%s temp set value='%s' where id='%s'",
				"update", data.value.c_str(), data.id.c_str());

	} else if(data.op == ACTION[2])//delete
	{
		sprintf(temp, "%s from temp where id = '%s'", "delete",
				data.id.c_str());
	}
	else{//get data
		sprintf(temp,"select value from temp where id = %s",data.id.c_str());
	}
	string sqlQuery = (const char*) temp;
	printf("sqlQuery : %s\n",sqlQuery.c_str());
	int mysqlStatus = mysql_query(MYSQLCONN::getInstance()->getConnection(),
			sqlQuery.c_str());
	if (mysqlStatus) {
		printf("mysql error: %s\n",
				(char*) mysql_error(MYSQLCONN::getInstance()->getConnection()));
		throw FFError(
				(char*) mysql_error(MYSQLCONN::getInstance()->getConnection()));
	}
	MYSQL_RES* res = mysql_store_result(
			MYSQLCONN::getInstance()->getConnection());
	if(data.op == ACTION[3]){
		if (res){  // there are rows
			// Returns the number of columns in a result set specified
			 unsigned int numFields = mysql_num_fields(res);
			 MYSQL_ROW  mysqlRow;
			while ((mysqlRow = mysql_fetch_row(res))) // row pointer in the result set
			{
				for (unsigned int ii = 0; ii < numFields; ii++) {
					data.value = mysqlRow[ii];
					printf("Row: %s\t", mysqlRow[ii] ? mysqlRow[ii] : "NULL"); // Not NULL then print
				}
				isKeyExisted = true;
				printf("\n");
			}
		} else {
			printf("Result set is empty");
		}
	}
	mysql_free_result(res);
	res = NULL;
	delete temp;

}

static int send_page(struct MHD_Connection *connection, const char *page) {
	int ret;
	struct MHD_Response *response;
	pthread_mutex_lock(&mutex_response);
	response = MHD_create_response_from_buffer(strlen(page), (void *) page,
			MHD_RESPMEM_PERSISTENT);
	if (!response)
		return MHD_NO;

	ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);
	pthread_mutex_unlock(&mutex_response);
	return ret;
}

static int iterate_post(void *coninfo_cls, enum MHD_ValueKind kind,
		const char *key, const char *filename, const char *content_type,
		const char *transfer_encoding, const char *data, uint64_t off,
		size_t size) {
	struct connection_info_struct *con_info =
			(connection_info_struct *) coninfo_cls;

	if (0 == strcmp(key, "name")) {
		if ((size > 0) && (size <= MAXNAMESIZE)) {
			char *answerstring;
			answerstring = new char[MAXANSWERSIZE];
			if (!answerstring)
				return MHD_NO;

			snprintf(answerstring, MAXANSWERSIZE, greetingpage, data);
			con_info->answerstring = answerstring;
		} else
			con_info->answerstring = NULL;

		return MHD_NO;
	}

	return MHD_YES;
}

static void request_completed(void *cls, struct MHD_Connection *connection,
		void **con_cls, enum MHD_RequestTerminationCode toe) {
	struct connection_info_struct *con_info =
			(connection_info_struct *) *con_cls;

	if (NULL == con_info)
		return;

	if (con_info->connectiontype == POST) {
		MHD_destroy_post_processor(con_info->postprocessor);
		if (con_info->answerstring)
			free(con_info->answerstring);
	}

	free(con_info);
	*con_cls = NULL;
}

int answer_to_connection(void *cls, struct MHD_Connection *connection,
		const char *url, const char *method, const char *version,
		const char *upload_data, size_t *upload_data_size, void **con_cls) {
	//check current connection and change to new connection if current connection died

	//
	if (NULL == *con_cls) {
		struct connection_info_struct *con_info;

//		con_info = malloc(sizeof(struct connection_info_struct));
		con_info = (connection_info_struct *) malloc(
				sizeof(struct connection_info_struct));
		if (NULL == con_info)
			return MHD_NO;
		con_info->answerstring = NULL;

		if (0 == strcmp(method, "POST")) {
			con_info->postprocessor = MHD_create_post_processor(connection,
			POSTBUFFERSIZE, iterate_post, (void *) con_info);

			if (NULL == con_info->postprocessor) {
				free(con_info);
				return MHD_NO;
			}

			con_info->connectiontype = POST;
		} else
			con_info->connectiontype = GET;

		*con_cls = (void *) con_info;

		return MHD_YES;
	}

	if (0 == strcmp(method, "GET")) {
		return send_page(connection, askpage);
	}

	if (0 == strcmp(method, "POST")) {
//		struct connection_info_struct *con_info = *con_cls;
		struct connection_info_struct *con_info =
				(connection_info_struct *) *con_cls;

		if (*upload_data_size != 0) {
			DATA data;
			pthread_mutex_lock(&mutex_db);
			usleep(500000);
			//check connect to current db and switch if it die
			SwitchToNewConnectionIfCurrentConnectionDied();
			//end check connect to current db
			parseUploadData(upload_data, data);
			//handle memcached
			bool isKeyExistedInMemcached = isKeyExisted(data.id.c_str());
			bool isKeyExistedInDb = false;
			handleMemcached(data,isKeyExistedInMemcached);
			if(!isKeyExistedInMemcached || data.op != ACTION[3]){
				handleDataIntoDb(data, isKeyExistedInDb);
			}
			if(!isKeyExistedInMemcached && isKeyExistedInDb && data.op == ACTION[3])
			{//load key-value from db to memcached if key-value pair does not exist in memcached
				data.op == ACTION[0];
				handleMemcached(data,isKeyExistedInMemcached);
			}
			//end handle memcached

			MHD_post_process(con_info->postprocessor, upload_data,
					*upload_data_size);
			*upload_data_size = 0;
			pthread_mutex_unlock(&mutex_db);

			return MHD_YES;
		} else if (NULL != con_info->answerstring)
			return send_page(connection, con_info->answerstring);
	}

	return send_page(connection, errorpage);
}

int main() {
	struct MHD_Daemon *daemon;

	daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, PORT, NULL, NULL,
			&answer_to_connection, NULL, MHD_OPTION_NOTIFY_COMPLETED,
			request_completed,
			NULL, MHD_OPTION_END);
	if (NULL == daemon)
		return 1;

	(void) getchar();

	MHD_stop_daemon(daemon);

	return 0;
}

