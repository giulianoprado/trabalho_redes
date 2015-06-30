#ifndef _CONTACTS_
#define _CONTACTS_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include "messages.h"

// Comodidades.
typedef char name_string;
typedef char ip_string;
typedef int Socket;

typedef struct element{
	name_string name[64];  	// Nome do Contato
	Socket Sock;			// Socket da conẽxão ativa do contato
	messageList *messages;	// Lista de mensagens
	pthread_t *thread;		// Ponteiro para a threadLeitora
	int pidThread;			// pid da threadLeitora (Não foi usado)
	int isValid;			// Marca de contato válido
	int isThreadFinished;	// Indica que a threadLeitora foi finalizada
	struct element *previous;
	struct element *next;
}element;

typedef struct infoList{
	element *begin;
	element *end;
	int size;
}infoList;

typedef element contact;

void initializeList(infoList *list);
contact* searchContact(char* key, infoList* list);
contact* insertContact(char* name, infoList *list, Socket sock);
void deleteContact(infoList* list, contact* target);
void printContacts(infoList* list);
void emptyList(infoList* list);
#endif