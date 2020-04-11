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
#include<libgen.h>

char workdir[256];

bool is_file(const char* path){
	struct stat buff;
	stat(path, &buff);
	return S_ISREG(buff.st_mode);
}

void *categorize(void* argument){
	char *fullPath = (char *) argument;
	pthread_t id = pthread_self();
	//printf("fullPath : %s\n", fullPath);

	//printf("hehe\n");

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
	memset(extension, '\0', sizeof(extension));

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
	sprintf(oldpath,"./%s", temp);
	sprintf(newpath,"./%s/%s", extension, temp);

	//printf("oldpath : %s\n", oldpath);
	//printf("newpath : %s\n", newpath);


	rename(oldpath, newpath);
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
						printf("%s\n", fullPath);
						if(pthread_create( &tid[counter], NULL, categorize, (void*) fullPath) != 0){
							printf("Cannot create thread\n");
						}
						++counter;
					}
					closedir(isDir);
				}
			}

			for(int i=0; i<counter; i++){
				pthread_join(tid[i], NULL);
			}
		}
		closedir(directory);
	}
	else if(argc == 3 && !strcmp(argv[1], "-d")){
		chdir(argv[2]);
		DIR* directory = opendir(".");
		DIR* isDir;
		struct dirent *ls;

		int counter = 1;
		if(directory){
			char cwd[1024];
			getcwd(cwd, sizeof(cwd));
			while(ls = readdir(directory)){
				if(strcmp(ls->d_name, ".") && strcmp(ls->d_name, "..")){
					char fullPath[1024];
					sprintf(fullPath,"%s/%s", cwd,ls->d_name);
					isDir = opendir(fullPath);
					if(isDir == NULL){
						printf("%s\n", fullPath);
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
					sprintf(fullPath,"%s/%s", cwd,ls->d_name);
					isDir = opendir(fullPath);
					if(!isDir){
						printf("%s\n", fullPath);
						if(pthread_create( &tid[counter], NULL, categorize, (void*) fullPath) != 0){
							printf("Cannot create thread\n");
						}
						pthread_join(tid[counter], NULL);
						++counter;
					}
					closedir(isDir);
				}
			}
		}
		closedir(directory);
	}
	else if(argc > 2 && !strcmp(argv[1], "-f")){
		
		pthread_t tid[argc];
		for(int i=2; i < argc; ++i){
			if(is_file(argv[i])){
				char fullPath[1024];
		
				char* path1 = strdup(argv[i]);
				char* path2 = strdup(argv[i]);

				char* dir = dirname(path1);
				char* filename = basename(path2);

				chdir(dir);
				char cwd[1024];
				getcwd(cwd, sizeof(cwd));

				sprintf(fullPath, "%s/%s",cwd, filename);
				//printf("ini cwd %s\n", filename);


				if(pthread_create(&tid[i], NULL, categorize, (void*) fullPath) != 0){
					printf("Cannot create thread\n");
				}
				pthread_join(tid[i], NULL);
			}
			else printf("Cuih\n");
		}
	}	
	return 0;
}