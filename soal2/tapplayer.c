#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <string.h>
#define PORT 8080

typedef struct Auth{
	char username[256];
	char password[256];
}Auth;
  
int main(int argc, char const *argv[]) {
	struct sockaddr_in address;
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
        
   	char charbuffer[1024] = {0};
    char input[256];

          
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		return -1;
	}

   	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

   	if(inet_pton(AF_INET, "127.0.0.1" , &serv_addr.sin_addr)<=0) {
   		printf("\nInvalid address/ Address not supported \n");
   		return -1;
   	}
   

   	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
   		printf("\nConnection Failed \n");
   		return -1;
   	}

    char auth[256];
    strcpy(auth, "Belum Login");

   	int screen = 1;
    while(true){
        memset(charbuffer, 0, sizeof(charbuffer));
    	if(screen == 1){
    		printf("\033c");
            if(!strcmp(auth, "authFailed")) printf("Login Gagal!\n");
            strcpy(auth, "Belum Login");
    		printf("1. Login\n");
    		printf("2. Register\n");
    		printf("   Choices : ");
    		fgets(input, 256, stdin);

    		Auth userBuff;
    		if(!strcmp(input, "login\n")){
    			strcpy(charbuffer, "loginRequest");
    			send(sock, charbuffer, strlen(charbuffer), 0);

    			printf("   Username : ");
    			fgets(input, 256, stdin);
    			strtok(input,"\n");
    			strcpy(userBuff.username, input);
    			send(sock, userBuff.username, strlen(userBuff.username), 0);
    			
    			printf("   Password : ");
    			fgets(input, 256, stdin);
    			strtok(input, "\n");
    			strcpy(userBuff.password, input);
    			send(sock, userBuff.password, strlen(userBuff.password), 0);

                memset(auth, '\0', sizeof(auth));
                read(sock, auth, 256);
                if(!strcmp(auth, "authSuccess")){
                    screen = 2;
                    continue;
                }

    		}
    		else if(!strcmp(input, "register\n")){
    			strcpy(charbuffer, "registerRequest");
    			send(sock, charbuffer, strlen(charbuffer), 0);

    			printf("   Username : ");
    			fgets(input, 256, stdin);
    			strtok(input,"\n");
    			strcpy(userBuff.username, input);
    			send(sock, userBuff.username, strlen(userBuff.username), 0);
    			
    			printf("   Password : ");
    			fgets(input, 256, stdin);
    			strtok(input, "\n");
    			strcpy(userBuff.password, input);
    			send(sock, userBuff.password, strlen(userBuff.password), 0);
    			

    		}
    		else continue;
    	}
    	if(screen == 2){
            printf("\033c");
            if(!strcmp(auth, "authSuccess")) printf("Login Berhasil\n");
            strcpy(auth, "Udah login");

            printf("1. Find Match\n");
            printf("2. Logout\n");
            printf("   Choices : ");
            fgets(input,256, stdin);

            if(!strcmp(input, "logout\n")){
                strcpy(auth, "Belum Login");
                screen = 1;
                continue;
            }
            if(!strcmp(input, "find\n")){
                strcpy(charbuffer, "findPlayer");
                send(sock, charbuffer, strlen(charbuffer), 0);

                char status[256];
                printf("\033c");
                printf("Waiting for player ...\n");

                memset(status, '\0', sizeof(status));
                read(sock, status, 256);

                while(strcmp(status, "Win") || strcmp(status, "Lose")){
                    printf("Play\n");
                    fgets(input,256, stdin);
                    char *tap = "tap";
                    send(sock, tap, strlen(tap), 0);
                    
                    memset(status, '\0', sizeof(status));
                    read(sock, status, 256);
                }
                printf("%s\n", status);       
            }
    	}
    }
}