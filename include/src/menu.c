#include <stdio.h>
#include <stdlib.h>
#include "contacts.c"

void displayMenu();
void readOption(int* target);

int main (int argc, char* argv[]){

	int option = -1;
	char ip[17];
	char name[30];
	char tempString[30];
	contact* tempContact; 
	infoList* list = (infoList*)calloc(1,sizeof(infoList));
	initializeList(list);

	while(option){
		displayMenu();
		printf("Option: ");
		readOption(&option);

		switch(option){
			case 1:
				printf("New name: "); scanf("%s", name);
				printf("New IP Adress: "); scanf("%s", ip);
				insertContact(ip,name,list);
			break;

			case 2:
				printContacts(list);
			break;

			case 3:
				printf("Type the IP Adress or Name of one of your contats to delete it.\n");
				scanf("%s", tempString);
				if((tempContact = searchContact(tempString,list)) == NULL)
					break;
				else deleteContact(list,tempContact);
			break;

			case 4:
			break;

			case 5:
			break;

			case 6:
				printf("Thanks for using. Cya!\n");
				option = 0;
			break;
		}
	}

	emptyList(list);
	free(list);
	return 0;
}

void displayMenu(){
	printf("1. Insert new contact\n2. Display existing contact(s)\n3. Delete one contact\n4. Send a message to one contact\n5. Send an message to all contacts\n6. Exit\n");
}

void readOption(int* target){

	scanf("%d", target);
	while(*target < 1 || *target > 6){
		printf("Option must be a integer from 1 to 6!\nType again: ");
		scanf("%d", target);
	}
}