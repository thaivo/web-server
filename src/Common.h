/*
 * common.h
 *
 *  Created on: Jan 27, 2016
 *      Author: INI\thai.vo
 */

#ifndef COMMON_H_
#define COMMON_H_
#include <fstream>
#include <string>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/mman.h>
using namespace std;
#define SEMKEYPATH "/dev/null"  /* Path used on ftok for semget key  */
#define SEMKEYID 1              /* Id used on ftok for semget key    */

#define NUMSEMS 1               /* Num of sems in created sem set    */

#define FILE_LENGTH 0x100

static const string AND_DELIMITER = "&";
static const string ASSIGNMENT_DELIMITER = "=";
static const string COMMA_DELIMITER = ",";

struct DATA {
	string op;
	string id;
	string value;
};

struct DB{//info db server will be push to config file
	string hostname;
	string username;
	string password;
	unsigned int port;
	string dbname;
};

extern vector<DB> dbInfoArrays;

class FFError {
public:
	string Label;

	FFError() {
		Label = (char *) "Generic Error";
	}
	FFError(char *message) {
		Label = message;
	}
	~FFError() {
	}
	inline const char* GetMessage(void) {
		return Label.c_str();
	}
};

int binary_semaphore_wait(int semid) {
	struct sembuf operations[1];
	/* Use the first (and only) semaphore.  */
	operations[0].sem_num = 0;
	/* Decrement by 1.  */
	operations[0].sem_op = -1;
	/* Permit undo'ing.  */
	operations[0].sem_flg = 0;
	printf("wait sem_op2 = %d\n", operations[0].sem_op);
	return semop(semid, operations, 1);
}

/* Post to a binary semaphore: increment its value by one.  This
 returns immediately.  */

int binary_semaphore_post(int semid) {
	struct sembuf operations[1];
	/* Use the first (and only) semaphore.  */
	operations[0].sem_num = 0;
	/* Increment by 1.  */
	operations[0].sem_op = 1;
	/* Permit undo'ing.  */
	operations[0].sem_flg = IPC_NOWAIT;
	printf("post sem_op2 = %d\n", operations[0].sem_op);
	return semop(semid, operations, 1);
}

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
	struct seminfo *__buf;
};

/* Initialize a binary semaphore with a value of one.  */

int binary_semaphore_initialize(int semid) {
	union semun argument;
	unsigned short values[1];
	values[0] = 1;
	argument.array = values;
	return semctl(semid, 1, SETALL, argument);
}


#endif /* COMMON_H_ */
