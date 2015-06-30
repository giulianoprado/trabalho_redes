// Importa o cabeçalho com as definições das estruturas que definem o TAD
// bem com as funções/bibliotecas necessárias
#include "contacts.h"

// Inicializa uma lista vazia. Começo e fim não apontam para nada e
// o tamanho é 0
void initializeList(infoList *list){
	list->begin = NULL;
	list->end = NULL;
	list->size = 0;
}

// Busca por um contato que tenha a chave key como nome
// Retorna o ponteiro para o contato achado ou NULL caso
// não haja nenhum contato com nome key
contact* searchContact(char* key, infoList* list){
	int j;
	contact* temp = list->begin;

	for(j = 0; j < list->size; j++){
		if(strcmp(temp->name,key) == 0 && temp->isValid) {
			return temp;
		}
		temp = temp->next;
	}

	return NULL;
}

// Insere um novo contato no final da lista desde que não haja algum
// contato com o mesmo nome. A função retorna NULL no caso de haver.
// A função também retorna NULL se houver falha na alocação de memória
contact* insertContact(char* name, infoList *list, Socket sock){
	contact* newContact;

	if((newContact = (contact*)calloc(1,sizeof(contact))) == NULL)
		return NULL;
	if(searchContact(name,list) != NULL)
		return NULL;

	strcpy(newContact->name,name);
	newContact->Sock = sock;
	newContact->messages = (messageList *) calloc(1,sizeof(messageList));
	initializeMessages(newContact->messages);
	newContact->isValid = 1;
	newContact->isThreadFinished = 1;

	if(list->size == 0){
		list->begin = list->end = newContact;
		list->size++;
		newContact->previous = newContact->next = NULL;
	}
	else{
		newContact->previous = list->end;
		newContact->next = NULL;
		list->end->next = newContact;
		list->end = newContact;
		list->size++;
	}
	return newContact;
}

// Apaga um contato da lista de contatos
// Previamente a função deleteContact, a função seachContact deve
// ser usada para que não haja acessos errados à memória. A função
// deleteContact só deve ser chamada caso o contato exista.
void deleteContact(infoList* list, contact* target){

	contact* temp;
	if(list->size == 1){
		close(target->Sock);
		if (isMessageListFull(target->messages))
			emptyMessages(target->messages);
		free(target->messages);
		free(target);
		list->begin = list->end = NULL;
		list->size = 0;
		return;
	}

	if(target == list->begin){
		close(target->Sock);
		temp = list->begin->next;
		if (isMessageListFull(target->messages))
			emptyMessages(target->messages);
		free(target->messages);
		free(list->begin);
		list->begin = temp;
		list->begin->previous = NULL;
	}
	else if(target == list->end) {
		close(target->Sock);
		temp = list->end->previous;
		if (isMessageListFull(target->messages))
			emptyMessages(target->messages);
		free(target->messages);
		free(list->end);
		list->end = temp;
		list->end->next = NULL;
	}
	else {
		target->previous->next = target->next;
		target->next->previous = target->previous;
		close(target->Sock);
		if(isMessageListFull(target->messages)){
			emptyMessages(target->messages);
		}
		free(target->messages);
		free(target);
	}
	list->size--;
}

// Imprime na tela todos os contatos válidos
// Contatos inválidos são marcados pelas threads de envio e recebimento
// de mensagens. Indicando problemas de timeout ou perda de conexão
void printContacts(infoList* list){
	int j;
	if (list->size == 0){
		fprintf(stdout, "You don't have any contacts yet.\n");
		return;
	}
	contact* temp = list->begin;

	for(j = 0 ; j < list->size ; j++){
		if(temp->isValid){
			printf("Name: %s\n", temp->name);
			temp = temp->next;
		}
	}
}

// Esvazia a lista de contatos
// o ponteiro list não sofre free nesta função! É necessário realizar
// a chamada de free(pointer) fora deste escopo.
void emptyList(infoList* list){
	int j;
	contact* temp = list->begin;

	for(j = 0 ; j < list->size ; j++){
		list->begin = list->begin->next;
		free(temp);
		temp = list->begin;
	}
	list->size = 0;
	list->begin = list->end = NULL;
}