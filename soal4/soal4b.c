#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<sys/shm.h>

#define MOD 20
#define row1 4
#define col1 2
#define row2 2
#define col2 5

typedef struct{
	int val;
	int res;
}idku;

pthread_t tid[row1][col2];

void *sum(void *arg){
	idku *hehe = (idku*)arg;
	int val = hehe->val;
	int temp = 0;
	
	for(int i=1; i<=val; i++) 
		temp += i;

	hehe->res = temp;
	return NULL;
}

int main(){

	key_t key = 1234;
	int shmid = shmget(key, sizeof(int[row1][col2]), IPC_CREAT | 0666);
	int (*shmres)[row1] = shmat(shmid, 0,0);

	for(int i=0; i<row1; i++){
		for(int j=0; j<col2; j++){
			printf("%3d ", shmres[i][j]);
		}
		printf("\n");
	}

	idku hehe[row1][col2];
	for(int i=0; i<row1; i++){
		for (int j = 0; j < col2; j++){
			hehe[i][j].val = shmres[i][j];
			pthread_create(&(tid[i][j]), NULL, &sum, (void *)&(hehe[i][j]));
		}
	}

	printf("\nHasil Penjumlahan 1 hingga a(i,j)\n");
	for(int i=0; i < row1; i++){
		for (int j = 0; j < col2; j++){
			pthread_join(tid[i][j], NULL);
			printf("%5d ", hehe[i][j].res);
		}
		printf("\n");
	}


	return 0;
}