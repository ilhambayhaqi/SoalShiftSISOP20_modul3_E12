# SoalShiftSISOP20_modul3_E12
Kendala  
No1. Masih bingung soalnya dan belum bisa membuat  
No2. Baru sampai login dan register  
No3. Belum selesai baru yang argumen *  
No4. Belum ada kendala

## Soal 2

Pada soal 2 terdapat code yaitu untuk server dan client.
- Untuk Client  
Pada client awalnya dilakukan koneksi ke server, bila berhasil kemudian diinisialisasi screen menjadi 1 menunjukkan saat ini berada pada screen 1 dengan auth "Belum Login".
```
int fd;
SetSocketBlockingEnabled(fd, true);

struct sockaddr_in address;
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

```
Kemudian pada screen 1 terdapat dua jenis pilihan yaitu login dan register. Untuk keduanya sama-sama mengambil 2 input yaitu Username dan Pasword yang nantinya akan dikirimkan ke server sesuai requestnya. Untuk login terdapat tambahan untuk melakukan read auth dari server sebagai berikut.
```
memset(auth, '\0', sizeof(auth));
read(sock, auth, 256);
if(!strcmp(auth, "authSuccess")){
	screen = 2;
	continue;
}
```
Pada potongan kode diatas, apabila login sukses maka akan beralih ke screen 2 apabila gagal maka kembali ke screen 1 dan karena isi auth saat ini adalah "authFailed" maka akan mencetak "Login Gagal!"
```
printf("\033c"); // Untuk melakukan clear screen
if(!strcmp(auth, "authFailed")) printf("Login Gagal!\n");
...
```
Pada screen 2 terdapat 2 pilihan juga yaitu find dan logout, untuk logout maka tinggal merubah auth menjadi "Belum Login" dan kembali ke screen 1.
```
if(!strcmp(input, "logout\n")){
	strcpy(auth, "Belum Login");
	screen = 1;
	continue;
}
```
Sedangkan pada find maka akan mengirimkan request pada server. Karena secara default read akan menunggu hingga mendapat jawaban dari server (karena secara default ter-block) maka bisa digunakan untuk menunggu game hingga mulai.
```
...
strcpy(charbuffer, "findPlayer");
send(sock, charbuffer, strlen(charbuffer), 0);

printf("\033c");
printf("Waiting for player ...\n");

read(sock, status, 256);
```
Ketika server telah membalas, maka tandanya game telah dimulai. Dan karena client harus dapat mengirim serta menerima request/status terhadap server maka dibuatlah thread. Thread tersebut merujuk pada read dari server sedangkan pada process utama berfungsi untuk melakukan scan pada input dan mengirimkan pada server.  
```
pthread_t tid;
pthread_create(&tid, NULL, readStatus, NULL);
```
Pada fungsi readStatus akan terus membaca status yang diberikan dari server hingga berubah menjadi Win atau Lose. Karena pada server nantinya menang dan kalah bisa dikirim oleh thread yang mengurusi lawan maka agar tidak terjadi error dikirim kembali ke server status game over agar status win tidak berubah menjadi lose (pada server setelah ada pemenang, healt akan diset menjadi 0 kembali sehingga bisa saja setelah thread mengirimkan status win, thread pada lawan mengirim status lose karena pada thread yang satunya health telah di set 0).
```
void* readStatus(void* argument){
    char *gameover = "gameover";
    while(!(!strcmp(status, "Win") || !strcmp(status, "Lose"))){
        memset(status, '\0', sizeof(status));
        read(sock, status, 256);
        printf("-Health %s-\n", status);
        if(!strcmp(status,"Win")){
            send(sock, gameover, strlen(gameover), 0);
            break;
        }
        if(!strcmp(status, "Lose")){
            send(sock, gameover, strlen(gameover), 0);
            break;
        }
    }
}
```
Pada main, while akan berhenti ketika terjadi status win atau lose. Karena getchar secara default melakukan block, maka diset dulu fd menjadi nonblock agar ketika status berubah tidak perlu menunggu input dari user. Untuk merubah dari block ke nonblock dan sebaliknya digunakan SetSocketBlockingEnabled() sebagai berikut
```
bool SetSocketBlockingEnabled(int fd, bool blocking)
{
    if (fd < 0) return false;

   int flags = fcntl(fd, F_GETFL, 0);
   if (flags == -1) return false;
   flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
   return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
}
```
Sedangkan karena input pada program client tidak perlu menekan enter(secara default, buffer harus ditekan enter untuk melakukan flush), maka sebelumnya diset pada STDIN_FILENO sebagai berikut
```
void setNewAttr(void){
    int c = 0;
    int res = 0;
    res = tcgetattr(STDIN_FILENO, &org_opts);
    assert(res==0);

    memcpy(&new_opts, &org_opts, sizeof(new_opts));
    new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
}
```
Untuk mengembalikannya sebagai berikut.
```
void returnAttr(void){
    int res;
    res=tcsetattr(STDIN_FILENO, TCSANOW, &org_opts);
    assert(res==0);
}
```
Secara keseluruhan, program inputnya sebagai berikut.
```
setNewAttr();
while(!(!strcmp(status, "Win") || !strcmp(status, "Lose"))){
	SetSocketBlockingEnabled(fd, false);
	char c = getchar();
	SetSocketBlockingEnabled(fd, true);
	
	if(c == ' ' && !(!strcmp(status, "Win") || !strcmp(status, "Lose"))){
		printf("HIT!!\n");
		send(sock, &c, 1, 0);
	}
}
returnAttr();
		
```

- Untuk Server  
Pada server awalnya dikalukan konfigurasi sebagai berikut.
```
int server_fd, new_socket, valread, activity, sd, max_sd;
char buffer[1024] = {0};
int opt = 1;
struct sockaddr_in address;
int addrlen = sizeof(address)
fd_set readfds;

pthread_t tid[max_client];

memset(busy, false, sizeof(busy));
memset(client_socket, 0, sizeof(client_socket));

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
```
Kemudian setelah konfigurasi selesai dilakukan looping dan listen. Pada awalnya telah disediakan set untuk file descriptor. Awalnya dilakukan insert pada fd server kemudian untuk setiap client yang berhasil terkoneksi juga akan dilakukan insert. Set ini digunakan apabila terjadi perubahan pada fd client (FD_ISSET menjadi true) maka akan dieksekusi oleh server. Kemudian untuk tiap client dibuat thread untuk menangani request dari tiap socket dan agar tidak ada create yang dobel, maka diberikan boolean array busy dimana bila busy[i] adalah true maka thread i tidak akan di create lagi sehingga tidak mengakses variabel yang sama.

```
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

	activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
	if ((activity < 0) && (errno!=EINTR)){   
  		printf("Select error\n");   
	}	 

	if(FD_ISSET(server_fd, &readfds)){
  		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
    			perror("accept");
    			exit(EXIT_FAILURE);
    
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
```
Tiap thread yang dibentuk menangani proses read dan send dari/ke satu client socket. Request yang mungkin ada adalah register, login, dan find.  
Untuk register maka akan membuka file akun.txt dalam mode append+, kemudian membaca username dan password dari client socket. Dari read tersebut dijadikan ke dalam struct Auth dan dilakukan fwrite. Kemudian karena pada soal diminta untuk menampilkan seluruh akun yang telah terdaftar maka dilakukan fread hingga pada akun.txt
```
FILE *database = fopen("akun.txt","a+");
if(!strcmp(buffer, "registerRequest")){
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
}
```
Untuk login, hampir sama dengan register namun pada freadnya dilakukan pembandingan, bila cocok maka bool login menjadi true dan akan mengirimkan "authSuccess" pada client. Bila gagal akan mengirim "authFailed" pada client.
```
bool login = false;
while(fread(&dataAccount, sizeof(Auth), 1, database)){
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
```
Untuk request find player, maka health akan diset menjadi 100, dan akan melakukan looping mencari lawan yang belum ada pasangan (opponentnya -1) dan healthnya 100. Bila sudah mendapat pasangan maka opponent dari tiap no socket akan diset menjadi no socket lawannya dan memasuki fase play.

```
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
}
```
Saat sudah play maka server mengirim status play pada client ```send(client_socket[i], Status, strlen(Status), 0);``` dan server akan membaca input dari client serta mengirimkan status bila telah ada pemenang. Setelah menang maka thread akan di sleep terlebih dahulu agar ada delay pada set opponent dan health agar hasil menang dan kalah tidak berubah.
```
while(true){
  memset(readBuff, '\0', sizeof(readBuff));
  read(client_socket[i], readBuff, 256);
  if(!strcmp(readBuff, "gameover")) break;
  if(!strcmp(readBuff, " ")){
      health[opponent[i]] -= 10;
      sprintf(life, "%d", health[opponent[i]]);
      send(client_socket[opponent[i]], life, strlen(life), 0);
  }
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
```
Setelah tidak ada perubahan pada fd client maka thread akan di exit dan busy menjadi false agar dapat dibentuk thread lagi.


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
