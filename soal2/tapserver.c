#include <stdio.h> 
#include <string.h>
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h> 
#include <arpa/inet.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h>
#include <stdbool.h>
#include <pthread.h>
#define PORT 8080 

typedef struct Auth{
	char username[256];
	char password[256];
}Auth;


typedef struct sock{
	int *i; 
}sock;
	
int max_client = 10;
bool busy[10];
int client_socket[10];
int opponent[10] = {[0 ... 9] = -1};
int health[10];

void *action(void*);

int main(int argc , char *argv[]) {
	int server_fd, new_socket, valread, activity, sd, max_sd;
	char buffer[1024] = {0};
    int opt = 1;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    fd_set readfds;

    pthread_t tid[max_client];

    memset(busy, false, sizeof(busy));
    memset(client_socket, 0, sizeof(client_socket));
    //memset(opponent, 0, sizeof(opponent));
    memset(health, 0 , sizeof(health));

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
      
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
      
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    int hehe = 0;

    while(true){
	    if (listen(server_fd, 3) < 0) {
	        perror("listen");
	        exit(EXIT_FAILURE);
	    }

    	FD_ZERO(&readfds);
    	FD_SET(server_fd, &readfds);
    	max_sd = server_fd;

    	for(int i=0; i < max_client; ++i){
    		sd = client_socket[i];
    		if(sd > 0) FD_SET(sd, &readfds);
    		if(sd > max_sd) max_sd = sd;
    	}

    	//int select(int maxfd, fd_set *readsset, fd_set *writeset 
    	//			,fd_set *exceptset, const struct timeval *timeout);
    	
    	activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
    	if ((activity < 0) && (errno!=EINTR))   
        {   
            printf("Select error\n");   
        } 

		if(FD_ISSET(server_fd, &readfds)){
			if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
        		perror("accept");
        		exit(EXIT_FAILURE);
    		}

            for(int i = 0; i < max_client; i++){
            	if(!client_socket[i]){
            		client_socket[i] = new_socket;
            		break;
            	}
            }
		}

		for(int i=0; i < max_client; i++){
    		sd = client_socket[i];
            if(FD_ISSET(sd, &readfds)){
                //printf("Request from client %d", i);
                int currPos = i;
                sock *passing = malloc(sizeof *passing);
                passing->i = &currPos;


                if(!busy[i]){
                    busy[i] = true;
                    pthread_create(&tid[currPos], NULL, action, passing);
                }
            }
		}
    }
}

void* action(void *argument){
	pthread_t id = pthread_self();
    sock *arg = (sock*)argument;
    int i = *(arg->i);

    printf("User %d Connected\n", i);
    
    char buffer[1024];
    int valread;

    while(true){
        valread = read(client_socket[i], buffer, 1024);
        buffer[valread] = '\0';

        if(valread == 0){
            printf("Connection Lost\n");
            close(client_socket[i]);
            client_socket[i] = 0;
        }
        else{            
        	FILE *database = fopen("akun.txt","a+");
    	   //printf("i = %d buffer = %s\n", i , buffer);
        	if(!strcmp(buffer, "registerRequest")){
        		//printf("hehe-- %s\n", buffer);
    		    Auth userBuff;
                Auth dataAccount;
    		    memset(userBuff.username, '\0', sizeof(userBuff.username));
    		    memset(userBuff.password, '\0', sizeof(userBuff.password));

        		read(client_socket[i], userBuff.username, 256);
        		read(client_socket[i], userBuff.password, 256);

        		fwrite(&userBuff, sizeof(Auth), 1, database);
        		printf("Register Success\n");
                fclose(database);

                FILE *data = fopen("akun.txt","r");
                while(fread(&dataAccount, sizeof(Auth), 1, data)){
                    printf("username: %s\nPassword: %s\n\n", dataAccount.username, dataAccount.password);
                }
        		//printf("username: %s\nPassword :%s\n", userBuff.username, userBuff.password);
        	}
        	else if(!strcmp(buffer, "loginRequest")){
    		    Auth userBuff;
    		    Auth dataAccount;
    		    memset(userBuff.username, '\0', sizeof(userBuff.username));
    		    memset(userBuff.password, '\0', sizeof(userBuff.password));

        		read(client_socket[i], userBuff.username, 256);
        		read(client_socket[i], userBuff.password, 256);

        		bool login = false;
        		while(fread(&dataAccount, sizeof(Auth), 1, database)){
        			//printf("%s %s", dataAccount.username, dataAccount.password);
        			if(!strcmp(dataAccount.username, userBuff.username) 
        				&& !strcmp(dataAccount.password, userBuff.password)){
        				login = true;
        				break;
        			}
        		}

        		if(login){
        			char *authSuccess = "authSuccess";
        			send(client_socket[i], authSuccess, strlen(authSuccess), 0);
        			printf("Auth Success\n");
        		}
        		else{
        			char *authFailed = "authFailed";
        			send(client_socket[i], authFailed, strlen(authFailed), 0);
        			printf("Auth Failed\n");
        		}

                fclose(database);

        	}
            else if(!strcmp(buffer, "findPlayer")){
                health[i] = 100;

                while(opponent[i] == -1){
                    for(int j=0; j< max_client; j++){
                        if(opponent[j] == -1 && health[j] == 100 && j != i){
                            opponent[i] = j;
                            opponent[j] = i;
                            break;
                        }
                    }
                    //printf("Lagi cari gan\n");
                }

                char *Status = "Play";
                char *Win = "Win";
                char *Lose = "Lose";

                char readBuff[256], life[10];
                send(client_socket[i], Status, strlen(Status), 0);
                printf("%d Lawan no %d\n", i,opponent[i]);

                while(true){
                    memset(readBuff, '\0', sizeof(readBuff));
                    read(client_socket[i], readBuff, 256);
                    if(!strcmp(readBuff, "gameover")) break;
                    if(!strcmp(readBuff, " ")){
                        health[opponent[i]] -= 10;
                        sprintf(life, "%d", health[opponent[i]]);
                        send(client_socket[opponent[i]], life, strlen(life), 0);
                        //send(client_socket[i], Status, strlen(Status), 0);
                    }
                    /*
                    if(health[i] <= 0){
                        send(client_socket[i], Lose, strlen(Lose), 0);
                        send(client_socket[opponent[i]], Win, strlen(Win), 0);
                        printf("Send Lose\n");
                        break;
                    }*/
                    if(health[opponent[i]] <= 0){
                        send(client_socket[i], Win, strlen(Win), 0);
                        send(client_socket[opponent[i]], Lose, strlen(Lose), 0);
                        printf("Send Win\n");
                        break;
                    }


                }

                sleep(2);
                opponent[i] = -1;
                health[i] = 0;
            }

    
	    //printf("socket : %d %s\n",i,buffer );
	    //send(sd , welcome , strlen(welcome) , 0 );
	    //printf("Hello message sent\n");

	
        }
    }

	busy[i] = false;
	pthread_exit(NULL);
}