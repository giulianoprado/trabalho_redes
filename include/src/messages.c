#include "messages.h"

void initializeMessages(messageList *list){
	list->begin = NULL;
	list->end = NULL;
	list->size = 0;
}

/* Inserts a new contact in list's tail */
message* insertMessage(char* received, messageList *list){
	message* newMessage;

	if((newMessage = (message*)calloc(1,sizeof(message))) == NULL)
		return NULL;

	strcpy(newMessage->text, received);

	if(list->size == 0){
		list->begin = list->end = newMessage;
		list->size++;
		newMessage->next = NULL;
	}
	else{
		list->end->next = newMessage;
		newMessage->next = NULL;
		list->end = newMessage;
		list->size++;
	}

	return newMessage;
}

void printMessages(messageList * list){

	message* temp = list->begin;
	while(temp != NULL){
		fprintf(stdout, "%s\n", temp->text);
		temp = temp -> next;
	}

	emptyMessages(list);
}

int isMessageListFull(messageList *list){
	return list->size;
}

void emptyMessages(messageList* list){
	if (list->size == 0){
		fprintf(stdout, "No messages in Inbox\n");
		return;
	}

	message* temp = list->begin;
	message* temp2;

	while(temp != NULL){
		temp2 = temp;
		temp = temp ->next;
		free(temp2);
	}
	list->size = 0;
	list->begin = list->end = NULL;
}