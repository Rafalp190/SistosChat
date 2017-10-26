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
	
void* receive(void* d){
		int sockfd=*((int *)d);
		int a;
		char buf[256];


while(1){

	bzero(buf,256);
	a=recv(sockfd,buf,255,0);
	if(a<=0){
		error("Error reading from socket");
	}
	else{
		/*char ** toki = NULL;
		int m_spaces = 0;
		char * q = strtok(buf, "|");
		while (q) {
			toki = realloc(toki , sizeof(char*) * ++m_spaces);
			if (toki == NULL)
				exit(-1);
				toki[m_spaces-1] = q;
				q = strtok(NULL, "|");
			}
		puts(toki[1]);*/
		//printf("\n%s\n%s\n",toki[1],toki[3]);
		printf("\n%s\n",buf);
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
		snprintf(buffer,sizeof(buffer),"00|%s|%s|%s|0",username,serverIP,clientPort);
		puts(buffer);
		send(sockfd,buffer,255,0);
		char a1[50];
		//pthread_create(&thread,NULL,receive,(void*)&sockfd);
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
				puts(buffer);
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
				printf("You've fucked up, motherfucker!\n");
				}
			}
		else if (strcmp(a1,"2")==0){		//user info

			char ** tok = NULL;
			int n_spaces = 0;
			char a3[50], status[50];
			char uinfobuff[256];
			
			printf("Ingrese el nombre del usuario\n");
			scanf("\n%s", &a3);
			bzero(buffer,256);
			bzero(uinfobuff,256);
			snprintf(buffer,sizeof(buffer),"04|%s|%s",a3,username);
			//puts(buffer);
			send(sockfd,buffer,255,0);
			recv(sockfd,uinfobuff,255,0);
			char * p = strtok(uinfobuff, "|");
			//puts(uinfobuff);
			while (p) {
				tok = realloc(tok , sizeof(char*) * ++n_spaces);
				if (tok == NULL)
					exit(-1);
		
				tok[n_spaces-1] = p;
				p = strtok(NULL, "|");
			}
			if(strcmp(tok[4],"0")==0) strncpy(status,"Activo",50);
			else if(strcmp(tok[4],"1")==0) strncpy(status,"Idle",50);
			else strncpy(status,"Away",50);
			printf("El usuario %s, con direccion IP %s y puerto %s se encuentra %s.", tok[1], tok[2], tok[3], status);
			}
		else if (strcmp(a1,"3")==0){		//user list
			char ** tok = NULL, ** tokind=NULL;
			char uinfobuff[256];
			int n = 0,j = 0;
			int n_spaces = 0;
			bzero(buffer,256);
			bzero(uinfobuff,256);
			snprintf(buffer,sizeof(buffer),"06|%s",username);
			//puts(buffer);
			send(sockfd,buffer,255,0);
			recv(sockfd,uinfobuff,255,0);
			//puts(uinfobuff);
			char * p = strtok(uinfobuff, "|");
			while (p) {
				tok = realloc(tok , sizeof(char*) * ++n_spaces);
				if (tok == NULL)
					exit(-1);
		
				tok[n_spaces-1] = p;
				p = strtok(NULL, "|");
			}
			p = strtok(tok[2],"&");
			tok = NULL;
			n_spaces = 0;
			while (p) {
				tok = realloc(tok , sizeof(char*) * ++n_spaces);
				if (tok == NULL)
					exit(-1);
		
				tok[n_spaces-1] = p;
				p = strtok(NULL, "&");
				n++;
			}
			
			//printf("tamano%d\n",n);
			
			while(j<n){
				p = strtok(tok[j],"+");
				n_spaces = 0;
					while (p) {
					tokind = realloc(tokind , sizeof(char*) * ++n_spaces);
					if (tokind == NULL)
						exit(-1);
			
					tokind[n_spaces-1] = p;
					p = strtok(NULL, "+");
					}	
					printf("El usuario %s se encuentra %s\n", tokind[0],tokind[1]);
					j++;
				}
			printf("%s\n%s\n",tok[0],tok[1]);
			}
		else if (strcmp(a1,"4")==0){		//send msg
			char a4[50], a5[255];
			printf("Ingrese el nombre del usuario al que desea enviarle un mensaje\n");
			scanf("%s", &a4);
			printf("Ingrese el mensaje que desea enviar. Un maximo de 255 caracteres\n");
			scanf("%s", &a5);
			printf("Enviaste esta mamada:\n%s", a5);
			bzero(buffer,256);
			snprintf(buffer,sizeof(buffer),"08|%s|%s|%s",username,a4,a5);
			send(sockfd,buffer,255,0);
			puts(buffer);
			}
		else if (strcmp(a1,"5")==0){		//gtfo
			bzero(buffer,256);
			snprintf(buffer,sizeof(buffer),"02|%s",username);
			send(sockfd,buffer,255,0);
			puts(buffer);
			puts("baibai");
			break;
			}
		else{								//mula...
			printf("You've gone and fucked it, asshole!");
			}
	}		
	}
	
}

