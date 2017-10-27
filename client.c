#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

pthread_t thread;

void error (char *msg){
		puts(msg);
		exit(0);
	}

char * readText(void){
	char *msgBuffer;
	size_t bufsize = 32;
	size_t characters;
	
	msgBuffer =(char*)malloc(bufsize * sizeof(char));
	if(msgBuffer ==NULL){
		error("Unable to allocate buffer");
		}
		printf("");
		characters = getline(&msgBuffer,&bufsize,stdin);
		return(msgBuffer);
	}
	
void* receive(void* d){
		int sockfd=*((int *)d);
		int a;
		char buf[1024];


while(1){

	bzero(buf,1024);
	a=recv(sockfd,buf,1024,0);
	if(a<=0){
		error("Error reading from socket");
	}
	else{
		char ** tok = NULL, **tokind =NULL;
		int m_spaces = 0,n_spaces=0;
		char * q = strtok(buf, "|");
		char status[10];
		while (q) {
			tok = realloc(tok, sizeof(char*) * ++m_spaces);
			if (tok == NULL)
				exit(-1);
				tok[m_spaces-1] = q;
				q = strtok(NULL, "|");
			}
		if(strcmp(tok[0],"05")==0){
			if(strcmp(tok[4],"0")==0) strncpy(status,"Activo",10);
			else if(strcmp(tok[4],"1")==0) strncpy(status,"Idle",10);
			else strncpy(status,"Away",10);
			printf("El usuario %s, con direccion IP %s y puerto %s se encuentra %s.\n", tok[1], tok[2], tok[3], status);
			}
		else if(strcmp(tok[0],"07")==0){
			int j =0, n=0;
			char * t = strtok(tok[2],"&");
			n_spaces = 0;
			while (t) {
				tok = realloc(tok , sizeof(char*) * ++n_spaces);
				if (tok == NULL)
					exit(-1);
		
				tok[n_spaces-1] = t;
				t = strtok(NULL, "&");
				n++;
			}
			
			
			//printf("tamano%d\n",n);
			
			while(j<n){
				tokind = NULL;
				char *r = strtok(tok[j],"+");
				n_spaces = 0;
					while (r) {
					tokind = realloc(tokind , sizeof(char*) * ++n_spaces);
					if (tokind == NULL)
						exit(-1);
			
					tokind[n_spaces-1] = r;
					r = strtok(NULL, "+");
					}
					char status[10];
					if (strcmp(tokind[1],"0")==0) strncpy(status,"Activo",10);
					else if(strcmp(tok[1],"1")==0) strncpy(status,"Idle",10);
					else strncpy(status,"Away",10);
					printf("El usuario %s se encuentra %s\n", tokind[0],status);
					j++;
					
				}
			}
		else if(strcmp(tok[0],"08")==0){
			
			printf("\nTiene mensaje de: %s\nMensaje: %s\n", tok[1],tok[3]);
			puts("");
			}
	
	}
}
close(sockfd);
pthread_exit(NULL);
}


int main(int argc, char *argv[]){
	int sockfd, portno, n, newsockfd,userno;
	char *username, *clientPort, *serverIP, *serverPort;
	struct sockaddr_in arr_in, serv_addr;
	struct hostent *server;
	char buffer[256];
	
	if (argc<5){
		fprintf(stderr,"Usage %s Host Name Port\n", argv[0]);
		exit(0);
	}
	
	username = argv[1];
	clientPort = argv[2];
	serverIP = argv[3];
	serverPort = argv[4];
	/*puts(username);
	puts(clientPort);
	puts(serverIP);
	puts(serverPort);
	*/

	portno=atoi(argv[4]);
	sockfd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
 
 
    if (sockfd<0){
		error("ERROR Opening Socket");
	}
    
   server=gethostbyname(argv[3]);
    
   if (server==NULL) {
    fprintf(stderr, "ERROR, No Such Host\n");
    exit(0);
    }
    
    bzero((char*)&serv_addr, sizeof(serv_addr));
     
    serv_addr.sin_family=AF_INET;
    bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port=htons(portno);
    newsockfd = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));
    if (newsockfd<0){
		error("ERROR Connecting");
	}

	
	else{
		bzero(buffer,256);
		snprintf(buffer,sizeof(buffer),"00|%s|%s|%s|0\n",username,serverIP,clientPort);
		char a1[50];
		pthread_create(&thread,NULL,receive,(void*)&sockfd);
		send(sockfd,buffer,255,0);
	while(1){
	
	printf("\n%s\n","Hola, selecciona una opcion");
	printf("\n%s\n","1. Cambiar de status");
	printf("\n%s\n","2. Obtener informacion de un usuario");
	printf("\n%s\n","3. Obtener el listado de usuarios");
	printf("\n%s\n","4. Enviar mensaje a un usuario");
	printf("\n%s\n","5. Cerrar conexion y salir");
	scanf("%s", &a1);
	

		if (strcmp(a1,"1")==0){				//status change
			char a2[50];
			printf("Seleccione un nuevo status\n");
			printf("0. Activo\n");
			printf("1. Idle\n");
			printf("2. Away\n");
			scanf("%s",&a2);
			if(strcmp(a2,"0")==0||strcmp(a2,"1")==0||strcmp(a2,"2")==0){
				bzero(buffer,256);
				snprintf(buffer,sizeof(buffer),"03|%s|%s",username,a2);
				send(sockfd,buffer,255,0);
				if(strcmp(a2,"0")==0){
					printf("Tu estado ahora es: Activo\n");
					}
				else if(strcmp(a2,"1")==0){
					printf("Tu estado ahora es: Idle\n");
					}
				else if(strcmp(a2,"2")==0){
					printf("Tu estado ahora es: Away\n");
					}
				
			}
			else{
				printf("Elige un estado entre 0-2\n");
				}
			}
		else if (strcmp(a1,"2")==0){		//user info
			char a3[50], status[50];
			char uinfobuff[256];
			
			printf("Ingrese el nombre del usuario\n");
			scanf("\n%s", &a3);
			bzero(buffer,256);
			bzero(uinfobuff,256);
			snprintf(buffer,sizeof(buffer),"04|%s|%s",a3,username);
			//puts(buffer);
			send(sockfd,buffer,255,0);
			}
		else if (strcmp(a1,"3")==0){		//user list
			char uinfobuff[1024];
			bzero(buffer,256);
			bzero(uinfobuff,1024);
			snprintf(buffer,sizeof(buffer),"06|%s",username);
			send(sockfd,buffer,255,0);				
			}
		else if (strcmp(a1,"4")==0){		//send msg
			char a4[50], a5[256];
			
			
			printf("Ingrese el nombre del usuario al que desea enviarle un mensaje\n");
			scanf("%s", &a4);
			
			printf("Ingrese el mensaje que desea enviar. Un maximo de 255 caracteres\n");
			scanf("%s", &a5);
			bzero(buffer,256);
			snprintf(buffer,sizeof(buffer),"08|%s|%s|%s",username,a4,a5);
			send(sockfd,buffer,255,0);
			
			
			}
		else if (strcmp(a1,"5")==0){		//gtfo
			bzero(buffer,256);
			snprintf(buffer,sizeof(buffer),"02|%s",username);
			send(sockfd,buffer,255,0);
			puts("baibai");
			break;
			}
		else{								//opcion incorrecta
			printf("Ingrese una opcion entre 1-5");
			}
	}		
	}
	
}

