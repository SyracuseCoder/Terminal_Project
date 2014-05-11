/*
 *	server.c
 *	this file defines some basic behavior of the server, But actually the file console.c describes the real
 *	functions of server. 
 *	console.c depend on this file, and console_t is the real server
 */


#ifndef SERVER_HEADER
#define SERVER_HEADER

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "shmem.h"
#include <signal.h>
#include <wait.h>
#include <pthread.h>

#define QUIT_SIG "My|Quit|Sig!!!"
#define MAX_SERVER_NUM 1024

typedef struct server{
        mem_t* mem;
        int client_pid;
        int status;             // 0 normal, -1 stop, -2 cancel cur running cmd
        int curr_cpid;
	int gpid;
} server_t;

// this is actually used by the console. A console has to mentain a shared memory to book-keep all the client info
typedef struct con_mem{
        int tail;
        server_t* arr[MAX_SERVER_NUM];
        int flag[MAX_SERVER_NUM];
} con_mem_t;

// defined in console.h, used to maintain some bookkeeping.
extern con_mem_t* mem;

int init_server(server_t* server, int cpid){
	server->client_pid = cpid;
	char pidbuf[20] = "";
	sprintf(pidbuf, "%d", cpid);
	char mem_name[1024]="";
	strcat(mem_name, "/mem");
	strcat(mem_name, pidbuf);
	server->mem = init_mem(mem_name);	
	server->status = 0;
	server->gpid = getpid();
	return 0;
}

int destroy_server(server_t* server){
	if(killpg(server->gpid, SIGKILL)==-1)
		errExit("killpg");
        destroy_mem(server->mem);
        return 0;
}

int cancel_cmd(server_t* server){
	if(killpg(server->curr_cpid, SIGKILL)==-1)
		errExit("killpg");
	char buf[30] = SERVER_DONE;
	write_mem(server->mem,buf); 
	return 0;
}
int kill_client(server_t* server){
	char buf[30] = KILL_CLIENT;
	write_mem(server->mem, buf);
	return 0;
}
int quit_server(server_t* server){
	write_mem_cmd(server->mem, QUIT_SIG);
	return 0;
}

int work_server(server_t* server){
	if(setpgid(0,0)==-1)
		errExit("setpgid");
	for(;;){
		char buf[1024];
		memset(buf, '\0', 1024);
		read_mem_cmd(server->mem, buf);
		if(strcmp(buf, QUIT_SIG)==0){
			printf("> server is quiting...\n");
			//destroy_mem(server->mem);
			destroy_server(server);
			break;
		}else if(strcmp(buf, CLIENT_READY)==0){
			//printf("> client: %d is ready...\n", server->client_pid);
		}else if(strcmp(buf, CLIENT_CLOSING)==0){
			int i;
			int find = 0;
			for(i=0; i<mem->tail; i++){
				if(mem->arr[i]->client_pid == server->client_pid && mem->flag[i]==1){
					mem->flag[i] = 0;
					find = 1;
					break;
				}
			}
			if(find==0){
				mem->flag[mem->tail-1] = 0;
			}
			break;
		}		 
		int pid = fork();
		if(pid==-1)
			errExit("fork");
		else if(pid!=0){
			server->curr_cpid = pid;		
		}else{
			if(setpgid(0,0)==-1)
				errExit("setpgid");
			int fd[2];
			if(pipe(fd)==-1)
				errExit("pipe");
			int cpid = fork();
			if(cpid==-1)
				errExit("fork");
			else if(cpid!=0){
				if(close(fd[0])==-1)
					errExit("close");
				if(close(STDOUT_FILENO)==-1)
					errExit("close");
				if(close(STDERR_FILENO)==-1)
					errExit("close");
				if(dup2(fd[1], STDOUT_FILENO)==-1)
					errExit("dup2");
				if(dup2(fd[1], STDERR_FILENO)==-1)
					errExit("dup2");
				if(strcmp(buf, CLIENT_READY)==0){
					printf(CLIENT_READY);
				}else{
					if(system(buf)==-1)
						errExit("system");	
					char done[30] = SERVER_DONE;
					write_mem(server->mem, done);
				}
				return 0;
			}else{
				if(close(fd[1])==-1)
					errExit("close fd1");
                		while(1){
                        		char buf2[1024];
                        		memset(buf2, '\0', 1024);
                        		int nread;
                        		nread = read(fd[0], buf2, 1024);
					if(nread==-1)
						errExit("read fd0");
                        		if(nread>0)
                                		write_mem(server->mem, buf2);
                		}
				return 0;
			}		
		}
	}
	return 0;
}

#endif
































	
