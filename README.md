# SoalShiftSISOP20_modul3_E12
Kendala  
No1. Masih bingung soalnya dan belum bisa membuat  
No2. Baru sampai login dan register  
No3. Belum selesai baru yang argumen *  
No4. Belum ada kendala

## Soal 3


Pada soal ke 3 terdapat 3 argument yang digunakan yaitu * untuk mengkategorikan file pada current working directory, -d [path] untuk mengkategorikan pada directory yang digunakan dan -f [file+1] [file_2] ... [file_n] untuk mengkategorikan file satu per satu.  
- Argument *  
Pada argument * awalnya dilakukan listing pada direktori saat ini. Untuk code dibawah ini awalnya melakukan pengecekan jumlah file yang ada pada direktori saat ini. Untuk menghitungnya terdapat variabel counter, counter dimulai dari 1 agar tidak perlu menambahkan lagi saat melakukan deklarasi array pthread.
```
DIR* directory = opendir("."); //membuka direktori saat ini
DIR* isDir; 

struct dirent *ls;
int counter = 1; 
while(ls = readdir(directory)){ //membaca isi direktori saat ini
	if(strcmp(ls->d_name, ".") && strcmp(ls->d_name, "..")){ //agar "." dan ".." tidak diikutkan
		char fullPath[1024];
		sprintf(fullPath,"%s/%s", workdir,ls->d_name);
		isDir = opendir(fullPath);
		if(!isDir){
			++counter;
		}
		closedir(isDir);
	}
}
```
Kemudian setelah dilakukan penghitungan diinisialisasi pthread_t sebanyak counter yang ada dan dengan cara seperti sebelumnya dilakukan pembuatan thread dengan menjalankan fungsi catgorize dengan input parameter yaitu full path dari file tersebut.
```
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
```
Pada potongan kode diatas dengan menggabungkan path working direktori saat ini dan nama file maka tebentuk full path dari tiap file yang akan dikategorisasikan. Full path tersebut kemudian dipassing menuju fungsi categorize pada thread.

- Argument -d
Konsep dari program argument ini sama dengan argumen * yaitu menghitung banyak file pada direktori kemudian melakukan passing tiap full path pada file yang akan dikategorikan menuju thread categorize. Perbedaannya yaitu pada penggabungan working direktori menggunakan working direktori sesuai input user, oleh karena itu pada awalnya terdapat ```chdir(argv[2]);``` untuk memindahkan working direktori pada argument input kemudian untuk current directory dicari dengan fungsi ```getcwd(cwd, sizeof(cwd));```
Untuk sisanya kurang lebih sama dengan argument *. Untuk kodenya sebagai berikut.
```
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
```

- Argument -f
Pertama dilakukan pengecekan apakah path yang diinputkan oleh user merupakan file atau bukan sebagai berikut.
```
if(is_file(argv[i])){
  ....
}
```
Untuk fungsi is_file tersebut detilnya sebagai berikut.
```
bool is_file(const char* path){
	struct stat buff;
	stat(path, &buff);
	return S_ISREG(buff.st_mode);
}
```
Pada fungsi tersebut akan mengembalikan true apabila path yang diberikan merupakan regular file dan false bila tidak.  

Untuk argument -f karena input dari user merupakan path file bukan direktori maka untuk mencari current working direktorinya menggunakan fungsi dirname() dengan parameter path filenya. Setelah diketahui working direktorinya maka dilakukan ```chdir(dir);``` untuk merubah current working direktori.
```
char* path1 = strdup(argv[i]);
char* path2 = strdup(argv[i]);

char* dir = dirname(path1);
char* filename = basename(path2);

chdir(dir);
char cwd[1024];
getcwd(cwd, sizeof(cwd));
```
Kemudian dilakukan penggabungan string cwd dan nama file agar menjadi full path dan dipassing menuju thread categorize
```
sprintf(fullPath, "%s/%s",cwd, filename);
if(pthread_create(&tid[i], NULL, categorize, (void*) fullPath) != 0){
	printf("Cannot create thread\n");
}
```

- fungsi categorize
Fungsi ini merupakan fungsi untuk melakukan kategorisasi pada tiap file dengan parameter input adalah full path dari tiap file yang akan dikategorikan. Pada fungsi ini dilakukan pencarian extensi terlebih dahulu, pencarian itu dilakukan dengan membagi full path dengan delimiter / terlebih dahulu (sebenarnya bisa menggunakan fungsi basename tapi udah terlanjur buat) k
```
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
```
Pada potongan diatas merupakan mencari current workdir (sebenarnya juga bisa menggunakan getcwd karena sebelumnya sudah dipindahkan workking directorynya atau juga menggunakan dirname dari fullpath yang ada). Sedangkan filenamenya didapatkan dari arr[i-1].
Kemudian untuk mencari extensinya juga dilakukan parsing dengan delimiter "." dan agar namafilenya tidak berubah maka sebelumnya dibuat variabel temp untuk menampung filename aslinya 
```
char filename[256], temp[256];
sprintf(filename, "%s", arr[i-1]);
sprintf(temp, "%s", arr[i-1]);
```
Bila setelah parsing hanya terdapat 1 string maka file tersebut tidak memiliki extensi dan ekstensinya dijadikan "unknown". Bila tidak maka ekstensinya dibuat menjadi kecil dengan fungsi tolower.
```
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
	if(extension[0] == '\0'){
		strcpy(extension, "unknown");
	}
```
Kemudian dilakukan pengecekan terhadap folder ekstensi apakah telah tersedia atau belum, bila belum maka dilakukan pembuatan folder ekstensi sebagai berikut. 
```
struct stat st = {0};
if (stat(extension, &st) == -1) {
    mkdir(extension, 0777);
}
```
Untuk fungsi movenya digunakan rename() dengan dua parameter yaitu old path dan newpath sebagai berikut.
```
char newpath[256], oldpath[256];
sprintf(oldpath,"./%s", temp);
sprintf(newpath,"./%s/%s", extension, temp);

rename(oldpath, newpath);
```

## Soal 4
