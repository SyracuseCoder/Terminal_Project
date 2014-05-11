/*	client.c
 *	this file defines how client work actually, and define some important behaviors of client.
 *	client.c is quite straightforward, only use some pthread
 */
#ifndef CLIENT_HEADER
#define CLIENT_HEADER

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include "shmem.h"

typedef struct client{
	int pid;
	mem_t* mem;
	pthread_t thd_read;
	int flag_waiting;
	int status;			// status=0, run, status = -1, stop,
} client_t;				// status=-2 exit;

int destroy_client(client_t* client);

void* read_handler(void* arg){
	client_t* client = (client_t*)arg;	
	char buf[MEM_SIZE];
	while(client->status!=-2){
		read_mem(client->mem, buf);
		if(strcmp(buf, CLIENT_READY)==0)
			continue;
		if(strcmp(buf, KILL_CLIENT)==0){
			printf("closing client...\n");
			write_mem_cmd(client->mem, CLIENT_CLOSING);
			destroy_client(client);
			raise(SIGINT);
			return NULL;
		}
		if(strcmp(buf, SERVER_DONE)==0){
			client->flag_waiting = 0;
			continue;
		}
		printf("%s", buf);
	}	
	return NULL;
}

int init_client(client_t* client){
	client->pid = getpid();
	char pidbuf[20] = "";
        sprintf(pidbuf, "%d", client->pid);
        char mem_name[1024]="";
        strcat(mem_name, "/mem");
        strcat(mem_name, pidbuf);
        client->mem = init_mem(mem_name);
	client->status = 0;
	write_mem_cmd(client->mem, CLIENT_READY);	
	return 0;
}

int work_client(client_t* client){
	if(pthread_create(&client->thd_read, NULL, read_handler, client)!=0){
		perror("pthread_create");
		exit(EXIT_FAILURE);
	}
	while(1){
		char* msg = NULL;
		size_t size;
		getline(&msg, &size, stdin);
		if(strcmp(msg, "exit\n")==0 ){
			printf("client closing...\n");
			write_mem_cmd(client->mem, CLIENT_CLOSING);
			destroy_client(client);
			return 0;
		}
		if(client->flag_waiting==1)
			continue;
		write_mem_cmd(client->mem, msg);
		client->flag_waiting=1;
	}
	return 0;
}	
	
int destroy_client(client_t* client){
	client->status = -2;
	if(pthread_cancel(client->thd_read)==-1)
		errExit("pthread cancel");
	destroy_mem(client->mem);
	return 0;
}

#endif












