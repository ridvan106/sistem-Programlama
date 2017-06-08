/**********************************************************************
 *  Rıdvan Demirci 141044070                                          *
 *      Sistem Programlama HW4                                        *
 *          thread semaphore kullanımı                                *
 *              haberleştirmek için txt dosyaları kullanıldı          *
 *                                                                    *
 **********************************************************************/
#define _POSIX_SOURCE
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <values.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
/*      Thread ile gonderilen struct    */
typedef struct {
    char pathName[PATH_MAX];
    char searchWord[PATH_MAX];
    int size;
    char fileName[PATH_MAX];
    char onlyFileName[PATH_MAX];
}seachEnvironment;

sem_t *semName;
/* her directory altında olan thread sayısı */
int numOfthread = 0;
char mainFile[PATH_MAX];    // threadlerden gelen bilgileri maine aktarır
int mainpid;    // anaPid e ki dosya sayısını bulmak için kullanıyom
int sinyal = 0;
char directorySize[PATH_MAX];
int cascade(FILE *fp);  // cacade hesaplama fonksyonu
//verilen filedaki sayıları toplar
int total(FILE *fp){
    int temp = 0,sum = 0;
    while(!feof(fp)) {
        fscanf(fp, "%d", &temp);
        sum += temp;
        temp=0;
    }
    return sum;

}

/*
 * Bu fonksyon File dosyasinda kaldıgı yerden devam
 * eder ve devam eden stringin olup olmadigina
 * bakar
 */

int IshasString(FILE *StringFile,char  aranan[],int size){
    int cRead;      //Stringin üzerinde dolaşmayı sağlayan iteretor
    //Eger String Gelen edegere esit degil ise donguyu bitiren flag
    int flag =1;
    char readChar;  //file dan karakter okuma
    for (cRead = 1; cRead < size && flag == 1; ++cRead) {

        fscanf(StringFile, "%c", &readChar);  //dosyadan bir karakter okur
        if (feof(StringFile)) {     // eger dosya sonu ise -1 return edilir
            return -1;
        } else if (readChar == '\n') { // \n Ignore edilir
            // pthread_mutex_lock(&mutex);//row sayısını almak için critical section olusturulur
            FILE * LineSize;
            LineSize = fopen("LineSize","a+");
            fprintf(LineSize,"%d\n",1); // file hesabına gore

            fclose(LineSize);

           // pthread_mutex_unlock(&mutex);   // unlock
            cRead--;
        } else if (readChar == ' ') { // bosluk Ignore edilir
            cRead--;
        }
        else if(readChar == '\t'){  // \t karakteri Ignore edilir
            cRead--;
        }
        else if (aranan[cRead] != readChar) {
            flag = 0;     // eger Stringdeki karakter file dakine eşit değilse donguden cikar

        }

    }
    return flag;    // en son flag return edilir
    /*
     * Eger her şey normal ise flag baslangictaki 1 degeridir
     * normal degil ise 0 olur
     */
}
// gelen sinyali handele eder
void signalHandler(int sigNo){
    sinyal = sigNo;
    remove(mainFile);
    remove(directorySize);
    remove("threadS");
    remove("size");
    remove("max");
}

int StringSize(char str[]){
    // Aranacak String boyutunu belirleme
    int size=0;
    for(size=0;str[size] !='\0';size++);
    return size;    //size return eder
}
void totalSize(char *fileName,char *word) {
    FILE * fp;
    char temp=' ';
    int totalSize=-1; /* en sondaki \n saymasın diye */
    fp =fopen(fileName,"r"); /* Log dosyasi acilir */
    while(!feof(fp)){
        fscanf(fp,"%c",&temp);
        if(temp == '\n')
            totalSize++;        /* kac tane String bulundugu hesaplanir */
    }
    fclose(fp);
    fp = fopen(fileName,"a+");  /* Dosya tekrar acilir */
    if(totalSize < 0)
        totalSize=0;            /* kactane String bulunmus ise onu yazar */
    printf("Total number of '%s' found : %d \n",word ,totalSize);
    fprintf(fp,"%d '%s' were found in total  \n",totalSize,word );
    fclose(fp);

}
// directory altında kaçtane file oldugunu bulur
int HowManyFile(char dir[]){
    int size = 0;
    struct dirent *direntp = NULL;
    DIR *dirp = NULL;
    if ((dirp = opendir(dir)) == NULL){
        printf("exit condition \n");
        exit(1);
    }/* eger yolda hata var ise */
    while ((direntp = readdir(dirp)) != NULL) {
        if(direntp->d_type == 8)
            size++;

    }
    while ((closedir(dirp) == -1));

    return size;
}
//directory path ini verilen char* atar
void fillFiles(char path[],char file[][PATH_MAX],int size,char onlyFileName[][PATH_MAX]){
   // printf("%s size %d \n",path,size);
    int count = 0;


    struct dirent *direntp = NULL;
    DIR *dirp = NULL;
    if ((dirp = opendir(path)) == NULL);/* eger yolda hata var ise */
    while ((direntp = readdir(dirp)) != NULL) {
        if(direntp->d_type == 8) { // regular file kocu
            char newPath[PATH_MAX];
            strcpy(newPath, path);
            strcat(newPath,direntp->d_name);
            strcpy(onlyFileName[count],direntp->d_name);
            strcpy(file[count],newPath);
            count++;
        }

    }
    while ((closedir(dirp) == -1));
}

// ilk odevlerde yapılan arama fonksyonu
// semophore kullanıldı iletişimi kendi özel dosyalar ile yapıldı
void *search(void *args){





    FILE *current;
    current = fopen("current","r");
    int current_size=0;
    fscanf(current,"%d",&current_size);
    fclose(current);
    current_size++;
    current = fopen("current","w");
    fprintf(current,"%d",current_size);
    fclose(current);
    current = fopen("max","r");
    int temp = 0;
    fscanf(current,"%d",&temp);
    fclose(current);

    if(current_size > temp){
        current = fopen("max","w");
        fprintf(current,"%d",current_size);
        fclose(current);
    }

    sem_wait(semName);

    numOfthread++;
    seachEnvironment *temp1 = (seachEnvironment *)args;
    char * fileName = temp1->pathName;
    int size = temp1->size;
    char *aranan = temp1->searchWord;
    char *onlyFileName = temp1->onlyFileName;

    FILE *file;
    file = fopen(fileName,"r");
    FILE * logfp;
    logfp = fopen("log.log","a+");
    char FirstCharacter = aranan[0]; //aranacak String ilk karakteri
    char readChar;  //Fscanf ile okunan 1 karakter
    int counter=0;  //Toplam kactane bulundugunu sayar
    int row = 1;    //satır sayısını hesaplama
    int col =1;     //sutun satısını hesaplama
    long currentPlace=0; // Dosyada bulunan yeri tutma
    int eof = 0;    // eger String fonksyonunda Dosya sonuna gelmis ise
    while(!feof(file) && eof==0){
        fscanf(file,"%c",&readChar);  //Dosyadan  character okuma
        if(readChar == '\n'){   //eger alt satır ise satır artır sutun sıfırla
            row++;

            col=1;
        }


        if(!feof(file)) {  // eger okunan karakter dosya sonu değilse

            if (readChar == FirstCharacter) { //okunan karakter arananın ilk karakterine eşitse
                currentPlace = ftell(file); // enson kalınan yerin yerini tutar
                int statu = IshasString(file,aranan,size);
                if(statu == 1 ){    //string Fileda var ise
                    counter++;
                    // string dogru oldugu icin satır ve sutunu ekrana yazar

                    fprintf(logfp," %d  %lu %s: [%d, %d] found  '%s' first character\n",getpid(),pthread_self(),onlyFileName,row, col,aranan);
                    fflush(logfp); // buffer bosaltır random yazma saglanır
                }
                else if(statu == -1){
                    eof = 1;    // dosyan sonunda ise

                }

                fseek(file, currentPlace, 0);   // dosyaya kaldigi yerden okumaya devam
            }
        }
        if(readChar != '\n')
            col++;  // \n de yukarda zaten 1 olmalı
    }





    fclose(file);
    fclose(logfp);
    sem_post(semName);
    current = fopen("current","r");
    fscanf(current,"%d",&current_size);
    fclose(current);
    current_size--;
    current = fopen("current","w");
    fprintf(current,"%d",current_size);
    fclose(current);


    pthread_exit(NULL);
}
/*
 * Gelen pathdeki dosyalar için thread oluşturur
 * daha sonra kalan directoryleri gezer
 */
void searcFiles(char * path,char word[],int size) {
    int fileSize = HowManyFile(path);   // directory içinde kaçtane file oldugu
    FILE *fp;
    fp = fopen(mainFile, "a+");
    fprintf(fp, "%d\n", fileSize);
    fclose(fp);
    pthread_t t1[fileSize]; //file kadar tread olusturulur
    char onlyFileName[fileSize][PATH_MAX];
    char files[fileSize][PATH_MAX];
    fillFiles(path, files, size,onlyFileName);     // directory içindeki fileları alır
    seachEnvironment vals[fileSize];  // vals structu
    for (int i = 0; i < fileSize; ++i) {
        strcpy(vals[i].fileName, files[i]);
        strcpy(vals[i].pathName, files[i]);
        vals[i].size = size;
        strcpy(vals[i].searchWord, word);
        strcpy(vals[i].onlyFileName,onlyFileName[i]);
    }
    // struct la threadler olusturulur
    for (int j = 0; j < fileSize; ++j) {

        pthread_create(&t1[j], NULL, search, &vals[j]);

    }



    /* file lar ile işlem bittikten sonra
     * eger var ise directory e geçilir
     * */

    struct dirent *direntp = NULL;
    DIR *dirp = NULL;

    if ((dirp = opendir(path)) == NULL) { /* eger yolda hata var ise */
        perror("FAİLED read Path");  /* perrror hata mesajı */
        exit(1);
    }

    while ((direntp = readdir(dirp)) != NULL) {  /* bos olana kadar verileri okt */
/* thread olusturulur ve arama islemi yapılır */

        if (strcmp("..", direntp->d_name) == 0 || strcmp(".", direntp->d_name) == 0) {
            /* Junk  ust dizin veya bulundugu deger*/
        } else {

            if (direntp->d_type == 4) { // directory kodu
                int summ = 0;
                /* eger directory ise mevcut proccess e fork yaparak torun olusturr */
                char newpath[256] = "";
                strcat(newpath, path);
                strcat(newpath, direntp->d_name);
                strcat(newpath, "/");

                int fd[2];
                pipe(fd);
                pid_t pid = fork();

                if (pid == 0) {
                    /* yol belirledikten sonra tekrar aynı fonksyona yollar */

                    numOfthread = 0;
                    //printf("new Fİle %s",newpath);
                    searcFiles(newpath, word, size);


                    FILE * fpThreadSize;
                    fpThreadSize = fopen("threadS","a+");
                    fprintf(fpThreadSize,"%d\n",numOfthread);
                    fclose(fpThreadSize);

                    int k = 1;
                    close(fd[0]);
                    write(fd[1], &k, sizeof(int));

                    close(fd[1]);


                   exit(1);
                } else if (pid < 0) {
                    printf("Fork Eror");
                    exit(2);
                } else {
                    wait(NULL);
                    int k = 0;
                    close(fd[1]);
                    read(fd[0], &k, sizeof(int));
                    close(fd[0]);

                    summ += k;
                    //printf("summ %d \n",summ);

                    FILE *fpTemp;
                    fpTemp = fopen(directorySize, "a+");
                    fprintf(fpTemp, "%d\n", summ);

                    fclose(fpTemp);


                }

            }


        }
        /*      Thread Beklenir     */
    }


    while (wait(NULL) != -1);
/* threadler oluşturulduktan sonra beklenir */
    for (int k = 0; k < fileSize; ++k) {

        pthread_join(t1[k], NULL);
    }


    while ((closedir(dirp) == -1));

    if (getpid() == mainpid) {  // ana directory içindeki file sayısı ogrenilir
        FILE *fp;
        fp = fopen("threadS", "a+");
        fprintf(fp, "%d\n", numOfthread);
        fclose(fp);
    }





}


int main(int argc,char *argv[]) {
    /* usage  */
    if(argc != 3) {
        fprintf(stderr, "Usage:%s 'string' directory_name\n", argv[0]);
        return 1;
    }

    semName = sem_open("/semaphore", O_CREAT, 0644, 1);


    /*
     * signal handler
     */
    signal(SIGINT,signalHandler);
    signal(SIGKILL,signalHandler);
    signal(SIGABRT,signalHandler);
    signal(SIGQUIT,signalHandler);


    /*
     *  aynı anda çalışan dosyaları bulma için oluşturlan
     *  geçici dosyalar
     */

    FILE *current;
    current = fopen("current","w");
    fprintf(current,"%d",0);
    fclose(current);
    current = fopen("max","w");
    fprintf(current,"%d",0);
    fclose(current);

    /*
     * timer
     */
    struct timeval t1;
    gettimeofday(&t1,NULL);
    long time = t1.tv_usec;

    /*
     * threadleri sayıları ile ilgili
     * haberleşmesi için gerekli geçici
     * dosyalar
     */

    strcpy(mainFile,"size");
    strcpy(directorySize,"dira");
    char * logFile ="log.log"; /* en son olusturacagi Log dosyasi */
    FILE *fp;

    /*
     * Log dosyasını sıfırlamak icin kullanılır
     * daha oncede calisacagi icin once log dosyasini temizler
     * yoksa acar
     */

    fp =fopen(logFile,"w+");
    fclose(fp); /* close file */
    /* Kullanıcıdan gelen direktory argümanı alır */
    char dirName[PATH_MAX]="";
    strcat(dirName,argv[2]);
    strcat(dirName,"/");
    /* ******************************************* */
    /* Aranacak kelimeyi word degiskenine alir */
    char * word = argv[1];
    int size = StringSize(argv[1]); /* gelen string boyutu */



    pid_t pid;
    /*
     * Main proccess den fork yapılır ve sonra o beklenir
     * o proccessin işlemi bittikten sonra dosya sonu oldugu
     * anlasilir ve en sona kactane bulunduysa o yazilir
     */
    pid = fork();
    if(pid == 0){  /* child ise dosya arama fonksyonuna gider */
        mainpid = getpid();
        searcFiles(dirName,word,size);
        exit(1);
    }
    else if(pid >1) {
        wait(NULL);
        totalSize(logFile, word);
    }



        /* 1kactane bulunmus ise onu yazar */


    else{
        printf("fork olusturma hatasi");
        exit(2);
    }
    FILE *fp2;


    fp2 = fopen(directorySize,"r");
    if(fp2 == NULL)
        return 1;
    printf("Number of directories searched: %d\n",total(fp2)+1);//ana ditectory oldugu icin
    fclose(fp2);
    remove(directorySize);


    fp2 = fopen(mainFile,"r");
    printf("Number of files searched: %d\n",total(fp2));
    fclose(fp2);
    remove(mainFile);
    fp2 = fopen("LineSize","r");
    printf("Number of lines searched: %d \n",total(fp2)+1);//hesaplama için gerekli
    fclose(fp2);
    remove("LineSize");

    fp2 = fopen("threadS","r");
    printf("Number of cascade threads created: %d \n",cascade(fp2));
    fclose(fp2);


    FILE *fptemp;
    fptemp = fopen("threadS","r");
    printf("Number of search threads created:   %d\n",total(fptemp));
    fclose(fptemp);
    remove("threadS");


    fp2 = fopen("max","r");

    printf("Max  of threads running concurrently:%d\n", total(fp2));
    fclose(fp2);
    remove("max");
    remove("current");

    struct timeval t2;
    gettimeofday(&t2,NULL);
    long time2 = t2.tv_usec;
    printf("Total run time, in milliseconds. :  %lu\n",(time2 -time)/1000);


    printf("Exit condition:  ");
    //sleep(5); //sinyali denemek için kullanılır
    if(sinyal == 0){
        printf("normal\n");
    }
    else if(sinyal > 0){
        printf("due to signal no %d\n",sinyal);
    }
    // error ile yap
    sem_close(semName);
    unlink("/semaphore");
    return 0;
}

int cascade(FILE *fp){
    int max = 0;
    while(!feof(fp)){
        int temp = 0;
        fscanf(fp,"%d",&temp);
        if(temp > max){
            max = temp;
        }


    }
    return max;

}
