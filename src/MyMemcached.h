/*
 * memcached.h
 *
 *  Created on: Jan 27, 2016
 *      Author: INI\thai.vo
 */

#ifndef MEMCACHED_H_
#define MEMCACHED_H_

#include <stdio.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <libmemcached/memcached.h>

#include "Common.h"
using namespace std;
class MyMemcached {
	memcached_server_st *servers;
	memcached_st *memc;
	static MyMemcached* myMemcached;
	MyMemcached() {
		memcached_return rc;
		servers = NULL;
		memc = memcached_create(NULL);
		servers = memcached_server_list_append(servers, "localhost", 11211,
				&rc);
		rc = memcached_server_push(memc, servers);
		if (rc == MEMCACHED_SUCCESS)
			fprintf(stderr, "Added server successfully\n");
		else
			fprintf(stderr, "Couldn't add server: %s\n",
					memcached_strerror(memc, rc));
	}
public:
	static MyMemcached* getInstance() {
		if (!myMemcached) {
			myMemcached = new MyMemcached;
		}
		return myMemcached;
	}

	memcached_st* getMemc() const {
		return memc;
	}
};

/*
 * check whether key exist or not in memcached server
 */
bool isKeyExisted(const char* key) {
	memcached_st *memc = MyMemcached::getInstance()->getMemc();
	memcached_return_t rc = memcached_exist(memc, key, strlen(key));
	if (rc == MEMCACHED_SUCCESS)
		return true;
	return false;
}

/*
 * process data for memcached
 */
void handleMemcached(DATA data, bool isKeyExisted) {
	memcached_st *memc = MyMemcached::getInstance()->getMemc();
	memcached_return_t rc;
	const char* key = data.id.c_str();

	if (isKeyExisted) {
		if (data.op == ACTION[0] || data.op == ACTION[1]) {//update
			const char* value = data.value.c_str();
			memcached_replace(memc,key,strlen(key),value,strlen(value),(time_t) 0, (uint32_t)0);
		} else if (data.op == ACTION[2]) { //remove key-value
			memcached_delete(memc, key, strlen(key), (time_t)0);
		} else{//get
			size_t* temp = (size_t*) malloc(sizeof(size_t));
			*temp = 64;
			data.op = (const char*)memcached_get(memc,key,strlen(key),temp,0,&rc);
		}
	} else {
		if (data.op == ACTION[0] || data.op == ACTION[1]) {
			const char* value = data.value.c_str();
			memcached_set(memc,key,strlen(key),value,strlen(value),(time_t)0,(uint32_t)0);
		} else
		{
			printf("key %s is not existed\n",key);
		}
	}

}

#endif /* MEMCACHED_H_ */
