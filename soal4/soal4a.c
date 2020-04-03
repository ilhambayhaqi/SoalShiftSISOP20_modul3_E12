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


int mat1[][col1] = {{1,2},
					{3,4},
					{5,6},
					{7,8}};

int mat2[][col2] = {{1,2,3,4,5},
					{6,7,8,9,10}};

pthread_t tid[row1][col2];

pid_t child;

typedef struct{
	int idx;
	int idy;
	int res;
}idku;

void *multiply(void *arg){
	idku *hehe = (idku*)arg;
	int idx = hehe->idx;
	int idy = hehe->idy;
	int temp = 0;
	//printf("%d %d\n", idx, idy);
	
	for(int i=0; i<col1; i++) 
		temp += mat1[idy][i]*mat2[i][idx];

	hehe->res = temp;
	return NULL;
}

int main(){

	printf("Matriks 1\n");
	for(int i=0; i<row1; i++){
		for(int j=0; j<col1; j++){
			printf("%2d ", mat1[i][j]);
		}
		printf("\n");
	}

	printf("\n Matriks 2\n");
	for(int i=0; i<row2; i++){
		for(int j=0; j<col2; j++){
			printf("%2d ", mat2[i][j]);
		}
		printf("\n");
	}

	idku hehe[row1][col2];
	for(int i=0; i<row1; i++){
		for (int j = 0; j < col2; j++){
			hehe[i][j].idy = i;
			hehe[i][j].idx = j;
			pthread_create(&(tid[i][j]), NULL, &multiply, (void *)&(hehe[i][j]));
		}
	}
	
	key_t key = 1234;
	int shmid = shmget(key, sizeof(int[row1][col2]), IPC_CREAT | 0666);
	int (*shmres)[row1] = shmat(shmid, 0,0);

	printf("\n Hasil Perkalian\n");
	for(int i=0; i < row1; i++){
		for (int j = 0; j < col2; j++){
			pthread_join(tid[i][j], NULL);
			printf("%3d ", hehe[i][j].res);
			shmres[i][j] = hehe[i][j].res;
		}
		printf("\n");
	}

	return 0;
}