
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>



//Variables del Thread
pthread_t thread;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//Variables del mensaje
char buffer[256], user[100], protclstr[700];
int status;
//Variables del socket
struct sockaddr_in serv_addr, cli_addr;
int n,nu=1,i;

struct user {
	char *username;
	char *ip;
	char *port;
	char *status;
}

int usercount;

struct users[50];

void error(char *msg)
{
	perror(msg);
	exit(0);
}

void* server(void*);


int main(int argc, char *argv[])
{ 
    int i, sockfd, newsockfd[1000], portno, clilen, no=0,n;
    if (argc<2) { 
		fprintf(stderr, "Error! Especifique un Port!\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd<0) {
		error("ERROR, No se puede abrir el socket");
	}
	bzero((char*) &serv_addr, sizeof(serv_addr));
	
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0)
		error("ERROR! No se puede hacer el Binding del Socket");
	
	listen(sockfd,5);
	
	clilen= sizeof(cli_addr);
	
	
	while(1)
	{
		newsockfd[no] = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
		
		if (newsockfd<0)
			error("ERROR! Aceptando la conexion");
		//CREA THREADS PARA CADA INCOMING CONECTION
		pthread_mutex_lock(&mutex);
		pthread_create(&thread, NULL, server,(void*)&newsockfd[no]);
		no+=1;
	}
	
	close(newsockfd[no]);
	close(sockfd);
	return 0;
	
}

//LOGICA DEL SERVIDOR
void* server(void* sock)
{
	int newsockfd=*((int*)sock),j=0,m;
	char to[100], from[100], name[100];
	
	checking:
	m=1;
	bzero(user,100);
	bzero(to, 100);
	bzero(from, 100);
	bzero(protclstr,150);
	
	if ( recv(newsockfd, protclstr, 150, 0) < 0) {
		error("ERROR! Fallo al recibir el mensaje");
	}
	
	char ** tokens = NULL;
	char * p = strtok(protclstr, "|");
	int n_spaces = 0, i;
	while (p) {
		tokens = realloc(tokens , sizeof(char*) * ++n_spaces);
		if (tokens == NULL)
			exit(-1);
		
		tokens[n_spaces-1] = p;
		p = strtok(NULL, "|");
	}
		
	
	if (strcmp(tokens[0],"00") == 0) {
		//Logica de registro de usuario
		char *username = tokens[1];
		char *ip = tokens[2];
		char *port = tokens[3];
		char *status = tokens[4];

		
	} else if ( strcmp(tokens[0],"02") == 0) {
		//Logica de liberacion de usuario
		char *username = tokens[1];
	} else if ( strcmp(tokens[0],"03") == 0) {
		//Logica de cambio de estado
		char *username = tokens[1];
		char *status = tokens[2];
	} else if ( strcmp(tokens[0],"04") == 0) {
		//Logica de peticion de informacion
		char *userDeInfo = tokens[1];
		char *username = tokens[2];
	} else if ( strcmp(tokens[0],"06") == 0) {
		//Logica de solicitud de listado de usuarios
		char *username = tokens[1];
	} else if ( strcmp(tokens[0],"08") == 0) {
		//Logica de envio de mensajes 
		char *username = tokens[1];
		char *target = tokens[2];
		char *message = tokens[3];
	} else {
		puts("WTF you didnt respect the protocol dickhead");
	}

	free(tokens);	
}	
