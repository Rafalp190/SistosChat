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


FILE *fp, *tmpfp;


//Variables del Thread
pthread_t thread;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//Variables del mensaje
char buffer[256], protclstr[700];
int status;
//Variables del socket
struct sockaddr_in serv_addr, cli_addr;
int n,nu=1,i;


int usercount = 0;



void error(char *msg)
{
	puts(msg);
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
	fp= fopen("user.txt", "w");
	fprintf(fp, "");
	fclose(fp);
	
	while(1)
	{
		newsockfd[no] = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
		
		if (newsockfd<0)
			error("ERROR! Aceptando la conexion");
		//CREA THREADS PARA CADA INCOMING CONECTION
		pthread_mutex_lock(&mutex);
		pthread_create(&thread, NULL, server,(void*)&newsockfd[no]);
		no+=1;
		puts("Conexion aceptada");
	}
	
	close(newsockfd[no]);
	close(sockfd);
	return 0;
	
}

//LOGICA DEL SERVIDOR
void* server(void* sock)
{
	int newsockfd=*((int*)sock),j=0,m, socktest;
	
	checking:
	puts("checking");
	bzero(protclstr,150);
	socktest = recv(newsockfd, protclstr, 149, 0);
	
	if (socktest < 0) {
		puts("ERROR! Fallo al recibir el mensaje");
	}
	char line[150];
	char changeStr[256];
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
		fp = fopen("user.txt", "r+");
		char *username = tokens[1];
		
		
		bzero(buffer, 256);
		bzero(line, 150);
		
		snprintf(buffer,sizeof(buffer), "01|%s|127.0.0.1", username);

		while (fgets(line, sizeof(line), fp)) {
			char ** tok = NULL;
			char * p = strtok(line, "|");
			int n_spaces = 0, i;
			while (p) {
				tok = realloc(tok , sizeof(char*) * ++n_spaces);
				if (tok == NULL)
					exit(-1);
		
				tok[n_spaces-1] = p;
				p = strtok(NULL, "|");
			}

			if (strcmp(tok[0],username) == 0){
			
				puts("Usuario ya existe");
				send(newsockfd, buffer, 255, 0);
				free(tok);
				fclose(fp);
				pthread_mutex_unlock(&mutex);
				goto checking;
			} 
		} 
		
		snprintf(protclstr, sizeof(protclstr), "%s|%s|%s|%d|%s", tokens[1],tokens[2], tokens[3], newsockfd, tokens[4]);

		fclose(fp);
		fp = fopen("user.txt", "a");
		fputs(protclstr, fp);
		pthread_mutex_unlock(&mutex);
		puts("usuario creado");
		fclose(fp);
		goto checking;
		
			
	} else if ( strcmp(tokens[0],"02") == 0) {
		//Logica de liberacion de usuario
		fp = fopen("user.txt", "r+");
		tmpfp = fopen("tmpuser.txt", "w");
		fprintf(tmpfp, "");
		fclose(tmpfp);
		tmpfp = fopen("tmpuser.txt", "a");
		char *username = tokens[1];
		bzero(line, 150);
		while (fgets(line, sizeof(line), fp)) {
			char fullLine[150];
			strcpy(fullLine, line);
			char ** tok = NULL;
			char * p = strtok(line, "|");
			int n_spaces = 0, i;
			while (p) {
				tok = realloc(tok , sizeof(char*) * ++n_spaces);
				if (tok == NULL)
					exit(-1);
		
				tok[n_spaces-1] = p;
				p = strtok(NULL, "|");
			}
			
			if (strncmp( tok[0], tokens[1], sizeof(tokens[1]-2))== 0) {
				
				fprintf(tmpfp, ""); 
				
			} else {
				
				fputs(fullLine, tmpfp);
			}
			free(tok);
		}
		
		fclose(fp);
		fclose(tmpfp);
		remove("user.txt");
		rename("tmpuser.txt", "user.txt");
		goto END;
	} else if ( strcmp(tokens[0],"03") == 0) {
		//Logica de cambio de estado
		int k;
		
		
		fp = fopen("user.txt", "r+");
		tmpfp = fopen("tmpuser.txt", "w");
		fprintf(tmpfp, "");
		fclose(tmpfp);
		tmpfp = fopen("tmpuser.txt", "a");
		char *username = tokens[1];
		bzero(line, 150);
		bzero(changeStr, 256);
		while (fgets(line, sizeof(line), fp)) {
			char fullLine[150];
			strcpy(fullLine, line);
			char ** tok = NULL;
			char * p = strtok(line, "|");
			int n_spaces = 0, i;
			while (p) {
				tok = realloc(tok , sizeof(char*) * ++n_spaces);
				if (tok == NULL)
					exit(-1);
		
				tok[n_spaces-1] = p;
				p = strtok(NULL, "|");
			}
			
			if (strncmp( tok[0], tokens[1], sizeof(tokens[1]-2))== 0) {
				
				puts(tok[0]);
				puts(tok[1]);
				puts(tok[2]);
				puts(tok[3]);
				puts(tokens[2]);
				
				snprintf(changeStr, sizeof(changeStr), "%s|%s|%s|%s|%s", tok[0],tok[1], tok[2], tok[3], tokens[2]);
				fputs(changeStr, tmpfp); 
				
			} else {
				
				fputs(fullLine, tmpfp);
			}
		free(tok);
		}
		
		fclose(fp);
		fclose(tmpfp);
		remove("user.txt");
		rename("tmpuser.txt", "user.txt");
		pthread_mutex_unlock(&mutex);
		goto checking;
	} else if ( strcmp(tokens[0],"04") == 0) {
		//Logica de peticion de informacion
		int k;
		char *userDeInfo = tokens[1];
		char *username = tokens[2];
		
	} else if ( strcmp(tokens[0],"06") == 0) {
		//Logica de solicitud de listado de usuarios
		int k;
		char newBuffer[1024];
		char *username = tokens[1];
		bzero(newBuffer, 1024);
		snprintf(newBuffer, sizeof(newBuffer), "07|%s|", username);
		
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
			
	} else {
		puts("WTF you didnt respect the protocol dickhead");
		goto checking;
	}
	END:
	free(tokens);
	close(newsockfd);
	pthread_exit(NULL);	
}	
