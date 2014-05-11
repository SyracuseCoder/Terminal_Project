/*	By: Yiming Xiao, SYR, Spring 2014
 *      main.c 		
 *
 *
 */

#include "console.h"
#include "client.h"
#include "server.h"

void print_usage(int a){
	int m=0, n=0;
	if(a==0){
		m=1;
		n=1;
	}else if(a==1)
		m=1;
	else 
		n=1;
	if(system("clear")==-1)
		errExit("system clear");
	printf("===============Welcome to Small Terminal=================\n");
	printf("> Basic Operations:\n");
	printf("> -c: enter client mode, for example: ./terminal -c\n");
	printf("> -s: enter server mode, for example: ./terminal -s\n");
	printf("=========================================================\n");
	if(m==1){
	printf("\n> >>>>>>>>>>>>>\n");
	printf("> Server Usage:\n");
	printf("> >>>>>>>>>>>>>\n");
	printf("> All commands: list, cancel, kill, exit, help\n\n");
        printf("> list: list all clients' PID, For example: list\n");
        printf("> cancel <pid>: cancel the current cammand of <PID>. For example: cancel 123\n");
        printf("> kill <pid>: kill the client <PID> For example: kill 123\n");
	printf("> exit: exit this server and force all clients to terminate.\n");
	printf("> help: helping information\n");
	printf("\n> NOTICE: when using cancel and kill commands, you should specify a pid behind these comands\n");
	printf(">         Otherwise, they will be treated as invalid commands.\n");
	printf(">         Here's some valid commands: cancel 1234, kill 1234.\n");
	}
	if(n==1){
	printf("\n> >>>>>>>>>>>>>\n");
	printf("> Client Usage:\n");
	printf("> >>>>>>>>>>>>>\n");
	printf("> All commands: exit\n\n");
	printf("> exit: exit this client \n"); 
	}
	printf("=========================================================\n");
	printf("		Input Your Command Below:\n");
	printf("=========================================================\n"); 
}

int is_cancel_cmd(char* buf){
	if(strlen(buf)<=7)
		return -1;
	char cmd[10];
	memset(cmd, '\0', 10);
	strncpy(cmd, buf, 7);
	if(strcmp(cmd, "cancel ")!=0)
		return -1;
	char pid[30];
	memset(pid, '\0', 30);
	int i=0; 
	for(i=7; i<strlen(buf); i++)
		pid[i-7] = buf[i];
	return atoi(pid); 
}

int is_kill_client(char* buf){
	if(strlen(buf)<=5)
                return -1;
        char cmd[10];
        memset(cmd, '\0', 10);
        strncpy(cmd, buf, 5);
        if(strcmp(cmd, "kill ")!=0)
                return -1;
        char pid[30];
        memset(pid, '\0', 30);
        int i=0;
        for(i=5; i<strlen(buf); i++)
                pid[i-5] = buf[i];
        return atoi(pid);
}

int main(int argc, char** argv){
	if(argc<2){
		print_usage(0);
		return 0;
	}
	extern char* optarg;
	int opt;
	int mode = -1;		// 1 server, 2 client, 
	while((opt=getopt(argc, argv, "hcs"))!=-1){
		switch(opt){
			case 'c':
				mode = 2;
				break;
			case 's':
				mode = 1;
				break;
			case 'h':
				print_usage(0);
				return 0;	
			default:
				print_usage(0);
				return 0;
		}
	}
	if(mode==0){
		printf("help\n");
	}else if(mode==1){
		print_usage(1);
		char buf[1024];
		console_t* console = malloc(sizeof(console_t));
		if(init_console(console)==-1)
			return -1;
		listen_console(console);
		for(;;){
			printf("> ");
			memset(buf, '\0', 1024);
			fgets(buf, 1024, stdin);
			if(strcmp(buf, "help\n")==0){
				//print_usage(1);
       				printf("> All commands: list, cancel, kill, exit, help\n\n");
			        printf("> list: list all clients' PID, For example: list\n");
			        printf("> cancel <pid>: cancel the current cammand of <PID>. For example: cancel 123\n");
			        printf("> kill <pid>: kill the client <PID> For example: kill 123\n");
			        printf("> exit: exit this server and force all clients to terminate.\n");
			        printf("> help: helping information\n");

			}else if(strcmp(buf, "exit\n")==0){
				kill_all_console(console);
				destroy_console(console);
				break;	
			}else if(strcmp(buf, "list\n")==0){
				list_console(console);	
			}else if(is_cancel_cmd(buf)!=-1){
				cancel_cmd_console(console, is_cancel_cmd(buf));			
			}else if(is_kill_client(buf)!=-1){
				kill_client_console(console, is_kill_client(buf));
			}else{
				printf("  Invalid Command: %s  Use help command to see the manual page.\n", buf);
			}	
		}	

	}else if(mode==2){
		print_usage(2);
		int fd_info = open(CONSOLE_INFO, O_RDONLY);
		if(fd_info==-1 && errno==ENOENT){
			printf("Server is not existed!\nPlease run ./terminal -s\n");
			return 0;
		}
		client_t* client = malloc(sizeof(client_t));
		char buf[20];
		memset(buf, '\0', 20);
		snprintf(buf, 20, "%d", getpid());
		int fd;
		if((fd=open(CONSOLE_FIFO, O_WRONLY))==-1)
			errExit("open fifo");
		if(write(fd, buf, 20)==-1)
			errExit("write fifo");	
		if(close(fd)==-1)
			errExit("close fifo");
		init_client(client);
		work_client(client);

	}
	return 0;
}















