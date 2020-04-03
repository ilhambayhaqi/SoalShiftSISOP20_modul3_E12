#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<syslog.h>
#include<string.h>
#include<stdbool.h>
#include<time.h>
#include<ctype.h>
#include<limits.h>
#include<dirent.h>
#include<math.h>
#include<wait.h>
#include<pthread.h> 
#include<sys/stat.h>

char workdir[256];

void *categorize(void* argument){
	char *fullPath = (char *) argument;
	pthread_t id = pthread_self();
	//printf("fullPath : %s\n", fullPath);


	char *str = strtok(fullPath, "/");
	int i = 0;

	char *arr[256];
	while(str != NULL){
		arr[i++] = str;
		str = strtok(NULL, "/");
	}

	
	char currWorkdir[1024];
	currWorkdir[0] = '\0';
	for(int j=0; j < i-1; j++){
		sprintf(currWorkdir, "%s%s/", currWorkdir, arr[j]);
	}
	//printf("%s\n", currWorkdir);

	char filename[256], temp[256];
	sprintf(filename, "%s", arr[i-1]);
	sprintf(temp, "%s", arr[i-1]);
	//printf("filename : %s\n", filename);

	char extension[64];

	i=0;
	str = strtok(filename, ".");
	while(str != NULL){
		arr[i++] = str;
		str = strtok(NULL, ".");
	}


	if(i == 1) extension[0] = '\0';
	else{
		for(int j=0; j < strlen(arr[i-1]); j++){
			extension[j] = tolower(*(arr[i-1]+j));
			if(!(isalpha(*(arr[i-1]+j)) || isdigit(*(arr[i-1]+j)) )){
				extension[0] = '\0';
				break;
			}
		}
	}
	//printf("%s\n", extension);

	if(extension[0] == '\0'){
		strcpy(extension, "unknown");
	}

	chdir(currWorkdir);
	struct stat st = {0};
	if (stat(extension, &st) == -1) {
    	mkdir(extension, 0777);
	}

	char newpath[256], oldpath[256];
	sprintf(oldpath,"\"./%s\"", temp);
	sprintf(newpath,"\"./%s/%s\"", extension, temp);

	printf("oldpath : %s\n", oldpath);
	printf("newpath : %s\n", newpath);


	if(rename(oldpath, newpath)){

	}
	//printf("extension : %s\n", extension);

}

int main(int argc, char const *argv[]){
	getcwd(workdir, sizeof(workdir));
	//printf("%s\n", workdir);

	if(argc == 2 && !strcmp(argv[1], "*")){
		DIR* directory = opendir(".");
		DIR* isDir;
		struct dirent *ls;

		int counter = 1;
		if(directory){
			while(ls = readdir(directory)){
				if(strcmp(ls->d_name, ".") && strcmp(ls->d_name, "..")){
					char fullPath[1024];
					sprintf(fullPath,"%s/%s", workdir,ls->d_name);
					isDir = opendir(fullPath);
					if(!isDir){
						//printf("%s\n", fullPath);
						++counter;
					}
					closedir(isDir);
				}
			}

			closedir(directory);
			directory = opendir(".");

			pthread_t tid[counter];

			counter = 0;
			while(ls = readdir(directory)){
				if(strcmp(ls->d_name, ".") && strcmp(ls->d_name, "..")){
					char fullPath[1024];
					sprintf(fullPath,"%s/%s", workdir,ls->d_name);
					isDir = opendir(fullPath);
					if(!isDir){
						//printf("%s\n", fullPath);
						if(pthread_create( &tid[counter], NULL, categorize, (void*) fullPath) != 0){
							printf("Cannot create thread\n");
						}
					}
					closedir(isDir);
				}
			sleep(3);
			}
		}
		closedir(directory);
	}
	else if(argc == 3 && !strcmp(argv[1], "-d")){
		DIR* directory = opendir(argv[2]);
		DIR* isDir;
		struct dirent *ls;

		int counter = 1;
		if(directory){
			while(ls = readdir(directory)){
				if(strcmp(ls->d_name, ".") && strcmp(ls->d_name, "..")){
					char fullPath[1024];
					sprintf(fullPath,"%s/%s", argv[2],ls->d_name);
					isDir = opendir(fullPath);
					if(!isDir){
						//printf("%s\n", fullPath);
						++counter;
					}
					closedir(isDir);
				}
			}

			closedir(directory);
			directory = opendir(".");

			pthread_t tid[counter];

			counter = 0;
			while(ls = readdir(directory)){
				if(strcmp(ls->d_name, ".") && strcmp(ls->d_name, "..")){
					char fullPath[1024];
					sprintf(fullPath,"%s/%s", argv[2],ls->d_name);
					isDir = opendir(fullPath);
					if(!isDir){
						//printf("%s\n", fullPath);
						if(pthread_create( &tid[counter], NULL, categorize, (void*) fullPath) != 0){
							printf("Cannot create thread\n");
						}
					}
					closedir(isDir);
				}
			sleep(3);
			}
		}
		closedir(directory);
	}
	else if(argc > 2 && !strcmp(argv[1], "-f")){

	}	
	return 0;
}