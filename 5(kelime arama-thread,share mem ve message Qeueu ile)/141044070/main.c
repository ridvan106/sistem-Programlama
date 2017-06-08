/* *********************************************************
 * Rıdvan Demirci 141044070                                *
 *      Sistem Programlama Homework 5                      *
 * ******************************************************* */
#define _POSIX_SOURCE
#define _GNU_SOURCE
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/time.h>
#define Path 256   /* maximum alacagi path */
/*      Thread ile gonderilen struct    */
typedef struct {
    char pathName[PATH_MAX];
    char searchWord[PATH_MAX];
    int size;
    char fileName[PATH_MAX];
    char onlyFileName[PATH_MAX];
}seachEnvironment;
int mainPid;
/**
 * Message Queue'da haberleşilecek
 * struct
 */
typedef struct {
    int size;
    int ShareMemory;
    int flag;
}conn;
/*  *   *   *   SIGNATURE*  *   *   *   *       **/
int hasDIr(char dir[]);
int SumFile(char fileName[]);
void fillFiles(char path[],char file[][PATH_MAX],int size,char onlyFileName[][PATH_MAX]);
int HowManyFile(char dir[]);
void *search(void *args);
int isFile(char *str); /* Dosyanın file veya directory olup olmamasına bakar */
/*
 * proccess olarak ilk ödevdeki dosyalarda arama yapar eger directory ise
 * tekrar en basa recursive call yapilir
 */
void searcFiles(char * path,char word[],int size);
void PrintFile(char file[]);
int StringSize(char str[]);
/*
 * En son file sonuna kaçtane string buldugunu yazar
 */
int totalSize(char *fileName);
/*
 * Main
 */
sem_t sem1;
FILE *filess;
char FileSize[PATH_MAX];
FILE *directory;
char DirectorySize[PATH_MAX];
FILE *fCascade;
char CascadeSize[PATH_MAX];
char running[PATH_MAX];
char ControlRunning[PATH_MAX];
int valRun;
char ShareSize[PATH_MAX];
sem_t *named;   //isimli semaphore için
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Linemutex = PTHREAD_MUTEX_INITIALIZER;
int TempCascade;
int sinyal = 0;
void signalHandler(int sigNo){
    remove(DirectorySize);
    remove(FileSize);
    remove("threadS");
    remove(CascadeSize);
    remove(ControlRunning);
    remove(running);
    sinyal = sigNo;
}
int main(int argc , char *argv[]) {
    signal(SIGINT,signalHandler);
    signal(SIGKILL,signalHandler);
    signal(SIGABRT,signalHandler);
    signal(SIGQUIT,signalHandler);


    /* usage  */
    if(argc != 3) {
        fprintf(stderr, "Usage: %s 'string' directory_name\n", argv[0]);
        return 1;
    }
    /** *   *   ShareSizeları dosyaya kayıt edilir   *   *   *   */
        strcpy(ShareSize,"ShareSizes");


    char tempsemo[PATH_MAX];
    sprintf(tempsemo,"%d",getpid());
    char semo[PATH_MAX];
    strcpy(semo,"/");
    strcat(semo,tempsemo);

    //printf("smophore %s \n",semo);
    named = sem_open(semo,O_CREAT|O_RDWR, 0666,1);

    /** *   *   *   *   *   *   *   *   *   *   */



    /**     Timer                   */
    struct timeval t1;
    gettimeofday(&t1,NULL);
    double time = t1.tv_usec/1000;
    /** *   *   *   *   *   *       *   */
    /**
     * Aynı Anda Çalışan max thread sayısını bulmak için */
     valRun = 0;
    strcpy(running,"running");
    strcpy(ControlRunning,"controlRunning");
    FILE *temp = fopen(running,"w");
    fprintf(temp,"%d",0);
    fclose(temp);
    temp = fopen(ControlRunning,"w");
    fprintf(temp,"%d",0);
    fclose(temp);
    /**     *   *   *   *   *   *   *   *       *   **   */



    strcpy(CascadeSize,"cascade");
    fCascade = fopen(CascadeSize,"a+");
    fprintf(fCascade,"%d",0);
    fclose(fCascade);
    TempCascade = 0;
    strcpy(DirectorySize,"directorySize");
    strcpy(FileSize,"fileSize");
    filess = fopen(FileSize,"a+");
    //directory = fopen(DirectorySize,"a+");
    char * logFile ="log.txt"; /* en son olusturacagi Log dosyasi */
    FILE *fp;

    /*
     * Log dosyasını sıfırlamak icin kullanılır
     * daha oncede calisacagi icin once log dosyasini temizler
     * yoksa acar
     */

    fp =fopen(logFile,"w+");
    fclose(fp); /* close file */
    /* Kullanıcıdan gelen direktory argümanı alır */
    char dirName[Path]="";
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
        mainPid = getpid();
        searcFiles(dirName,word,size);

        /*
         * En son gelen main pid si ile message Queuedan veri alınır
         */
        if(mainPid == getpid()) {
            key_t key = getppid();
            int Qeueid = msgget(key, IPC_CREAT | 0666);
            conn veri;
            msgrcv(Qeueid, &veri, sizeof(conn), 0, 0);
            /*
             * en son kac adet bulundugu ekrana ve log dosyasına yazılır
             */
            printf("Share Memory Size = %d -- Sizes = ",veri.ShareMemory);
            PrintFile(ShareSize);
            remove(ShareSize);
            printf("Total number of '%s' found : = %d \n", word,veri.size);
            FILE *log;
            log = fopen(logFile,"a+");
            fprintf(log,"%d '%s' were found in total  \n",veri.size,word );
            fclose(log);
            sem_close(named);
        }


    }
    else if(pid >1) {

        wait(NULL);
        printf("Number of directories searched: %d \n",totalSize(DirectorySize)+1);
        // ana directoryde eklenir
        remove(DirectorySize);


        printf("Number of File searched: %d \n",totalSize(FileSize));
        fclose(filess);
        remove(FileSize);

        printf("Number of lines searched: %d \n",SumFile("LineSize"));
        remove("LineSize");

        printf("Number of search threads created: %d \n",totalSize("threadS"));
        remove("threadS");
        fCascade =fopen(CascadeSize,"r");
        int cascade = 0;
        fscanf(fCascade,"%d",&cascade);
        fclose(fCascade);
        printf("Number of cascade threads created: %d \n",cascade);
        remove(CascadeSize);
        FILE *max = fopen(ControlRunning,"r");
        int Smax = 0;
        fscanf(max,"%d",&Smax);
        fclose(max);
        printf("Max # of threads running concurrently: %d\n",Smax);
        remove(ControlRunning);
        remove(running);

        struct timeval t2;
        gettimeofday(&t2,NULL);
        double time2 = t2.tv_usec/1000;
        printf("Total run time, in milliseconds. :  %lf\n",time2 -time);

        printf("Exit condition:  ");
      //  sleep(5); //sinyali denemek için kullanılır
        if(sinyal == 0){
            printf("normal\n");
        }
        else if(sinyal > 0){
            printf("due to signal no %d\n",sinyal);
        }
        sem_close(named);
        sem_unlink("/semo");
        // error ile yap
      //  unlink("/semo");

    }

    else{

        printf("fork olusturma hatasi");
        exit(2);
    }






    return 0;
}
/*
 * Kac tana string bulundugunu hesap eder ve onu
 * ekrana yazar
 */
int totalSize(char *fileName) {
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
    return totalSize;
}
/*
 * Gelen path in file mi yoksa directory
 * olup olmamasına bakılır
 */
int isFile(char *str){
    int i = 0;
    for (i = 0; str[i]!='\0'; ++i) {
        if(str[i] == '.')   /* nokta aramasi yapar */
            return 1;
    }
    return 0;
}
/*
 * Verilen path de dirent structi ile
 * dongu olusturur eger file ise fork ile
 * proccess olusturur ve arama yapar
 * direktory ise fork ile tekrar basa doner
 * torun olusur
 *
 */

void searcFiles(char * path,char word[],int size){

    //gelen proccess in pid degeri alınır
    int PID = getpid();
    //Dizin içinde folder Sayısını bulur
    int folderSize = hasDIr(path);

    int flag = 0;
    /*
     * Eger folder var ise  flag degeri 1 olur
     */
    if(folderSize == 0){
        flag =1;

    }else { //eger directory var ise

        struct dirent *direntp = NULL;
        DIR *dirp = NULL;
        if ((dirp = opendir(path)) == NULL) { /* eger yolda hata var ise */
            perror("FAİLED read Path");  /* perrror hata mesajı */
            return;
        }
        while ((direntp = readdir(dirp)) != NULL) {  /* bos olana kadar verileri okt */

            if (strcmp("..", direntp->d_name) == 0 || strcmp(".", direntp->d_name) == 0) {
                /* Junk  ust dizin veya bulundugu deger*/
            } else {

                /**
                 * Directory ise  tekrar proccess olusturup
                 * aynı dizin cagrilir
                 * yeni path verilerek
                 */
                if (!isFile(direntp->d_name)) {

                    char newpath[256] = "";
                    strcat(newpath, path);
                    strcat(newpath, direntp->d_name);
                    strcat(newpath, "/");
                    // Proccess olusturma
                    pid_t pid = fork();

                    if (pid == 0) {
                        directory = fopen(DirectorySize,"a+");
                        /** Directory oldugu için sayıysını 1 artırır*/
                        fprintf(directory,"%d %s \n",1,newpath);
                        fclose(directory);
                        /* yol belirledikten sonra tekrar aynı fonksyona yollar */
                        searcFiles(newpath, word, size);
                        // isini bitirdikten sonra donguden cikar
                        sem_close(named);   //isimli semophore proccess işi bittikten sonra kapatılır
                        break;
                    } else if (pid < 0) {
                        printf("Fork Eror");
                        exit(2);
                    } else {
                        //Ana proccess yeri


                    }

                }

            }


        }//Dırectory arama Dongu bitisi


        while (wait(NULL) != -1);// olusturulan proccessler beklenir
        //Acilan Dosyalar kapatılır
        while ((closedir(dirp) == -1) && (errno == EINTR)) ;
    }

    /*
     * Bu ifin içine sadece bu fonksyonu olusturan
     * proccess girebilir
     */
    if(PID == getpid()) {
        // O dizin içindeki bulunan sayi
        int ToplamTemp = 0;

        int fileSize = HowManyFile(path);   // directory içinde kaçtane file oldugu




        pthread_t *t1;
        t1 =(pthread_t *)malloc(sizeof(pthread_t)*fileSize);
        char files[fileSize][PATH_MAX];
        char onlyFileName[fileSize][PATH_MAX]; // sadece Dosya isimlerini alır
        fillFiles(path, files, size, onlyFileName);     // directory içindeki fileları alır
        seachEnvironment *vals;   //Threadlere gonderilicek struct
        vals =(seachEnvironment *)malloc(sizeof(seachEnvironment)*fileSize);
        for (int i = 0; i < fileSize; ++i) {
            strcpy(vals[i].fileName, files[i]);
            strcpy(vals[i].pathName, files[i]);
            vals[i].size = size;
            strcpy(vals[i].searchWord, word);
            strcpy(vals[i].onlyFileName, onlyFileName[i]);
        }


        /**
         * Thhreadlerin haberleşmesi için
         * oluşturulan Share Memory
         * */
        key_t key = (int) getpid();
        int shmid = shmget(key,fileSize *(int)sizeof(int), IPC_CREAT | 0666);
        /** *   *   Size Share Memorye yazılı   *   *   *   */
        FILE *ShareFile;
        ShareFile = fopen(ShareSize,"a+");
        fprintf(ShareFile,"%d\n",fileSize * (int)sizeof(int));
        fclose(ShareFile);

        /** *   *   *   *   *   *   *   *   *   *   *   *   */
        char *adetSayisi;
        adetSayisi = shmat(shmid, NULL, 0);
        /**
         * Threadler için Semaphore olusturulur
         * search içindeki critical bolge için
         */
        sem_init(&sem1, 0, 1);
        valRun = 0;
        for (int j = 0; j < fileSize; ++j) {

            pthread_create(&t1[j], NULL, search, &vals[j]);

        }
        int counter = 0;
        /**
         * threadlerden gelen size degerleri
         * counter'a atılır
         */
        for (int k = 0; k < fileSize; ++k) {

            pthread_join(t1[k], NULL);
            // printf("sayac = %d \n",vals[k].size);
            counter += vals[k].size;

        }
        free(t1); // threadler free edilir
        free(vals); // threadler free edilir
        fCascade = fopen(CascadeSize,"r");
        int temo;
        fscanf(fCascade,"%d",&temo);
       // printf("temo %d \n",TempCascade);
        fclose(fCascade);

        if (TempCascade > temo){
            fCascade = fopen(CascadeSize,"w");
            fprintf(fCascade,"%d\n",TempCascade);
            fclose(fCascade);
        }
        TempCascade = 0;
        fclose(filess);

        /* Thread den alınan degerler Share memorye atılır */
        char num[sizeof(counter)];

        sprintf(num, "%d", counter);
        memcpy(adetSayisi, num, strlen(num)); // Deger Share Memorye atılır
        sem_destroy(&sem1);
       // printf("shared memory %d  pid %d  \n", counter,getpid());
        /** * * * * *    *   *   *   *       *   *   *   *   *   */

        /**
         * dizinde enson file lara bakılır ve flag 1 yapılır
         * Queue okunurkende flag'e bakılır ona göre message Queue dan
         * okuma durdurulur
         *
         * Eger dizinde sadece File var ise
         * Share Memory deki deger
         * Ust proccesse Message Queue ile aktarılır
         * */
        if(flag == 1){
            conn data;
            data.ShareMemory = 1;
            data.size = counter;    //memory shared den alındı
            data.flag = 0;  // flag Sıfır cünkü eklenen son deger degil
            key_t  Pkey = getppid();    // ust proccessin Key degeri
            int Queuid = msgget(Pkey,IPC_CREAT | 0666);
            // data structu ust proccesse aktarılır
            msgsnd(Queuid,&data,sizeof(data),0);

        }
        /**    *   *   *   *   *   *   *   *   *   *       **/

            /** Ege dizin de Dırectoyler var ise */

        else {

            conn data;

            key_t key = getpid();
           // printf("pid %d \n",getpid());
            /**
             * Share memoryden alınan deger Queunun sonuna flag 1 olarak
             * eklenir
             * Boylece enson flag 1 ile okuma islemi durur
             */
            int Queuid = msgget(key, IPC_CREAT | 0666);
            data;
            data.size = counter; //memory shared den alındı
            data.ShareMemory = 0;
            data.flag = 1;
            msgsnd(Queuid,&data,sizeof(data),0);

            /** Gonderme veriler Queue ya eklendi
             *
             ** * *   *   *   *   *       *   */


            /**
             * Queuedaki tüm veriler okunur
             *
             */
            int toplam = 0;
            int ShareSize=0;
            while (1) {
                msgrcv(Queuid, &data, sizeof(data), 0, 0);
                toplam += data.size;
                ShareSize += data.ShareMemory;
                if (data.flag == 1)
                    break;
            }
            /**
             * Queudaki okunan veriler
             * bir ust Message Queue ya aktarılır
             */
            ToplamTemp = toplam;
            data;
            data.ShareMemory = ShareSize+1;
            data.size = ToplamTemp;
            data.flag = 0;  // Flag 0 cünkü Ustteki message Queue nun sonu degil
            key_t  keyy = getppid();
            Queuid = msgget(keyy,IPC_CREAT | 0666);
            msgsnd(Queuid,&data,sizeof(data),0);
        }

    }


}
int StringSize(char str[]){
    // Aranacak String boyutunu belirleme
    int size=0;
    for(size=0;str[size] !='\0';size++);
    return size;    //size return eder
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

void *search(void *args){
    /*      Threadler Buraya geldimi Listeyi artırmalı
     *  gelen threadler isimli threade girer boylece
     *  farklı proccesslerin threadleride handle edilir
     * */
    sem_wait(named);
    FILE* temp = fopen(running,"r");
    fscanf(temp,"%d",&valRun);
    fclose(temp);
    valRun++;
    temp = fopen(running,"w");
    fprintf(temp,"%d",valRun);
    fclose(temp);

    temp =fopen(ControlRunning,"r");
    int t = 0;
    fscanf(temp,"%d",&t);
    fclose(temp);
    if(valRun > t){
        temp = fopen(ControlRunning,"w");
        fprintf(temp,"%d",valRun);
        fclose(temp);
    }
   sem_post(named);




    /*  *   *   *   *   *   *   *   *   *   *   *   */


    /*
     * toplam Çalışan threadSize belirlemek için
     */
    pthread_mutex_lock(&mutex);
    FILE *thred;
    thred = fopen("threadS","a+");
    fprintf(thred,"%d \n",1);
    fclose(thred);
    TempCascade++;
    pthread_mutex_unlock(&mutex);


    /**
     * File işlemi için açılan semaphore
     */
    sem_wait(&sem1);

    int *total;
    fprintf(filess,"%d \n",1);
    seachEnvironment *temp1 = (seachEnvironment *)args;
    char * fileName = temp1->pathName;
    int size = temp1->size;
    char *aranan = temp1->searchWord;
    char *onlyFileName = temp1->onlyFileName;

    pthread_mutex_lock(&Linemutex);//row sayısını almak için critical section olusturulur
    FILE * LineSize;
    LineSize = fopen("LineSize","a+");
    fprintf(LineSize,"%d\n",totalSize(fileName)); // file hesabına gore
    fclose(LineSize);
    pthread_mutex_unlock(&Linemutex);

    FILE *file;
    file = fopen(fileName,"r");
    FILE * logfp;
    logfp = fopen("log.txt","a+");
    char FirstCharacter = aranan[0]; //aranacak String ilk karakteri
    char readChar;  //Fscanf ile okunan 1 karakter
    int counter=0;  //Toplam kactane bulundugunu sayar
    total = &counter;
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
                    fprintf(logfp,"%d  %ld  %s: [%d, %d] found  '%s' first character\n",getpid(),pthread_self(),onlyFileName,row, col,aranan);
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
    } fclose(file);
    fclose(logfp);
    sem_post(&sem1);
    temp1->size = counter; // structaki size değiri buldugu sayıya ekler

    //aynı anda çalışan thread sayısını bulmak için
    sem_wait(named);
    temp = fopen(running,"r");
    int decrement = 0;
    fscanf(temp,"%d",&decrement);
    fclose(temp);
    decrement--;
    temp = fopen(running,"w");
    fprintf(temp,"%d",decrement);
    fclose(temp);
    sem_post(named);
    //  *   *   *   *   *   *   *   *   *
    return NULL;
}
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
int hasDIr(char dir[]){
    int size = 0;
    struct dirent *direntp = NULL;
    DIR *dirp = NULL;
    if ((dirp = opendir(dir)) == NULL){
    printf("exit condition \n");
    exit(1);
    }/* eger yolda hata var ise */
    while ((direntp = readdir(dirp)) != NULL) {
     if(!isFile(direntp->d_name)) {
         size++;
     }

    }
    while ((closedir(dirp) == -1));
   // printf(" %s size %d",dir,size);
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
int SumFile(char fileName[]){
    int sum = 0;
    int temp = 0;
    FILE *Ftemp = fopen(fileName,"r");
    while (!feof(Ftemp)){
        fscanf(Ftemp,"%d",&temp);
        sum += temp;
        temp = 0;
    }
    fclose(Ftemp);
    return  sum;
}
void PrintFile(char file[]){
    FILE *temp;
    temp = fopen(file,"a+");
    int t=0;
    while(!feof(temp)){
        fscanf(temp,"%d",&t);
        if(!feof(temp))
            printf("%d ",t);
        t=0;
    }
    printf("\n");
    fclose(temp);
}