/* The project does not depend on this file as expected when I was designing the whole project. 
 * But other files may use some functions defined in this file.
 *
 */
#ifndef LIST_HEADER
#define LIST_HEADER

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define NODE_SIZE 1024

void errExit(char* msg);

typedef struct Node{
	char str[NODE_SIZE];
	struct Node* next;
} Node_t;

typedef struct List{
	int size;
	struct Node* head;
	struct Node* tail;
	pthread_mutex_t mtx;
	pthread_cond_t cond;

} List_t;

int push_tail(List_t* list, char* val){
	int s;
	if((s=pthread_mutex_lock(&(list->mtx)))!=0)
		errExit("pthread_mutex_lock");
	Node_t* n = malloc(sizeof(Node_t));
	n->next = NULL;
	strcpy(n->str, val);
	if(list->tail!=NULL){
		list->tail->next = n;
		list->tail = list->tail->next;
	}
	else{
		list->head = n;
		list->tail = n;
	}	
	list->size++;
	if((s=pthread_mutex_unlock(&(list->mtx)))!=0)
		errExit("pthread_mutex_unlock");
	if((s=pthread_cond_signal(&(list->cond)))!=0)
		errExit("pthread_cond_signal");
	return 0;
}

void pop_head(List_t* list, char* val){
	int s;
	if((s=pthread_mutex_lock(&(list->mtx)))!=0)
		errExit("pthread_mutex_lock");
	while(list->size==0)
		if((s=pthread_cond_wait(&(list->cond), &(list->mtx)))!=0)	
			errExit("pthread_cond_wait");	
	strcpy(val, list->head->str);
	Node_t* tmp = list->head;
	list->head = list->head->next;
	if(list->head==NULL)
		list->tail = NULL;
	free(tmp);
	list->size--;
	if((s=pthread_mutex_unlock(&(list->mtx)))!=0)
		errExit("pthread_mutex_unlock");
}

List_t* list_init(){
	int s;
	List_t* tmp = malloc(sizeof(List_t));
	tmp->size = 0;
	tmp->head = NULL;
	tmp->tail = NULL;	
	if((s=pthread_mutex_init(&(tmp->mtx), NULL))!=0)
		errExit("pthread mutex init");
	if((s=pthread_cond_init(&(tmp->cond), NULL))!=0)
		errExit("init cond");
	return tmp;
}

void list_destroy(List_t* list){
	int s;
	if(s=pthread_mutex_destroy(&(list->mtx))!=0)
		errExit("mtx destroy");
	if(s=pthread_cond_destroy(&(list->cond))!=0)
		errExit("cond destroy");
	Node_t* tmp = list->head->next;
	while(list->head){
		free(list->head);
		list->head = tmp;
		tmp =list->head->next;
	}
	free(list);
}		

void errExit(char* msg){
	perror(msg);
	exit(EXIT_FAILURE);
}	

#endif
