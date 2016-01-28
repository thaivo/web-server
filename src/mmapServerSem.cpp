/*
 * shm_Server_s.c
 *
 *  Created on: Jan 21, 2016
 *      Author: INI\thai.vo
 */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/* FUNCTION:  This program acts as a server to the client program.   */
/*                                                                   */
/* LANGUAGE:  ILE C                                                  */
/*                                                                   */
/* APIs USED: semctl(), semget(), semop(),                           */
/*            shmat(), shmctl(), shmdt(), shmget()                   */
/*            ftok()                                                 */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <errno.h>
#include <string>
#include <vector>
#include <mysql/mysql.h>
#include "common.h"
using namespace std;

struct DB{//info db server will be push to config file
	string hostname;
	string username;
	string password;
	unsigned int port;
	string dbname;
};
/*
 * init array of database server info
 */
void initArrayDBS(vector<DB> &dbs){//enhance: get info db servers from config file
	for (int i = 0; i < 2; ++i) {
		DB db;
		db.hostname = "127.0.0.1";
		db.username = "root";
		db.password = "root";
		db.port = 3306+i;
		db.dbname = "OC";
		dbs.push_back(db);
	}
}

int main(int argc, char *argv[]) {
	int rc, semid;
	key_t semkey;

	/* Generate an IPC key for the semaphore set and the shared      */
	/* memory segment.  Typically, an application specific path and  */
	/* id would be used to generate the IPC key.                     */
	semkey = ftok(SEMKEYPATH, SEMKEYID);
	if (semkey == (key_t) -1) {
		printf("main: ftok() for sem failed\n");
		return -1;
	}

	/* Create a semaphore set using the IPC key.  The number of      */
	/* semaphores in the set is two.  If a semaphore set already     */
	/* exists for the key, return an error. The specified permissions*/
	/* give everyone read/write access to the semaphore set.         */

	semid = semget(semkey, NUMSEMS, 0666 | IPC_CREAT | IPC_EXCL);
	if (semid == -1) {
		semid = semget(semkey, NUMSEMS, 0666);
		if (semid == -1) {

			printf("main: semget() failed :%s\n", strerror(errno));
			return -1;
		}

	}
	printf("semid : %d\n", semid);
	rc = binary_semaphore_initialize(semid);
	if (rc == -1) {
		printf("main: semctl() initialization failed: %s\n", strerror(errno));
		return -1;
	}
	int fd;
	void* file_memory;

	/* Prepare a file large enough to hold an unsigned integer.  */
	fd = open("test.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	lseek(fd, FILE_LENGTH + 1, SEEK_SET);
	write(fd, "", 1);
	lseek(fd, 0, SEEK_SET);

	/* Create the memory-mapping.  */
	file_memory = mmap(0, FILE_LENGTH, PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);

	printf("Ready for client jobs\n");
	int port = 0;

	vector<DB> dbs;
	initArrayDBS(dbs);
	MYSQL *MySQLConnectionRet;
	MYSQL *connect = mysql_init( NULL);
	bool createConnection = false;
	for (;;) {
		//ping mysql and check mysql die or not
		//TO DO
		int current_port = port;
		printf("wait...\n");
		if(!createConnection)
		{
			for(size_t i = 0; i < dbs.size(); ++i){
				MySQLConnectionRet = mysql_real_connect(connect,
									dbs[i].hostname.c_str(), dbs[i].username.c_str(),
									dbs[i].password.c_str(), dbs[i].dbname.c_str(), dbs[i].port,
									NULL, 0);
				if (MySQLConnectionRet != NULL){
					createConnection = true;
					break;
				}
			}

			if (MySQLConnectionRet == NULL) {
				printf("all connections is off. Exit\n");
				return 0;
			}
		}
		int isAlive = mysql_ping(connect);
		printf("isAlive = %d\n",isAlive);
		sleep(5);
		if(isAlive != 0){
			if (connect->port == dbs[0].port){
				connect->port = dbs[1].port;
			}
			else{
				connect->port = dbs[0].port;
			}
			printf("connect-> port = %d\n",connect->port);
			connect->reconnect = true;
			continue;
		}
		port = connect->port;
		//end
		if (current_port != port) {
			printf("wait semaphore\n");
			rc = binary_semaphore_wait(semid);
			sleep(5);
			if (rc == -1) {
				printf("main: semop() failed\n");
				return -1;
			}

			sprintf((char*) file_memory, "%d\n", port);
			printf("post semaphore\n");
			rc = binary_semaphore_post(semid);
			if (rc == -1) {
				printf("main: semop() failed\n");
				return -1;
			}
		}

	} /* End of FOR LOOP */

	/* Clean up the environment by removing the semid structure,     */
	/* detaching the shared memory segment, and then performing      */
	/* the delete on the shared memory segment ID.                   */

	rc = semctl(semid, 1, IPC_RMID);
	if (rc == -1) {
		printf("main: semctl() remove id failed\n");
		return -1;
	}
	munmap(file_memory, FILE_LENGTH);
	return 0;
}

