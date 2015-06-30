#ifndef _MESSAGES_
#define _MESSAGES_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char name_string;
typedef char ip_string;
typedef int Socket;

typedef struct element2{
	char text[1024];
	struct element2 *next;
}element2;

typedef struct messageList{
	element2 *begin;
	element2 *end;
	int size;
}messageList;

typedef element2 message;

void initializeMessages(messageList *list);
message* insertMessage(char* received, messageList *list);
int isMessageListFull(messageList *list);
void printMessages(messageList * list);
void emptyMessages(messageList* list);

#endif