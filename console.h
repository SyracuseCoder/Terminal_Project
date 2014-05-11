/*
	console.c
	In this file, the real server is defined by struct console;
*/
#ifndef CONSOLE_HEADER
#define CONSOLE_HEADER

#include "server.h"

#define CONSOLE_INFO "/tmp/console_info"
#define CONSOLE_FIFO "/tmp/console_fifo"
#define CONSOLE_MEM  "/console_mem"
#define CONSOLE_MESG "console existed"

con_mem_t* mem;

typedef struct console{
	int fd_info;
	int fd_fifo;
	pthread_t thd_listen;
} console_t;

int init_console(console_t* console){
	if((console->fd_info = open(CONSOLE_INFO, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR))==-1)
		errExit("console open");
	char buf[10];
	int nread;
	if((nread = read(console->fd_info, buf, 10))==-1)
		errExit("console read");
	if(nread!=0){
		printf("Server Console is already existed... cannot start a new one...\n");
		return -1;
	}
	if(write(console->fd_info, CONSOLE_MESG ,1+strlen(CONSOLE_MESG) )==-1)
		errExit("write console msg");
	if((console->fd_fifo = mkfifo(CONSOLE_FIFO, S_IRWXU))==-1)
		errExit("mkfifo");
	int fd_mem = shm_open(CONSOLE_MEM, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	if(fd_mem==-1)
		errExit("shm open");
	if(ftruncate(fd_mem, sizeof(con_mem_t))==-1)
		errExit("ftruncate");
	mem = mmap(NULL, sizeof(con_mem_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, 0);
        if(mem==MAP_FAILED)
                errExit("mmap");
	if(close(fd_mem)==-1)
		errExit("close fdmem");
	mem->tail = 0;
	int i;
	for(i=0; i<MAX_SERVER_NUM; i++){
		mem->arr[i] = NULL;
		mem->flag[i] = 0;
	}	

	
	return 0;
}

int destroy_console(console_t* console){
	int i;
	for(i=0; i<mem->tail; i++){
			quit_server(mem->arr[i]);
			free(mem->arr[i]);
	}
	if(close(console->fd_info)==-1)
		errExit("close fd info");
	if(unlink(CONSOLE_INFO)==-1)
		errExit("unlink console info");
	if(unlink(CONSOLE_FIFO)==-1)
		errExit("unlink console fifo");
	if(munmap(mem, sizeof(con_mem_t))==-1)
		errExit("munmap con mem");
	if(shm_unlink(CONSOLE_MEM)==-1)
		errExit("shm unlink con mem");
	return 0;	
}

void* produce_server(void* arg){
	int* pid = (int*)arg;
	server_t* server = malloc(sizeof(server_t));
	init_server(server, *pid);
	work_server(server);

	int i;
	int existed = 0;
	for(i=0; i<mem->tail; i++){
		if(mem->arr[i]->client_pid==*pid ){
			existed = 1;
			break;
		}
	}
	if(existed==0){
		mem->arr[mem->tail] = server;
		mem->flag[mem->tail] = 1;
		mem->tail++;
	}
	//printf("produce server!!!mem->tail :%d\n", mem->tail);	
	return NULL;
}	

void* listen(void* arg){
	console_t* console = (console_t*)arg;
	int nread;
	char buf [20];
	int fd;
	if((fd = open(CONSOLE_FIFO, O_RDONLY))==-1)
		errExit("open fifo");
	for(;;){
		if((nread = read(fd, buf, 20))==-1)
			errExit("read fifo");
		if(nread>0){
			int pid = atoi(buf);
			pthread_t t1;
			pthread_create(&t1, NULL, produce_server, &pid);
		}	
	}
	return NULL;
}

int listen_console(console_t* console){
	if(pthread_create(&console->thd_listen, NULL, listen, console)==-1)
		errExit("pthread create");	
	return 0;
}

int list_console(console_t* console){
	int num = 0;
	int i=0; 
	for(i=0; i<mem->tail; i++){
		if(mem->flag[i]==1){
			num++;
			printf("  Client PID: %d\n", mem->arr[i]->client_pid);
		}
	}
	if(num==0)
		printf("  No client found!\n");
	return 0;
}

int cancel_cmd_console(console_t* console, int pid){
	int i=0;
	int find=0;
	for(i=0; i<mem->tail; i++){
		if(mem->flag[i]==1 && mem->arr[i]->client_pid==pid){
			find = 1;
			cancel_cmd(mem->arr[i]);
			break;
		}
	}
	if(find==0)
		printf("Invalid PID!\n");
	return 0;
}

int kill_client_console(console_t* console, int pid){
	int i=0;
	int find=0;
	for(i=0; i<mem->tail;i++){
		if(mem->flag[i]==1 && mem->arr[i]->client_pid ==pid){
			find = 1;
			kill_client(mem->arr[i]);
			break;
		}
	}
	if(find==0)
		printf("Invalid PID!\n");
	return 0;	
}

int kill_all_console(console_t* console){
	int i=0;
	for(i=0; i<mem->tail; i++){
		if(mem->flag[i]==1){
			kill_client(mem->arr[i]);
		}
	}
	return 0;
}

#endif























