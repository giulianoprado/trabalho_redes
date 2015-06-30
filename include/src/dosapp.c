#include <stdio.h>
#include <stdlib.h>
#include "contacts.h"
#include "messages.h"
#include "lib.c"

#define PASS_SIZE 128
#define PORT_SIZE 5

int portNumber = -1;
int *running;
char closingPassword[PASS_SIZE];
infoList* contactList;
Socket sockServer;
pthread_t varThreadAceitadora;
pthread_t varThreadAck;
pthread_t varThreadEraser;
pthread_mutex_t mutexContact;

void displayMenu();
void readOption(int* target);
int readUserInput(char *string, size_t size);
void serverHandlerRoutine(void *notUsed);
int messageSenderHandler(contact *target);
int broadcastHandler();
void setupData(char *string, int size);
void threadLeitora (contact* contact);
void threadAceitadora (void *args);
void threadAck (void *notUsed);
void threadEraser(void *notUsed);

int main (int argc, char* argv[]){
	char defaultPort[PORT_SIZE+1];
	int option = -1; // flag de menu
	int tempNum = 1;

	// Bloco de variáveis auxiliares para leitura e verificações 	//
	char ip[IP_STRING_SIZE];
	char name[NAME_STRING_SIZE];
	char tempString[NAME_STRING_SIZE];
	int temp;
	pthread_t *threadContact = NULL;
	contact* tempContact;
	Socket tempSock;
	// 								FIM 							//

	// Aloca uma variável para lista de contatos e a inicializa
	contactList = (infoList*)calloc(1,sizeof(infoList));
	initializeList(contactList);

	// Seta running para 1. Indicando que tudo está executando
	running = (int *) calloc(1,sizeof(int));
	*running = 1;

	// Verificacoes iniciais
	if(argc < 2){
		fprintf(stdout,"No port was specified. Default port based on config.cfg\n");
		loadServerConfiguration(closingPassword,defaultPort);
		portNumber = atoi(defaultPort);
	}
	else if (argc > 2){
		fprintf(stderr,"Too many arguments!\nUsage: ./dosapp portNumber\n");
		exit(argc);
	}
	else portNumber = atoi(argv[1]);

	if (portNumber <= 0 || portNumber > 65535){
		fprintf(stderr,"Invalid port number. Bigger than 0. Smaller than 65535\n");
		exit(portNumber);
	}

	// inicializa mutex
	pthread_mutex_init(&mutexContact, NULL);

	// cria socket servidor
	sockServer = criarSocketServidor(portNumber);

	// inicializa as threads de gerenciamento
	pthread_create(&varThreadAceitadora, 0, (void *) threadAceitadora, (void *) 0);
	pthread_create(&varThreadAck, 0, (void *) threadAck, (void *) 0);
	pthread_create(&varThreadEraser, 0, (void *) threadEraser, (void *) 0);

	system("clear");
	fprintf(stdout, "Ohayoo! Welcome to Dos-App!\nType in your username to begin: ");
	while (tempNum){
		readUserInput(myName,NAME_STRING_SIZE);
		if(strcmp(myName,"quit") != 0) {
			tempNum = 0;
		}else fprintf(stdout, "\"quit\" is a reserved word. Choose another name: ");
	}

	system("clear");
	while(option){
		displayMenu();
		readOption(&option);

		// bloco switch que implementa a funcionalidade de menu

		switch(option){
			// insere de contato
			case 1:
				// lê os dados do novo contato
				printf("New name: ");
				readUserInput(name,NAME_STRING_SIZE);
				printf("New IP Adress: ");
				readUserInput(ip,IP_STRING_SIZE);
				system("clear");

				// bloco para verificação de palavra reservada
				if(strcmp(name,"quit") != 0){
					// Checa se os dados são válidos
					if ((tempSock = solicitarConexao(ip,portNumber)) != -1) {
						// Checa se foi possível adicionar o contato à lista
						if ((tempContact = insertContact(name,contactList,tempSock)) != NULL){
							fprintf(stdout,"%s was successfully added to your list!\n", tempContact->name);
							// Rotina para criar uma nova threadLeitora para a nova conexão estabelecida
							threadContact = (pthread_t *) calloc (1,sizeof(pthread_t));
							tempContact->thread = threadContact;
							tempContact->pidThread = pthread_create(threadContact,0,(void *)threadLeitora,(void *)tempContact);
							sleep(1.5);
							// Envia o nome verdadeiro para o destinatário atualizar a lista de contatos
							enviarDados(tempSock,NULL,5,&temp);
						} else fprintf(stderr, "Choose another name. %s is already in your contacts list.\n", name);
					}
					else {
						// mensagens de erro para dados inválidos (host ruim)
						fprintf(stderr, "Dosapp was unable to reach %s whose IP address is %s.\n",name, ip);
						fprintf(stderr, "Contact wasn't added to your list.\n");
					}
				}else fprintf(stderr, "\"quit\" is a reserved word. Choose another name\n");
			break;

			// Listar contatos
			case 2:
				system("clear");
				fprintf(stdout, "***** Contact List *****\n");
				printContacts(contactList);
			break;

			// Apagar algum contato
			case 3:
				printf("Type the Name of one of your contats to delete it.\n");
				scanf("%s", tempString);
				system("clear");

				// verifica se o contato com o nome tempString está na lista de contatos
				if((tempContact = searchContact(tempString,contactList)) == NULL){
					fprintf(stderr, "DosApp couldn't find %s in your contacts list!\n", tempString);
					break;
				}
				else{
					// Marca o contato como inválido para que a threadEraser faça seu trabalho
					tempContact->isValid = 0;
					fprintf(stdout, "%s will be removed from your contacts list\n", tempString);
				}
			break;

			// Envio único de mensagem
			case 4:
				// verifica se a lista não está vazia
				if (contactList->size > 0){
					fprintf(stdout, "Which contact do you want to send a message?\n");
					// lê o contato para o qual o usuário gostaria de enviar mensagem
					readUserInput(tempString,NAME_STRING_SIZE);

					// checa se o contato existe na lista de contato
					tempContact = searchContact(tempString,contactList);
					system("clear");
					if (tempContact != NULL) {
						// Chama o handler de envio de mensagem caso o contato exista
						if (messageSenderHandler(tempContact) < 0){
							fprintf(stderr, "Something went wrong\n");
						}
						else fprintf(stdout, "Message sent to %s!\n", tempString);
					}
					else fprintf(stderr, "Contact not found! Try another.\n");
				}
			break;

			// Broadcast
			case 5:
				// verifica se a lista não está vazia
				if (contactList->size > 0){
					system("clear");
					// Se não estiver, chama o handler de broadcast
					broadcastHandler();
				}
				else fprintf(stdout, "No contacts in your list. Can't sent any message!\n");
			break;

			// Exibir Mensagens Recebidas de X
			case 7:
				// verifica se a lista não está vazia
				if (contactList->size > 0){
					fprintf(stdout, "From which contact?\n");
					readUserInput(tempString,NAME_STRING_SIZE);
					// verifica se o contato requerido pelo usuário está na lista de contatos
					tempContact = searchContact(tempString,contactList);
					system("clear");
					if (tempContact != NULL) {
						// imprime as mensagens recebidas deste contato desde a última visualização
						fprintf(stdout, "***** Messages from %s *****\n", tempString);
						printMessages(tempContact->messages);
					}
					else fprintf(stderr, "Contact not found! Try another.\n");
				}else fprintf(stderr, "No contacts yet\n");

			break;

			// Múltiplos contatos
			case 6:
				// verifica se a lista não está vazia
				if (contactList->size > 0){
					system("clear");
					// chama o handler de envio para múltiplos contatos
					pseudoBroadcastHandler();
					system("clear");
				}
				else fprintf(stdout, "No contacts in your list. Can't sent any message!\n");
			break;

			// Sair
			case 8:
				// Sai do loop while setando option para 0
				fprintf(stdout, "Thanks for using. Cya!\n");
				option = 0;
			break;
		}
	}
	*running = 0;

	emptyList(contactList);
	free(contactList);
	return 0;
}

/*		Contém todas as frases para exibição do menu principal */
void displayMenu(){
	fprintf(stdout, "Welcome %s. What you want to do?\n", myName);
	fprintf(stdout,"1. Insert new contact\n2. Display existing contact(s)\n3. Delete one contact\n");
	fprintf(stdout,"4. Send a message to one contact\n5. Send an message to all contacts\n6. Send a message to selected contacts\n");
	fprintf(stdout,"7. Display Received Messages\n8. Exit\n");
}

/* 		Interage com o usuário para que possa escolher alguma
 *	dos serviços oferecidos pelo menu principal
 *		*** Argumentos ***
 *	>> target: Lugar onde será armazenado o valor inteiro após a leitura
 *	correta da entrada do usuário
 */
void readOption(int *target){

	char temp[2];
	char aux;

	printf("Option: ");

	scanf("%s", &temp[0]);
	*target = atoi(temp);
	scanf("%c", &aux);

	while(*target < 1 || *target > 8){
		printf("Option must be a integer from 1 to 8\nType again: ");
		scanf("%s", &temp[0]);
		*target = atoi(temp);
		scanf("%c", &aux);
	}
}

/*  Lê os dados de entrada do usuário */
int readUserInput(char *string, size_t size){

	int iterator = 0;
	int temp;
	int count = 0;
	char temp2;

	while(iterator < size){
		temp = scanf("%c", &temp2);
		count = count + temp;
		if (temp == 0){
			string[iterator] = '\0';
			iterator = size;
		}
		else if (temp2 == '\n'){
				string[iterator] = '\0';
				iterator = size;
			 }
			 else string[iterator++] = temp2;
	}
	fflush(stdin);

	return count;
}

/*		Rotina responsável por enviar mensagens separadamente. */
int messageSenderHandler(contact *target){

	char *buffer = (char *)calloc(MSG_SIZE,sizeof(char));
	pid_t pid;
	pid_t pidTemp;
	int temp;
	int state;

	pid = fork();
	//Father process instrucion
	if (pid > 0){
		// Espera pelo término do processo filho
		pidTemp = wait(&state);
	}
	//Child process instructions
	if (pid == 0){
		// Mensagens de instrução ao usuário para que ele insira a mensagem à ser enviada
		// A mensagem é armazenada no buffer
		fprintf(stdout, "Type your message:\n");
		readUserInput(buffer,MSG_SIZE);
		strcat(buffer,"\n");

		// Envia a mensagem lida para o contato previamente escolhido pelo usuário
		// O tratamento de timeout e EPIPE é feito nesse contexto também
		enviarDados(target->Sock,buffer,1,&temp);
		if (temp == EAGAIN || temp == EWOULDBLOCK || temp == EPIPE) {
			target->isValid = 0;
		}
		// Encerra o processo filho criado pelo fork
		exit(0);
	}
	//Fork failed
	if (pid < 0){
		fprintf(stderr, "UNKNOWN ERROR (FAILED TO SEND MESSAGE)\nPlease, try again.\n");
		free(buffer);
		return pid;
	}

	// liebra o buffer utilizado
	free(buffer);
	return 1;
}

int broadcastHandler(){
	char *buffer = (char *)calloc(MSG_SIZE,sizeof(char));
	pid_t pid;
	pid_t pidTemp;
	int temp;
	int state;
	contact *tempContact = contactList->begin;
	contact *aux;

	pid = fork();
	//Father process instrucion
	if (pid > 0){
		// Espera pelo término do processo filho
		pidTemp = wait(&state);
	}
	//Child process instructions
	if (pid == 0){
		// Mensagens de instrução ao usuário para que ele insira a mensagem à ser enviada
		// A mensagem é armazenada no buffer
		fprintf(stdout, "Type your message:\n");
		readUserInput(buffer,MSG_SIZE);
		strcat(buffer,"\n");

		// Itera sobre toda a lista de contato enviando a mensagem para cada um deles
		// É feito o tratamento para erros de timeout e EPIPE.
		while(tempContact != NULL){
			if (tempContact->isValid){
				enviarDados(tempContact->Sock,buffer,1,&temp);
				if (temp == EAGAIN || temp == EWOULDBLOCK || temp == EPIPE) {
					tempContact->isValid = 0;
				}
				tempContact = tempContact->next;
			}
			else tempContact = tempContact->next;
		}
		// Encerra o processo filho criado pelo fork
		exit(0);
	}
	//Fork failed
	if (pid < 0){
		fprintf(stderr, "UNKNOWN ERROR (FAILED TO SEND MESSAGE)\nPlease, try again.\n");
		free(buffer);
		return pid;
	}

	// libera o buffer utilizado
	free(buffer);
	return 1;
}

int pseudoBroadcastHandler(){
	char *buffer = (char *)calloc(MSG_SIZE,sizeof(char));
	char *buffer2 = (char *)calloc(NAME_STRING_SIZE,sizeof(char));
	pid_t pid;
	pid_t pidTemp;
	int temp;
	int state;
	int i = 0;
	contact *tempContact = NULL;

	pid = fork();
	//Father process instrucion
	if (pid > 0){
		// Espera pelo término do processo filho
		pidTemp = wait(&state);
	}
	//Child process instructions
	if (pid == 0){
		// Mensagens de instrução ao usuário para que ele insira a mensagem à ser enviada
		// A mensagem é armazenada no buffer
		fprintf(stdout, "Type your message:\n");
		readUserInput(buffer,MSG_SIZE);
		strcat(buffer,"\n");

		// Mostra a lista de contatos para o usuário
		fprintf(stdout, "***** Contact List *****\n");
		printContacts(contactList);

		// Pergunta à quais contatos o usuário deseja enviar a mensagem
		fprintf(stdout, "To whom would you like to send the message above? Type \"quit\" to leave\n");

		// Itera recebendo o nome do contato requisitado e envia a mensagem em seguida
		// É feito o tratamento para erros de timeout e EPIPE e além disso é possível
		// que o usuário digite "quit" dizendo que terminou o envio. O número máximo de
		// mensagens enviadas nesse modo é o número de contatos
		while(i < contactList->size){
			fprintf(stdout, "Contact number %d: ", i++ + 1);
			readUserInput(buffer2,NAME_STRING_SIZE);
			if(strcmp(buffer2,"quit") == 0) break;

			tempContact = searchContact(buffer2,contactList);
			if (tempContact != NULL && tempContact->isValid) {
				enviarDados(tempContact->Sock,buffer,1,&temp);
				if (temp == EAGAIN || temp == EWOULDBLOCK || temp == EPIPE) {
					tempContact->isValid = 0;
				}
			}
			else fprintf(stderr, "Contact not found! Try another.\n");
		}
		// Fecha o processo filho criado pelo fork
		exit(0);
	}
	//Fork failed
	if (pid < 0){
		fprintf(stderr, "UNKNOWN ERROR (FAILED TO SEND MESSAGE)\nPlease, try again.\n");
		free(buffer);
		return pid;
	}

	// libera os buffers alocados
	free(buffer);
	free(buffer2);
	return 1;
}

/*		Rotina que fica aguardando por novos dados em uma conexão TCP.
 *	As mensagens possíveis de se receber são:
 *	>> Dados: O comportamento é imprimir o remetente e a mensagem recebida.
 *	>> Ack: O comportamento é nenhum. O socket está baseado em um time-out
 *	pré-estabelecido de 30s, então é necessário o recebimento do Ack para que a
 *	thread não determine que a conexão está morta.
 *	>> Informativo: Quando uma conexão é recebida, o "servidor" não sabe o nome
 *	do requisitante. O requisitante então deve enviar uma mensagem com o código
 *	5 seguido de <>  seguido de seu nome.  *** 5<>nome. ***
 */
void threadLeitora (contact* contact) {
	char *tempMsg;
	char finalMsg[MSG_SIZE];
	int temp;
	int i;

	while (contact->isValid) {
		tempMsg = receberDados(contact->Sock, &temp);

		// Código 5 indica que é uma mensagem de atualização de informação
		if(tempMsg[0] == '5'){
			// Zera o buffers
			bzero(contact->name,NAME_STRING_SIZE);

			// Armazena o nome verdadeiro na estrutura do contato
			for(i = 0 ; tempMsg[i+3] != '\n' ; i++){
				contact->name[i]=tempMsg[i+3];
			}
		}
		// Código 1 indica que uma mensagem de dados foi recebida
		else if (tempMsg[0] == '1'){
			for(i = 0 ; tempMsg [i+3] != '\n' ; i++){
				finalMsg[i] = tempMsg[i+3];
			}
			finalMsg[i] = '\0';
			// Mensagem é armazenada na lista de mensagens do contato
			insertMessage(finalMsg,contact->messages);
		}
		else if (tempMsg[0] == '3'){
			// Não implementado ainda a opção de requisitar nome verdadeiro...
			// Se receber uma requisição 3, envia o nome verdadeiro.
			enviarDados(contact->Sock,NULL,5,&temp);
		}
		free(tempMsg);

		// Se exceder o Timeout ou tentar acessar uma conexão inválida
		// o contato será inválidado
		if (temp == EAGAIN || temp == EWOULDBLOCK || temp == EPIPE) {
			contact->isValid = 0;
		}
	}
	// Sinaliza que a threadLeitora foi finalizada com sucesso
	contact->isThreadFinished = 1;
	pthread_exit(0);
}

/*		Rotina que implementa a parte servidor do DosApp.
 *		Fica aguardando por conexões e cria uma nova thread quando uma nova conexão
 *	é aceita. Essa nova thread é responsável por ficar esperando dados da conexão TCP esta-
 *	-belecida. As mensagens possíveis podem ser dados, Ack ou informativo de nome.
 */
void threadAceitadora (void* notUsed) {
	pthread_t *tempThread = NULL;
	contact *tempContact = NULL;

	while (*running) {

		// Aguarda uma conexão e recebe o novo socket criado em sockCriado
		Socket sockCriado = recebeConexao(sockServer);
		if (sockCriado > 0){

			// Se o socket foi criado com sucesso, aloca um ponteiro para uma thread
			tempThread = (pthread_t *) calloc(1,sizeof(pthread_t));
			// Insere o contato ainda desconhecido na lista
			tempContact = insertContact("UNKNOWN",contactList, sockCriado);
			// Coloca o ponteiro na estrutura do contato
			tempContact->thread = tempThread;
			// Cria a threadLeitora da nova conexão
			tempContact->pidThread = pthread_create(tempThread, 0, (void *) threadLeitora, (void *) tempContact);
		}
	}
	pthread_exit(0);
}

/*		Rotina responsável por manter conexões ativas. O programa interpreta uma
 *	conexão morta caso não receba um ack por 20 segundos. a threadAck envia mensagens
 *	para todos os contatos com um intervalo de aproximadamente 3 segundos para sinalizar
 *	uma conexão ainda ativa.
 */
void threadAck(void *notUsed){

	contact *tempContact;
	contact *aux;
	int temp;

	while(*running){

		// Adquire a posse da região crítica para mandar Ack tranquilamente para todos
		// os contatos válidos na lista de contatos
		pthread_mutex_lock(&mutexContact);
		tempContact = contactList->begin;

		// Itera até o final da lista de contatos
		while(tempContact != NULL){

			// Verifica se o contato é válido
			if(tempContact->isValid){

				// Envia ack
				enviarDados(tempContact->Sock,NULL,2,&temp);

				// Caso algo dê errado com o envio do Ack (timeout ou EPIPE), marca
				// o contato como inválido para ser apagado
				if (temp == EAGAIN || temp == EWOULDBLOCK || temp == EPIPE){
					tempContact->isValid = 0;
				}
			}
			// Aponta para o próximo da lista e continua a iteração
			tempContact = tempContact -> next;
		}
		// Abre mão da região crítica
		pthread_mutex_unlock(&mutexContact);

	// Dorme por três segundos
		sleep(3);
	}
	pthread_exit(0);
}


/*		A threadEraser é responsável por apagar da lista de contato
 *	todos àqueles que estiverem marcados como inválidos por motivos
 *	já no arquivo lib.c
 *
 *		Para que a remoção aconteça com coerência, o comando "deleteContact"
 *	é envolvido por um mutex para que não interfira no funcionamento da
 *	threadAck.
 */
void threadEraser (void *notUsed){
	contact *tempContact;
	contact *aux;
	pthread_t *tempThread;

	// Somente sairá do loop while quando o usuário optar por finalizar DosApp
	while(*running){

		tempContact = contactList->begin;

		// Loop while para iterar sobre a lista toda
		while(tempContact != NULL){

			// verifica se o contato está válido.
			if (!tempContact->isValid){

				// Aguarda até que a threadLeitora do contato termine
				while(!tempContact->isThreadFinished){
					sleep(1);
				}

				// Libera o ponteiro da thread alocado na threadAceitadora
				tempThread = tempContact->thread;
				free(tempThread);

				// Aponta para o próximo na lista
				aux = tempContact->next;

				// Apaga o contato da lista caso adquira posse da região crítica
				pthread_mutex_lock(&mutexContact);
				deleteContact(contactList,tempContact);
				pthread_mutex_unlock(&mutexContact);
				
			} else aux = tempContact->next;

			// atribui o pŕoximo da lista à tempContact para dar continuidade
			// na iteração
			tempContact = aux;
		}
		// Dorme por 5 segundos
		sleep(5);
	}
	pthread_exit(0);
}