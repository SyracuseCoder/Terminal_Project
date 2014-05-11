/*	By: Yiming Xiao
 *	shmem.h, this file constructs a shared memory and used for communication between clients and server,
 *	client.c and server.c are built based on the functions in this file.
 */
#ifndef SHMEM_HEADER
#define SHMEM_HEADER

#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <semaphore.h>
#include <errno.h>
#include "list.h"

#define MEM_SIZE	1024    

#define CLIENT_CLOSING "client|closing!!!"
#define CLIENT_READY   "client|ready!!!"
#define KILL_CLIENT    "kill|you|client!!!"
#define SERVER_DONE    "server|is|finished!!!"

typedef struct mem{
	char name[MEM_SIZE];
        pid_t sender_id;
        sem_t sem;
        int head;
        int tail;
        int size;
	int kill_sig;

	sem_t sem_cmd;
	int head_cmd;
	int tail_cmd;
	int size_cmd;

        char str[MEM_SIZE][MEM_SIZE];
	char cmd[MEM_SIZE][MEM_SIZE];
} mem_t;

//for debug only
void get_status(mem_t* mem){
	int val;
	if(sem_getvalue(&mem->sem, &val)==-1)
		errExit("sem get value");
	printf("current value sem: %d\n", val);
}

mem_t* init_mem( char* file){
	int init_sem = 0;
        int fd = shm_open(file, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
        if(fd==-1){
                fd = shm_open(file, O_RDWR, 0);
		if(fd==-1)
			errExit("shm_open");
	}
        else
                init_sem =1;
        if(fd==-1){
		if(close(fd)<0)
			errExit("close fd");
                errExit("fail to open shmem");
	}
        if(ftruncate(fd, sizeof(mem_t))==-1)
                errExit("ftruncate");
        mem_t* mem = mmap(NULL, sizeof(mem_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if(mem==MAP_FAILED)
                errExit("mmap");
        if(init_sem==1){
                if(sem_init(&mem->sem, (int)mem, 0)==-1)
                        errExit("sem_init");
		if(sem_init(&mem->sem_cmd, (int)mem, 0)==-1)
			errExit("sem_init");
		strcpy(mem->name, file);
		mem->head = 0;
		mem->tail = 0;
		mem->size = 0;
	
		mem->head_cmd = 0;
		mem->tail_cmd = 0;
		mem->size_cmd = 0;
	}
	if(close(fd)<0)
		errExit("close fd");
	return mem;
}

// read reponse part of the memory
void read_mem(mem_t* mem, char* buf){
	if(sem_wait(&mem->sem)==-1)
		errExit("sem_wait");
	strncpy(buf, mem->str[mem->head],MEM_SIZE );
        mem->head++;
        if(mem->head==MEM_SIZE)
               	mem->head = 0;
       	mem->size--;
}

// read the comd from the memory
void read_mem_cmd(mem_t* mem, char* buf){
	if(sem_wait(&mem->sem_cmd)==-1)
		errExit("sem_wait");
	strncpy(buf, mem->cmd[mem->head_cmd], MEM_SIZE);
	mem->head_cmd++;
	if(mem->head_cmd==MEM_SIZE)
		mem->head_cmd = 0;
	mem->size_cmd--;
}

// write response to memory
void write_mem(mem_t* mem, char* buf){
	while(mem->size==MEM_SIZE)
		sleep(1);
	mem->sender_id = getpid();
	strncpy(mem->str[mem->tail], buf, MEM_SIZE);
	mem->tail++;
	if(mem->tail == MEM_SIZE)
		mem->tail = 0;
	mem->size++;
	if(sem_post(&mem->sem)==-1)
		errExit("sempost");
} 

// write cmd to memory
void write_mem_cmd(mem_t* mem, char* buf){
	while(mem->size_cmd == MEM_SIZE)
		sleep(1);
	mem->sender_id = getpid();
	strncpy(mem->cmd[mem->tail_cmd], buf, MEM_SIZE);
	mem->tail_cmd++;
	if(mem->tail_cmd == MEM_SIZE)
		mem->tail_cmd = 0;
	mem->size_cmd++;
	if(sem_post(&mem->sem_cmd)==-1)
		errExit("sempost");
}

void destroy_mem(mem_t* mem){
	char tmp[1024];
	strncpy(tmp, mem->name, MEM_SIZE);
	if(munmap(mem, sizeof(mem_t))<0)
		errExit("munmap");
	if(sem_destroy(&mem->sem)<0)
		errExit("sem_destroy");
	if(sem_destroy(&mem->sem_cmd)<0)
		errExit("sem destroy");
	if(shm_unlink(tmp)<0){
		if(errno!=ENOENT) // this case is due to the file des has been unlinked in one process
			errExit("shm_unlink");
	}
}
		
#endif


















