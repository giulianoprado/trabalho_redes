#ifndef _CONTACTS_
#define _CONTACTS_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contacts.h"

void initializeList(infoList *list){
	list->begin = NULL;
	list->end = NULL;
	list->size = 0;
}

contact* searchContact(char* key, infoList* list){
	int end, j;
	contact* temp = list->begin;

	for(j = 0; j < list->size; j++){
		if(strcmp(temp->name,key) == 0 || strcmp(temp->ip,key) == 0) {
			return temp;
		}
		temp = temp->next;
	}

	return NULL;
}

/* Inserts a new contact in list's tail */
int insertContact(char* ip, char* name, infoList *list){
	contact* newContact;

	if((newContact = (contact*)calloc(1,sizeof(contact))) == NULL)
		return -1;
	if(searchContact(ip,list) != NULL || searchContact(name,list) != NULL)
		return -2;

	strcpy(newContact->ip,ip);
	strcpy(newContact->name,name);

	if(list->size == 0){
		list->begin = list->end = newContact;
		list->size++;
	}
	else{
		newContact->previous = list->end;
		newContact->next = NULL;
		list->end->next = newContact;
		list->end = newContact;
		list->size++;
	}

	return 0;
}

void deleteContact(infoList* list, contact* target){

	contact* temp;
	if(list->size == 1){
		free(target);
		list->begin = list->end = NULL;
		list->size--;
		return;
	}

	if(target == list->begin){
		temp = list->begin->next;
		free(list->begin);
		list->begin = temp;
		list->begin->previous = NULL;
	}
	else if(target == list->end) {
		temp = list->end->previous;
		free(list->end);
		list->end = temp;
		list->end->next = NULL;
	}
	else {
		target->previous->next = target->next;
		target->next->previous = target->previous;
		free(target);
	}
	list->size--;
}

void printContacts(infoList* list){
	int j;
	contact* temp = list->begin;

	for(j = 0 ; j < list->size ; j++){
		printf("Name: %s\tIP Adress: %s\n", temp->name, temp->ip);
		temp = temp->next;
	}
}

void emptyList(infoList* list){
	int j;
	contact* temp = list->begin;

	for(j = 0 ; j < list->size ; j++){
		list->begin = list->begin->next;
		free(temp);
		temp = list->begin;
	}
	list->begin = list->end = NULL;
}
#endif