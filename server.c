
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
char buffer[256], protclstr[700];
int status;
//Variables del socket
struct sockaddr_in serv_addr, cli_addr;
int n,nu=1,i;

struct user {
	char *username;
	char *ip;
	char *port;
	char *status;
	int socket;
};

int usercount = 0;

struct user users[100];

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
		error("Error! Especifique un Port!\n");
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
	bzero(protclstr,150);
	
	if ( recv(newsockfd, protclstr, 149, 0) < 0) {
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
		int k;
		char *username = tokens[1];
		bzero(buffer, 256);
		snprintf(buffer,sizeof(buffer), "01|%s|127.0.0.1", username);
		
		for (k=0;k<usercount;k++){
			if(strcmp(users[k].username, tokens[1]) == 0){
				send(newsockfd, buffer, 255, 0);
				goto checking;
			} else if (k+1 == usercount) {
					users[usercount].username = tokens[1];
					users[usercount].ip = tokens[2];
					users[usercount].port = tokens[3];
					users[usercount].status = tokens[4];
					users[usercount].socket = newsockfd;
					usercount++;
					pthread_mutex_unlock(&mutex);
					goto checking;
					
					
			}
		}
		
	} else if ( strcmp(tokens[0],"02") == 0) {
		//Logica de liberacion de usuario
		int k, counter;
		char *username = tokens[1];
		for (k=0;k<usercount;k++){
			if(strcmp(users[k].username, username) == 0){
				for(counter = k; counter < usercount-1; counter ++){
					users[counter] = users[counter+1];
				}
			}
		}
		goto END;
	} else if ( strcmp(tokens[0],"03") == 0) {
		//Logica de cambio de estado
		int k;
		char *username = tokens[1];
		char *status = tokens[2];
		for (k=0;k<usercount;k++){
			if(strcmp(users[k].username, username) == 0){
				users[k].status = status;
				goto checking;
			}
		}
	} else if ( strcmp(tokens[0],"04") == 0) {
		//Logica de peticion de informacion
		int k;
		char *userDeInfo = tokens[1];
		char *username = tokens[2];
		for (k=0;k<usercount;k++){
			if(strcmp(users[k].username, userDeInfo) == 0){
				char *userDeInfoPort = users[k].port;
				char *userDeInfoIP = users[k].ip;
				char *userDeInfoStatus = users[k].status;
				
				bzero(buffer, 256);
				snprintf(buffer,sizeof(buffer), "05|%s|%s|%s|%s", userDeInfo,userDeInfoIP, userDeInfoPort, userDeInfoStatus);
				send(newsockfd, buffer, 255, 0);
				goto checking;
			}
		}
		
	} else if ( strcmp(tokens[0],"06") == 0) {
		//Logica de solicitud de listado de usuarios
		int k;
		char newBuffer[1024];
		char *username = tokens[1];
		bzero(newBuffer, 1024);
		snprintf(newBuffer, sizeof(newBuffer), "07|%s|", username);
		for (k=0;k<usercount;k++){
			strncat(newBuffer,users[k].username, sizeof(newBuffer));
			strncat(newBuffer,"+", sizeof(newBuffer));
			strncat(newBuffer,users[k].status, sizeof(newBuffer));
			strncat(newBuffer,"&", sizeof(newBuffer)); 
		}
		send(newsockfd, newBuffer, 1023, 0);
		goto checking;
	} else if ( strcmp(tokens[0],"08") == 0) {
		//Logica de envio de mensajes 
		char *username = tokens[1];
		char *target = tokens[2];
		char *message = tokens[3];
		int targetsockfd, k;
		
		bzero(buffer, 256);

		n=recv(newsockfd, buffer, 255,0);
		if (n<0){
			error("ERROR! Leyendo del Socket");
			goto END;
		}
		for (k=0;k<usercount;k++){
			if(strcmp(users[k].username, target) == 0){
				targetsockfd = users[k].socket;
				snprintf(buffer, sizeof(buffer), "08|%s|%s|%s", username, target, message);
				send(targetsockfd, buffer, 255, 0);
				goto checking;
					
			}
			
		}	
	} else {
		puts("WTF you didnt respect the protocol dickhead");
	}
	END:
	free(tokens);
	close(newsockfd);
	pthread_exit(NULL);	
}	
