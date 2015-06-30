/*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*
 *  lib.c 																						*
 *																								*
 *  Created by Giuliano Barbosa Prado and Marcello de Paula Ferreira Costa on 22/05/15.			*
 *  Edited until /06/2015																		*
 *  Copyright (c) 2015 GBPMDPFC Corp. All rights reserved.										*
 *																								*
 *	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*/

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

typedef int Socket;
#define MSG_SIZE 1024
#define NAME_STRING_SIZE 30
#define IP_STRING_SIZE 17

char myName[NAME_STRING_SIZE];
char myIP[IP_STRING_SIZE];

/*		Função usada pela rotina do cliente quando quer adicionar um novo
 *	contato. O Cliente para poder adicionar um novo contato deve primeiro
 *	verificar se o IP fornecido está funcionando. Caso contrário, não será
 *	possível adicionar o alvo em sua lista de contatos
 *	>> serverName: Nome ou endereço IP do alvo
 *	>> portNumber: Porta onde o alvo deve estar aguardando conexões
 */
Socket solicitarConexao(char *serverName, int portNumber){

	Socket sock;
	struct hostent *host;
	struct sockaddr_in serverAddress;
	host = gethostbyname(serverName);

	// geshostbyname seta h_errno caso haja algum erro.
	// Abaixo é feita a verificação
	if (h_errno == HOST_NOT_FOUND || h_errno == TRY_AGAIN){
		return -1;
	}

	// Tenta criar um socket do tipo UDP e sai da função caso alho dê errado
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr,"Socket Creation Failure");
		return sock;
	}

	// Parâmetros da estrutura de endereço para ser usada no connect
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portNumber);
	serverAddress.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(serverAddress.sin_zero),8);

	// Tenta estabelecer uma conexão com o comando accept no servidor requisitado
	if(connect(sock,(struct sockaddr *)&serverAddress,sizeof(struct sockaddr)) == -1){
		fprintf(stderr,"Connection Failure\n");
		return -1;
	}

	// Estrutura do TIMEOUT
	struct timeval tv;

	tv.tv_sec = 15;  // 15 Secs Timeout
	tv.tv_usec = 0;  // Not init'ing this can cause strange errors

	// Coloca a opção de timeout no socket criado previamente
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

	return sock;
}

/*		Função usada pela rotina de montagem da aplicação. É necessário
 *	criar um socket que ficará aguardando conexões novas após o comando
 *	"listen()". A função abaixo faz o encapsulamento das diretivas
 *	necessárias para criação de um socket servidor (bind,listen) e
 *	retorna o socket criado. Retorna -1 em caso de erro.
 *		*** Argumentos ***
 *	>> portNumber: Porta onde a aplicação decidiu aguardar por conexões
 *	(Pode ser do arquivo config.cfg ou passada como argumento via linha
 *	de comando).
 */
Socket criarSocketServidor(int portNumber){

	int temp = 1;
	Socket sock;
	struct sockaddr_in serverAddress;

	// Tenta criar um socket e sai da função retornando o erro em sock
	if ((sock = socket(AF_INET,SOCK_STREAM,0)) < 0){
		fprintf(stderr,"Failed to create socket\n");
		return sock;
	}

	// Atributos da estrutura de socket servidor
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); // "ip da máquina"
	serverAddress.sin_port = htons(portNumber);

	// Coloca a opção de o socket aceitar múltiplas requisições
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &temp,sizeof(int)) == -1){
		fprintf(stderr,"Setsockopt Error");
		return -1;
	}

	// Bind do socket criado com o endereço do servidor
	if (bind(sock,(struct sockaddr *)&serverAddress,(socklen_t)(sizeof(serverAddress))) < 0){
		fprintf(stderr,"Bind Error\n");
		return -1;
	}

	// Coloca o socket em estado de "espera".
	// Fica ouvindo/esperando por novas conexões
	if (listen(sock, 10) == -1){
		fprintf(stderr,"Listen Error\n");
		return -1;
	}

	// Retorna o socket do tipo servidor criado
	return sock;
}

/* 		Função usada pela rotina de servidor. Após o comando "listen()" ter sido
 *	chamado, o servidor fica escutando em um socket já criado aguardando por
 *	novas requisições de conexão.
 *		*** Argumentos ***
 *	>> sock: Socket onde o servidor fica aguardando por novas conexões;
 *	>> clientAdress: Estrutura onde a função "accept" armazenará as
 *	informações do novo socket criado quando uma requisição de conexão
 *	for aceita;
 */
Socket recebeConexao(Socket sock) {
	struct sockaddr_in cliente_endereco;
	// Tamanho do sockaddr_in
	socklen_t sin_size = (socklen_t)sizeof(struct sockaddr_in);

	// A função accept faz o dual com a função connect
	// Aqui é onde a conexão é aceita e um novo socket é criado para a conexão TCP
	// estabelecida
	Socket conectado = accept(sock, (struct sockaddr *)&cliente_endereco, &sin_size);

	// Estrutura de TIMEOUT
	struct timeval tv;

	tv.tv_sec = 15;  // 10 Secs Timeout
	tv.tv_usec = 0;  // Not init'ing this can cause strange errors

	// Coloca a opção de timeout no socket criado previamente
	setsockopt(conectado, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

	return conectado;
}


/*		Tenta receber dados em um socket e retorna a cadeia de caracteres
 *	lida. Caso o valor de bytes lidos seja 0 ou menor, indicando erro,
 *	retorna 0
 *		*** Argumentos ***
 *	>> sock: Socket de recebimento
 */
char *receberDados (Socket sock, int *temp){
	char *outputBuffer = (char *) calloc(MSG_SIZE,sizeof(char));

	//recv seta errno para EPIPE caso haja problema de conexão e também pode
	// colocar errno para valores que indiquem o timeout excedido
	int numReadBytes = (int) recv(sock,outputBuffer,MSG_SIZE,MSG_NOSIGNAL);
	*temp = errno; 	// Copiado para a variável de parâmetro para futura avaliação
					// Isso evita que outra recv modifique a variável errno antes
					// da devida avaliação ser feita
	errno = 0;

	// Garante que haverá '\0' no fim da string
	if(numReadBytes <= 0){
		outputBuffer[0] = '\0';
	}
	else outputBuffer[numReadBytes]='\0';

	// Retorna o buffer alocado dinâmicamente para a função que chamou receberDados
	// É importante que o programador libere a memória alocada após o uso!
	return outputBuffer;
}

/* 		Monta a mensagem à ser enviada de acordo com o tipo escolhido
 *	e envia a cadeia de caracteres formada.
 *		*** Argumentos ***
 *	>> type: Indica o tipo da mensagem à ser enviada (Check/Data/...)
 *	>> sock: Socket de envio.
 */
void enviarDados (Socket sock, char* data, int type, int *temp){

	char *newBuffer = (char *) calloc(MSG_SIZE,sizeof(char));

	switch(type){
		
		// Mensagem que envia o nome verdadeiro ao requisitante
		// Sempre é enviada quando uma nova conexão é aceita
		case 5:
			newBuffer[0] = '5';
			newBuffer[1] = '<';
			newBuffer[2] = '>';
			strcat(newBuffer,myName);
			strcat(newBuffer,"\n");
		break;

		// Mensagem de dados.
		// Código 1<>Mensagemdefato
		case 1:
			newBuffer[0] = '1';
			newBuffer[1] = '<';
			newBuffer[2] = '>';
			strcat(newBuffer,data);
		break;

		// Mensagem de Ack
		// Usada pela threadAck para auxliar no gerenciamento de conexões ativas
		case 2:
			newBuffer[0] = '2';
			newBuffer[1] = '<';
			newBuffer[2] = '>';
			strcat(newBuffer,"ackMessage\n");
		break;

		// Mensagem de requisição de nome
		// Não foi usada no projeto
		case 3:
			newBuffer[0] = '3';
			newBuffer[1] = '<';
			newBuffer[2] = '>';
			strcat(newBuffer,"nameRequest\n");
		break;
	}
	// Assim como a função recv, seta errno para EPIPE e outros valores de erro
	// para futura avaliação
	// send envia a mensagem montada para o destinatário

	//////////////////////////////////////////////////////////////////////////////////
	// A flag MSG_NOSIGNAL é usada para que nem rec nem send gerem o sinal SIGPIPE	//
	// que causa o término do programa. Essa flag permite a avaliação da variável	//
	// EPIPE.																		//
	// Não é portável para OS X														//
	//////////////////////////////////////////////////////////////////////////////////
	send(sock,newBuffer,strlen(newBuffer),MSG_NOSIGNAL);
	*temp = errno;
	errno = 0;
	free(newBuffer);
}

/*		Lê o arquivo config.cfg, que deve estar no mesmo diretório do
 *	executável. Desse arquivo extrai as configurações de porta do servidor
 *	e a senha de fechamento remoto do servidor
 *		*** Argumentos ***
 *	>> string: cadeia de caracteres onde será armazenada a senha de
 *	fechamento remoto
 *	>> portString: cadeia de caracteres onde será armazenado o valor da
 * 	porta do servidor
 */
void loadServerConfiguration(char *string, char *portString){
	FILE *fp;
	int i = 0;

	if (fp = fopen("config.cfg", "r")){
		fseek(fp, (long int)5*sizeof(char), SEEK_SET);
		fscanf(fp,"%s\n",string);

		fseek(fp, (long int)5*sizeof(char), SEEK_CUR);
		fscanf(fp,"%s\n",portString);

		fclose(fp);
	}
}