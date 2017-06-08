#define _POSIX_SOURCE
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>

#include <arpa/inet.h>
#include <sys/stat.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Mqueu = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Mservece = PTHREAD_MUTEX_INITIALIZER;

int portNO;
int mainPid;
int servece;
/** *   *   *
 * http://bilgisayarkavramlari.sadievrenseker.com/2008/04/16/sira-queue/
 * Worker Pool için Qoueue *   *   *   *   */
int *a;     /* Queunun */
int Queusize=0;
int Quecapasity=10;
/* size kapasiteye yaklasinca eklstra yer alma */
void initQuueu(){
    a =(int *) malloc(sizeof(int)*Quecapasity);
}
void reallocate(){
    pthread_mutex_lock(&Mqueu);

    Quecapasity = Quecapasity *2;
    int i=0;
    int *temp = malloc(sizeof(int)*Queusize);
    for (i = 0; i < Queusize; ++i) {
        temp[i] = a[i];
    }
    free(a);
    int j=0;
    a = malloc(sizeof(int)*Quecapasity);
    for (j = 0; j < Queusize; ++j) {
        a[j] = temp[j];
    }
    free(temp);
    pthread_mutex_unlock(&Mqueu);
}
/*Queue ya veri ekleme ve size artırma */
void enqueue(int data){
    pthread_mutex_lock(&Mqueu);
    if(Quecapasity - Queusize == 2){
        reallocate();
    }

    a[Queusize++] = data;
    pthread_mutex_unlock(&Mqueu);
}
/* Queudan veri cikarma ve size azaltma */
int dequeue(){
    pthread_mutex_lock(&Mqueu);

    if(Queusize<=0)
        return -1;

    int result = a[0];
    int i=0 ;
    for (i = 0; i < Queusize; ++i) {
        a[i] = a[i+1];
    }
    Queusize--;
    pthread_mutex_unlock(&Mqueu);
    return result;
}
//**    *   *End OF Queue   *   *   *   *   */
/**
 *  Haberleşmede Kullanılan Yapılar
 */
typedef struct{
    int row;
    int col;
    long int threadId;
    int pid;
}dataOfMatrix;
typedef struct {
    double *val;

}matrix;

typedef struct {
    int row;
    int col;
    int order;
}paket;
/**
 *  *   *   *   *   *   *       *   *   **
 */

/**
 * proccesslerde kullanılacak Global matrixler
 */

matrix *matrixOfA,*matrixOfB;
matrix *result1,*result2,*result3;

/**
 *
 * SIGNUTURE
 */
//workerpool threadleri için method
void *worker(void* args);
// Hata hesapları için
void calculateError(matrix result1[],matrix result2[],matrix result3[],double *er1,double *er2,double *er3,int row,int col);
// iki matrixi çıkarıp resulta atar
void difference(matrix matrix1[],matrix matrix2[],matrix result[],int row,int col);
/*
 * Githubdan alınan SVD hesabı
 * https://github.com/baiyangbupt/SVD-Decomp
*/
void SVDCalculate(matrix matrix1[],matrix matrixOfB[],int row,int col);
/*
 * 2.proccessin kullandığı solve methodu
 */
void *solveMatrix(void *args);
//matrixi sıfırlar
void setTozero(matrix matrix1[],int row,int col);
//solution
void psudoInverse(matrix matrix1[],matrix matrixOfB[],int row,int col);
//matrix tersi alma
//http://bilgisayarkavramlari.sadievrenseker.com/2008/11/19/matrisin-tersinin-alinmasi-mantrix-inverse/
void intverse(matrix deneme[],int detS);
//2 matrixi çarpıp üçüncüye atılır
//http://bilgisayarkavramlari.sadievrenseker.com/2010/04/02/matris-carpimi-matrix-multiplication/
void productTwoMatrix(matrix matrix1[],matrix matrix2[],matrix result[],int row,int col1,int col2);
//iki matrixi toplar 3.ye atar
void sumOfMatrix2(matrix *matrix1, matrix *matrix2, int row, int whicCol);
void sumOfMatrix(matrix *matrix1, matrix *matrix2, int row, int whicCol);
//matrixi constant ile toplar
void constantProductMatrix(matrix temp[],matrix matrix1[],int row,int whichCol,double constant);
double sumOfLine(matrix matrix1[],matrix matrix2 [],int row,int WhichcolOne,int whichcolTwo);
//QR factorizasyon çözümü
void QRfactorization(matrix matrixOfA[],matrix matrixOfB[],int row,int col);
// genel hesaplama fonksyonu
void calculatesMatrixes(int row,int col,int socket,long int gelenThread,int pidd);
//servece sayısı azaltır
void serveceDec();
void serveceInc();
//  *   *   *   *   *   *   *
// Gelen pidListesini file yazar
void writeList(int num);
//matrixi mesage ile file yazar
void printFile(char *massge,matrix matrix1[],int row,int col);
//listedeki pidlerioldurur
void killClients();
/** **  *   *   *   *   *   *   *   *   *   *   *       **/
// raskele matrix olusturur
void matrixGenaration(matrix temp[],int col,int row){


    struct timeval tpstart1;
    gettimeofday(&tpstart1, NULL);
    long timestart1 = tpstart1.tv_usec;

    srand(timestart1);   /* microsecond cinsinden raskele sayılar uretilir */

    for (int j = 0; j < row; ++j) {
        for (int i = 0; i < col; ++i) {
            temp[j].val[i] = rand() %70+1;
        }
    }


    //free yapılacak


}
void printMatrix(matrix matrix1[],int row,int col){

    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < col; ++j) {
            printf("%lf ",matrix1[i].val[j]);
        }
        printf("\n");
    }

    printf("calculate2 %d \n",(int)1234);

}
/*
 * Thread per request için kullanılan client fonksyonu
 */
void * foo(void *args){

    int *newSocket = (int *)args;
    dataOfMatrix data;


    while(1) {
        int recSize = (int) recv(*newSocket, &data, sizeof(data), 0);
        if (recSize > 0) {
           // fprintf(stderr, "okunan %d  row %d col %d\n", recSize, data.row, data.col);
            //fprintf(stderr, "thred gelen %ld  - %ld  \n", data.threadId,pthread_self());

            serveceInc(); // hizmet edilen client artırılır
            calculatesMatrixes(data.row,data.col,*newSocket,data.threadId,data.pid);
            serveceDec(); // Azaltılır
            break;

        }
    }
    //Client dan senkranizosyon amaçlı deger alır
    int control = 0;
    while(control != 1) {
        recv(*newSocket, &control, sizeof(int), 0);

    }
   // usleep(100);

    close(*newSocket);  //socket kapanır

}
/*
 * hizmet edilen client sayısı ile
 *
 * interrupt sinyali için kullanılan method
 */
void signalHandler(int signo){
    if(signo == SIGINT) {
        printf("signal geldi \n");
        close(portNO);
        killClients();
        remove("pidList");
        exit(1);
    }
    else if(signo == SIGUSR1){
        if(servece >= 1)
        fprintf(stderr,"Hizmet Edilen Thread Sayısı %d \n",--servece);
    }
    else if(signo == SIGUSR2){
        fprintf(stderr,"Hizmet Edilen Thread Sayısı %d \n",++servece);
    }
}

//Githubdan alınan kod
//https://github.com/baiyangbupt/SVD-Decomp
#include "svdcmp.c"

int main(int argc ,char *argv[]) {

    if(argc != 3){
        printf("usage:./server <serverPort> <poolSize>\n");
        return 0;
    }
    /**
     * signal handler
     */

    signal(SIGINT,signalHandler);
    signal(SIGUSR1,signalHandler);
    signal(SIGUSR2,signalHandler);
    mainPid =getpid();
    servece  =0;
    mkdir("serverLog",0777);/* Log Direktorysinin oluşturulması */


    portNO = atoi(argv[1]);
    int poolSize = atoi(argv[2]);
    printf("portNo %d  size %d \n",portNO,poolSize);

    printf("Hizmet Edilen Thread Sayısı %d \n",servece);


    struct sockaddr_in serverInfo,clientInfo;

    int socketNo = socket(AF_INET,SOCK_STREAM,0);
    //yap
    //setsockopt(socketNo,SOL_SOCKET,SO_REUSEADDR,NULL,0);
    if(socketNo < 0){
        perror("socket Error");
        return 0;
    }
    serverInfo.sin_port = htons(portNO);
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
    // server adresinin size bilgisini öğrenme
    socklen_t size = sizeof(serverInfo);

    if(bind(socketNo,(struct sockaddr *)&serverInfo,size)){
        perror("bind error");
        close(socketNo);
        return 0;
    }
    if(listen(socketNo,10) !=0){
        perror("listenning error");
    }
    int CLIENTSIZE = 100;
    int newSocket;
    /**
     * eger pool size <= 0 ise per request tarafına girer
     */

    if(poolSize <= 0){
        printf("thread-per-request \n");
        int sayac = 0;

        socklen_t clientSize = sizeof(clientInfo);

        int *socket;
        socket = (int *)malloc(sizeof(int)*CLIENTSIZE);

      //  pthread = (pthread_t *)malloc(sizeof(pthread_t)*CLIENTSIZE);
        while (1){
            //request yakalayıcı
            //her Request için thred oluşturur

        newSocket = accept(socketNo,(struct sockaddr *)&clientInfo,&clientSize);

            socket[sayac] = newSocket;

            pthread_t pthread;
            pthread_create(&pthread,NULL,foo,&socket[sayac]);


            /**
             * Client size doldugunda reallocate yAPILIR
             */

            if(CLIENTSIZE - sayac == 2){
                CLIENTSIZE *=2;
                int orgin = CLIENTSIZE/2;
                int temp[orgin];
                pthread_t tempThread[orgin];
                for (int i = 0; i <orgin ; ++i) {
                    temp[i] = socket[i];
               //     tempThread[i] = pthread[i];

                }
                free(socket);
               // free(pthread);
                socket = (int *)malloc(sizeof(int)*CLIENTSIZE);
             //   pthread = (pthread_t *)malloc(sizeof(pthread_t)*CLIENTSIZE);
                for (int j = 0; j < orgin; ++j) {
                    socket[j] = temp[j];
                //    pthread[j] = tempThread[j];
                }

            }
            /**
             * *    *   *   *   *       *   *   *   *
             */



            sayac++;


        }



    }
    else {
        printf("WORkER POLL \n ");

        socklen_t clientSize = sizeof(clientInfo);
        initQuueu();    //Queue yu ilklendirme

        //poolSize kadar thread olusturulur

        pthread_t pthread[poolSize];
        for (int i = 0; i < poolSize; ++i) {
            pthread_create(&pthread[i],NULL,worker,NULL);
        }


        // gelen request Queue ya eklenir
        while(1) {

            newSocket = accept(socketNo, (struct sockaddr *) &clientInfo, &clientSize);
            enqueue(newSocket);

        }


        return 0;
    }
}
/*
 * matrixlerin hesap kısmı
 * 3 proccess olusturulur
 * genarete ve hesaplamalar burada yapılır
 */
void calculatesMatrixes(int row,int col,int socket,long int gelenThread,int pidd){

    pthread_mutex_lock(&mutex);
    srand(time(NULL));

    int pid = fork();
    /**
     * Genarate methodunun olusturulması
     */

    if(pid == 0){
        free(a);    //Queue bosaltılır
        writeList(pidd);    //Listeye  pidsi yazılır kill sinyali için
        /**
         * Matrixlerin olusturlması
         */

        matrixOfA = (matrix *)malloc(sizeof(matrix)*row);
        matrixOfB = (matrix *)malloc(sizeof(matrix)*row);
        for (int j = 0; j < row; ++j) {
            matrixOfA[j].val = (double *)malloc(sizeof(double)*col);
            matrixOfB[j].val = (double *)malloc(sizeof(double)*1);
        }
        matrixGenaration(matrixOfA,col,row); // matrixe raskele degerler atama

        key_t key = getppid();  //share memeory key
        char *sender;   //Buffer
        //share memeory
        int shmid = shmget(key,1000,IPC_CREAT | 0666);
        if(shmid < 0){
            perror("hata \n");
        }

        //memory Attack
        sender=(char *) shmat(shmid,NULL,0);

        /*
         * matrix degerlerinin Share memorye yazılması
         * yazıldıktan sonra okuanana kadar bekler
         */
        for (int j = 0; j < row ; ++j) {
            for (int i = 0; i < col; ++i) {
                sprintf(sender,"%d",(int)matrixOfA[j].val[i]);
                while(strcmp(sender,"*") !=0){
                    usleep(200);
                }

            }
        }
        usleep(200);
        //matrixB nin olusturulması
        matrixGenaration(matrixOfB,1,row);

        /*
                * matrix degerlerinin Share memorye yazılması
                * yazıldıktan sonra okuanana kadar bekler
                */
        for (int j = 0; j < row; ++j) {

            sprintf(sender,"%d",(int)matrixOfB[j].val[0]);
               while(strcmp(sender,"*") !=0){
                    usleep(200);
               }
        }

        /**
         * matrixlerin realloce edilmesi
         */
        for (int j = 0; j < row; ++j) {
            free(matrixOfA[j].val);
            free(matrixOfB[j].val);
        }

        free(matrixOfA);
        free(matrixOfB);

        exit(1);
    }

        int pid2 = fork();
        if(pid2 == 0){
            free(a);
            // share memory için

            key_t key = getppid();
            char * getter;
            int shmid =-1;
            /**
             * eger share memory olusmamıs ise olusana
             * kadar bekler
             */
            while(shmid<0){
            	shmid = shmget(key,1000,0666);
            	usleep(200);
            }
            /**
             * matrixler için yer alınması
             */


            matrixOfA = (matrix *)malloc(sizeof(matrix)*row);
            matrixOfB = (matrix *)malloc(sizeof(matrix)*row);
            for (int j = 0; j < row; ++j) {
                matrixOfA[j].val = (double *)malloc(sizeof(double)*col);
                matrixOfB[j].val = (double *)malloc(sizeof(double)*1);
            }

            int colTemp = 0;
            int rowTemp = 0;
                getter =(char*) shmat(shmid, NULL, 0);
            /**
             * share memorye deger gelene kadar bekler
             * A matrixi alınır
             */
            for (int i = 0; i < row*col; ++i) {

                while(strcmp(getter,"*") == 0){
                    usleep(200);
                }
                matrixOfA[rowTemp].val[colTemp] = atof(getter);
                colTemp++;

                strcpy(getter,"*");

               if((i+1)%col == 0) {
                   colTemp = 0;
                   rowTemp++;
               }
            }

            rowTemp = 0;
            /**
             * share memorye deger gelene kadar bekler
             * B matrixi alınır
             */

            for (int i = 0; i < row; ++i) {

                while(strcmp(getter,"*") == 0){
                    usleep(200);
                }
               // printf("\n%s  ",getter);
                matrixOfB[rowTemp].val[0] = atof(getter);
                rowTemp ++;

                strcpy(getter,"*");

            }
            close(shmid);   //share memeory kapanır

            /**
             * result matrixlerini olusturma
             */

            result1 = (matrix *)malloc(sizeof(matrix)*col);
            result2 = (matrix *)malloc(sizeof(matrix)*col);
            result3 = (matrix *)malloc(sizeof(matrix)*col);
            for (int m = 0; m < col; ++m) {
                result1[m].val = (double *)malloc(sizeof(double)*1);
                result2[m].val = (double *)malloc(sizeof(double)*1);
                result3[m].val = (double *)malloc(sizeof(double)*1);
            }
            //paketler çözümü yapacak threadlere bilgiler gonderir
            //ve onun bilgilerini
            paket matrixes[3];



            pthread_t SolverTheread[3];
            /**
             * 3 adet thread olusturulur parelel
             */
            for (int k = 0; k < 3; ++k) {
                matrixes[k].col =col;
                matrixes[k].row = row;
                matrixes[k].order = k;
                pthread_create(&SolverTheread[k],NULL,solveMatrix,&matrixes[k]);
            }
            // onlar beklenir
            for (int l = 0; l < 3; ++l) {
                pthread_join(SolverTheread[l],NULL);

            }



            //p3 e gonderme işlemi
            // gelen matrixler ve hesaplanan sonuclar
            //p3 e gonderilir
            int shmid2 = shmget(key,1000,IPC_CREAT | 0666);
            char * valu = (char *)shmat(shmid2,NULL,0);
            *valu = '?';
            /**
             * result1 gonderme
             */

                for (int i = 0; i < col; ++i) {

                    while(*valu != '*') {
                        usleep(200);
                    }

                        sprintf(valu,"%lf",result1[i].val[0]);
                    }

            /**
             * result2 gonderme
             */
            for (int i = 0; i < col; ++i) {

                while(*valu != '*') {
                    usleep(200);
                }

                sprintf(valu,"%lf",result2[i].val[0]);
            }
            /**
             * result2 gonderme
             */
            for (int i = 0; i < col; ++i) {

                while(*valu != '*') {
                    usleep(200);
                }

                sprintf(valu,"%lf",result3[i].val[0]);
            }

            /**
             * A matrixini gonderme gonderme
             */
            for (int l1 = 0; l1 < row; ++l1) {
                for (int i = 0; i < col; ++i) {
                    while(*valu != '*') {
                        usleep(200);
                    }

                    sprintf(valu,"%lf",matrixOfA[l1].val[i]);
                }
            }
            /**
              * B matrixini gonderme gonderme
            */
            for (int i = 0; i < row; ++i) {

                while(*valu != '*') {
                    usleep(100);
                }

                sprintf(valu,"%lf",matrixOfB[i].val[0]);
            }



        /** *   *   *   realloc*   *   *   *   */
            for (int j = 0; j < row; ++j) {
               free(matrixOfA[j].val);
                free(matrixOfB[j].val);
            }
            for (int n = 0; n < col; ++n) {
                free(result1[n].val);
                free(result2[n].val);
                free(result3[n].val);
            }
            free(result1);
            free(result2);
            free(result3);
            free(matrixOfA);
            free(matrixOfB);


            exit(1);
        }
    int pid3 = fork();
    // verify proccesi
    if(pid3 == 0){
        free(a);
        key_t  key = getppid();

        int shmid2 = -1;


        while(shmid2<0){
            shmid2 = shmget(key,1000,S_IRUSR);
            usleep(200);

        }


        /*
         * matrixler için allocate
         */
        char * valu = (char *)shmat(shmid2,NULL,0);
        matrixOfA = (matrix *)malloc(sizeof(matrix)*row);
        matrixOfB = (matrix *)malloc(sizeof(matrix)*row);
        for (int j = 0; j < row; ++j) {
            matrixOfA[j].val = (double *)malloc(sizeof(double)*col);
            matrixOfB[j].val = (double *)malloc(sizeof(double)*1);
        }

        result1 = (matrix *)malloc(sizeof(matrix)*col);
        result2 = (matrix *)malloc(sizeof(matrix)*col);
        result3 = (matrix *)malloc(sizeof(matrix)*col);
        for (int m = 0; m < col; ++m) {
            result1[m].val = (double *)malloc(sizeof(double)*1);
            result2[m].val = (double *)malloc(sizeof(double)*1);
            result3[m].val = (double *)malloc(sizeof(double)*1);
        }
        /**
        * egerShare Memorye beklenen işaret yazıldıysa devam eder
         * yoksa bekler
        */
        while(1){
            if(*valu == '?'){
                *valu ='*';
                /**
                 * result1
                 */
                for (int i = 0; i < col; ++i) {
                    while(*valu=='*'){
                        usleep(200);
                    }
                    result1[i].val[0] = atof(valu);
                    *valu ='*';
                }


                *valu =='*';
                /**
                 * result2
                                */

                for (int i = 0; i < col; ++i) {
                    while(*valu=='*'){
                        usleep(200);
                    }
                    result2[i].val[0] = atof(valu);
                    *valu ='*';
                }


                *valu =='*';

                /**
                 * result3
                                */
                for (int i = 0; i < col; ++i) {
                    while(*valu=='*'){
                        usleep(200);
                    }
                    result3[i].val[0] = atof(valu);
                    *valu ='*';
                }

                *valu =='*';
                /*
                 * A matrixini alma
                 */

                for (int i = 0; i < row; ++i) {
                    for (int j = 0; j < col; ++j) {
                        while(*valu=='*'){
                            usleep(200);
                        }
                        matrixOfA[i].val[j] = atof(valu);
                        *valu ='*';
                    }

                }
                *valu =='*';
                /*
               * B matrixini alma
               */
                for (int i = 0; i < row; ++i) {
                    while(*valu=='*'){
                        usleep(200);
                    }
                    matrixOfB[i].val[0] = atof(valu);
                    *valu ='*';
                }



                break;// dongu bire
            }
            usleep(200);
        }
        /***Error Hesabı
         * hatalar hesaplanıp degişkene atılır**/
        double err1,err2,err3;
        calculateError(result1,result2,result3,&err1,&err2,&err3,row,col);

        /**
         *  A matrixi Clienta gonderilir
         */
        for (int i1 = 0; i1 < row; ++i1) {
            for (int i = 0; i < col; ++i) {
                send(socket,&matrixOfA[i1].val[i],sizeof(double),0);
            }
        }

        /**
         *  A matrixi file Log'a
         */
        FILE *fp;
        char fName[256];
        char temp[256];
        sprintf(temp,"%d",getppid());

        strcpy(fName,"serverLog/");
        strcat(fName,temp);

        fp = fopen(fName,"a+");
        fprintf(fp,"Gonderilen Thread %ld \n",gelenThread);
        fclose(fp);


        printFile("matrixOfA",matrixOfA,row,col);
        /**
         * B matrixi gonderilir ve Loga yazılır
         */


        for (int i1 = 0; i1 < row; ++i1) {
            send(socket,&matrixOfB[i1].val[0],sizeof(double),0);

        }
        printFile("matrixOfb",matrixOfB,row,1);
        for (int k1 = 0; k1 < col; ++k1) {
            send(socket,&result1[k1].val[0],sizeof(double),0);
        }
        /**
        * result1 matrixi gonderilir ve Loga yazılır
        */

        printFile("result1",result1,col,1);
        for (int k1 = 0; k1 < col; ++k1) {
            send(socket,&result2[k1].val[0],sizeof(double),0);
        }
        /**
       * result2 matrixi gonderilir ve Loga yazılır
       */
        printFile("result2",result2,col,1);
        /**
       * result3 matrixi gonderilir ve Loga yazılır
       */
        for (int k1 = 0; k1 < col; ++k1) {
            send(socket,&result3[k1].val[0],sizeof(double),0);
        }
        printFile("result3",result3,col,1);

        send(socket,&err1,sizeof(double),0);
        send(socket,&err2,sizeof(double),0);
        send(socket,&err3,sizeof(double),0);


       /* printf("rıdvan Demirci \n");
        printMatrix(result1,col,1);
        printMatrix(result2,col,1);
        printMatrix(result3,col,1);
        printMatrix(matrixOfA,row,col);
        printMatrix(matrixOfB,row,1);*/

        shmctl(shmid2,IPC_RMID,NULL);   //share memory bosaltılır
        close(shmid2);
        /** *   *   *   * realloc()   *   *   *   */
        for (int j = 0; j < row; ++j) {
            free(matrixOfA[j].val);
            free(matrixOfB[j].val);
        }
        for (int n = 0; n < col; ++n) {
            free(result1[n].val);
            free(result2[n].val);
            free(result3[n].val);
        }
        free(result1);
        free(result2);
        free(result3);
        free(matrixOfA);
        free(matrixOfB);


        exit(1);
    }





    while(wait(NULL) != -1);

    pthread_mutex_unlock(&mutex);




}
/**
 * matrixi QR faktorization ile bulma methodu
 * @param matrixOfA
 * @param matrixOfB
 * @param row
 * @param col
 */
void QRfactorization(matrix matrixOfA[],matrix matrixOfB[],int row,int col){

    matrix *ortogolnalSet;
    ortogolnalSet = (matrix *)malloc(sizeof(matrix)*row);
    for (int i = 0; i < row; ++i) {
        ortogolnalSet[i].val =(double *)malloc(sizeof(double)*col);
        ortogolnalSet[i].val[0] = matrixOfA[i].val[0];  //basis ataması
    }
    for (int m = 0; m < row; ++m) {
        for (int i = 1; i < col; ++i) {
            ortogolnalSet[m].val[i] = 0;
        }
    }

    double temp,temp2;

    matrix tempMatrix[row];
    for (int l = 0; l < row; ++l) {
        tempMatrix[l].val = (double *)malloc(sizeof(double)*1);
        tempMatrix[l].val[0] = 1;
    }

        for (int k = 1; k <col ; ++k) {
            for (int i = k-1; i >=0; --i){
                temp =sumOfLine(matrixOfA,ortogolnalSet,row,k,i);
                temp2 = sumOfLine(ortogolnalSet,ortogolnalSet,row,i,i);
                double constant = temp /temp2;
                //printf("temp \n");
               // printMatrix(tempMatrix,row,1);
                constantProductMatrix(tempMatrix,ortogolnalSet,row,i,constant*(-1));

                sumOfMatrix(tempMatrix, ortogolnalSet, row, k);


            }
            sumOfMatrix2(matrixOfA, ortogolnalSet, row, k);


        }
    /**  printf("ortogonal Val \n");

  printMatrix(ortogolnalSet,row,col);

  /**
* vectorlerin normuı bulunur
   * ve bulundugu sutuna bolunur
   * normu alınır
   */
    for (int n = 0; n < col; ++n) {
        double factorization = sumOfLine(ortogolnalSet,ortogolnalSet,row,n,n);
        factorization = sqrt(factorization);

        for (int i = 0; i < row; ++i) {
            ortogolnalSet[i].val[n] = ortogolnalSet[i].val[n] / factorization;
        }
    }


    matrix TranspozeOFQ[col];
    for (int i1 = 0; i1 < col; ++i1) {
        TranspozeOFQ[i1].val = (double *)malloc(sizeof(double)*row);
    }
    /**
     * Q nun transpoze işlemi
     * col *row
     */
    for (int k1 = 0; k1 < row; ++k1) {
        for (int i = 0; i < col; ++i) {
            TranspozeOFQ[i].val[k1] = ortogolnalSet[k1].val[i];
        }
    }
    /**
     * *    *   *   *   *   *  *    *   *   *
     */
    matrix resultOfR[col];
    for (int m1 = 0; m1 < col; ++m1) {
        resultOfR[m1].val = (double *)malloc(sizeof(double)*col);
    }
    setTozero(resultOfR,col,col);
    /**
     * R factorünün bulunması
     */
    productTwoMatrix(TranspozeOFQ,matrixOfA,resultOfR,col,row,col);
    /*
     * *    *   *   *   *   *       *   *   *
     */


    /**
     * transpozeQ * b işemlemi
     */
     matrix matrixOfresult[col];
    for (int n1 = 0; n1 < col; ++n1) {
        matrixOfresult[n1].val = (double *)malloc(sizeof(double)*1);
    }
    setTozero(matrixOfresult,col,1);

    productTwoMatrix(TranspozeOFQ,matrixOfB,matrixOfresult,col,row,1);


    intverse(resultOfR,col);





    setTozero(result1,col,1);
    /**
     * sonuc result1 e atılır
     * kendi yazdığım çözüm yolu
     */
    productTwoMatrix(resultOfR,matrixOfresult,result1,col,col,1);



/**
 * realloc
 */


    for (int l1 = 0; l1 < col; ++l1) {
        free(TranspozeOFQ[l1].val);
        free(resultOfR[l1].val);
        free(matrixOfresult[l1].val);
    }




    for (int j = 0; j <row ; ++j) {
        free(ortogolnalSet[j].val);
        free(tempMatrix[j].val);

    }
    free(ortogolnalSet);

}
/**
 * verilen matrixlerin sutunlarını toplar
 * @param matrix1
 * @param matrix2
 * @param row
 * @param col
 * @return
 */
double sumOfLine(matrix matrix1[],matrix matrix2 [],int row,int WhichcolOne,int whichcolTwo){
    double sum = 0;
    for (int i = 0; i < row; ++i) {
        sum += matrix1[i].val[WhichcolOne]*matrix2[i].val[whichcolTwo];

    }

    return sum;
}
/*
 * matrixi sabit ile çarpar
 */
void constantProductMatrix(matrix temp[],matrix matrix1[],int row,int whichCol,double constant){
    for (int i = 0; i < row; ++i) {
        //printf("%lf \n",matrix1[i].val[whichCol]*constant);
        temp[i].val[0] = matrix1[i].val[whichCol]*constant;
    }
}
void sumOfMatrix(matrix matrix1[], matrix matrix2[], int row, int whicCol){
    for (int i = 0; i < row; ++i) {
        matrix2[i].val[whicCol] = matrix1[i].val[0] + matrix2[i].val[whicCol];
    }

}
void sumOfMatrix2(matrix *matrix1, matrix *matrix2, int row, int whicCol){
    for (int i = 0; i < row; ++i) {
        matrix2[i].val[whicCol] = matrix1[i].val[whicCol] + matrix2[i].val[whicCol];
    }

}
/*
 * 2 matrixi çarpar
 */
void productTwoMatrix(matrix matrix1[],matrix matrix2[],matrix result[],int row,int col1,int col2){

    for (int i = 0; i < row; ++i) { //birincimatrisin sutunu row
        for (int j = 0; j < col2; ++j) {    //ikinci matrisin col
            for (int k = 0; k <col1 ; ++k) {
                result[i].val[j] += matrix1[i].val[k] * matrix2[k].val[j];
            }
        }
    }


}
void intverse(matrix deneme[],int detS){
    double  d,k;
    matrix birim[detS];
    for (int i = 0; i < detS; ++i) {

        birim[i].val = (double *)malloc(sizeof(double)*detS);
    }
    for (int j = 0; j < detS; ++j) {
        for (int i = 0; i < detS; ++i) {

            if(i ==j)
                birim[i].val[j] = 1.0;
            else
                birim[i].val[j] = 0.0;
        }
    }

    for (int i = 0; i <detS ; ++i) {
        d = deneme[i].val[i];
        if(d == 0){
            printf("determinatn sıfır \n");
            d +=0.000001;
        }
        for (int j = 0; j < detS; ++j) {
            deneme[i].val[j] = deneme[i].val[j] / d;

            birim[i].val[j] = birim[i].val[j] / d;
        }
        for (int x = 0; x < detS; ++x) {
            if(x != i){
                k = deneme[x].val[i];
                for (int j = 0; j <detS ; ++j) {
                    deneme[x].val[j] = deneme[x].val[j] - (deneme[i].val[j]*k);
                    birim[x].val[j] = birim[x].val[j] - (birim[i].val[j]*k);
                }
            }

        }

    }
    for (int l = 0; l < detS; ++l) {
        for (int i = 0; i < detS; ++i) {
            deneme[l].val[i] = birim[l].val[i];

        }
    }
    for (int m = 0; m < detS; ++m) {
        free(birim[m].val);
    }

}
void psudoInverse(matrix matrix1[],matrix matrixOfB[],int row,int col){
    /**
     * videolarını izleyip kendim kodladım
     * internette bulamadım
     * matrix transpoze il çarpılıp
     * tersi alınır sonra birdaha transpoze ile çarpılır
     */
    matrix Transpoze[col];
    for (int i = 0; i < col; ++i) {
        Transpoze[i].val = (double *)malloc(sizeof(double) * row);
    }
    for (int k = 0; k < col; ++k) {
        for (int i = 0; i < row; ++i) {
            Transpoze[k].val[i] = matrix1[i].val[k];
        }
    }


    matrix Inverse[col];
    for (int i = 0; i < col; ++i) {
        Inverse[i].val = (double *)malloc(sizeof(double) * col);
    }
    setTozero(Inverse,col,col);
    productTwoMatrix(Transpoze,matrix1,Inverse,col,row,col);

    intverse(Inverse,col);

    matrix PesudoInverseM[col];
    for (int i = 0; i < col; ++i) {
        PesudoInverseM[i].val = (double *)malloc(sizeof(double) * row);
    }
    setTozero(PesudoInverseM,col,row);

    productTwoMatrix(Inverse,Transpoze,PesudoInverseM,col,col,row);



    setTozero(result2,col,1);

    productTwoMatrix(PesudoInverseM,matrixOfB,result2,col,row,1);



    for (int j = 0; j < col; ++j) {
        free(Transpoze[j].val);
        free(Inverse[j].val);
        free(PesudoInverseM[j].val);

    }

}
void setTozero(matrix matrix1[],int row,int col){
    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < col; ++j) {
            matrix1[i].val[j] = 0;
        }
    }
}
/*
 * çözüm yapan threadlerin methodu
 */
void *solveMatrix(void *args){



    paket *getPaket = (paket *)args;
    /*
     * pakette gelen deger eger 0 ise
     * Qr factorization çözümü yapar
     * Qr çözzüm yapılırken diğer çözümler yapılamaz
     */
    if((getPaket->order)== 0){
        pthread_mutex_lock(&mutex2);
        QRfactorization(matrixOfA,matrixOfB,getPaket->row,getPaket->col);


        pthread_mutex_unlock(&mutex2);
    }
        /**
         * paket degeri 1 ise
         * psudo inverse yapılır
         */
    else if((getPaket->order)== 1){

        pthread_mutex_lock(&mutex2);
        psudoInverse(matrixOfA,matrixOfB,getPaket->row,getPaket->col);

        pthread_mutex_unlock(&mutex2);
    }
        /**
         * paket degeri 2 ise
         * SVD çözümü yapar
         */
    else if((getPaket->order)==2){
        pthread_mutex_lock(&mutex2);
        SVDCalculate(matrixOfA,matrixOfB,getPaket->row,getPaket->col);
        pthread_mutex_unlock(&mutex2);
    }


    pthread_exit(NULL);




}
/*
 * githupdan aldıgım çözüm tam çalışmadığını
 * geç fark ettim
 * https://github.com/baiyangbupt/SVD-Decomp
 */
void SVDCalculate(matrix matrix1[],matrix matrixOfB[],int row,int col){
    matrix Transpoze[col];
    for (int i = 0; i < col; ++i) {
        Transpoze[i].val = (double *)malloc(sizeof(double) * row);
    }
    for (int k = 0; k < col; ++k) {
        for (int i = 0; i < row; ++i) {
            Transpoze[k].val[i] = matrix1[i].val[k];
        }
    }
    matrix square[col];
    matrix Eigen[col];
    matrix matrixOfV[col];
    matrix matrixOfU[col];
    for (int i = 0; i < col; ++i) {
        square[i].val = (double *)malloc(sizeof(double) * col);
        Eigen[i].val = (double *)malloc(sizeof(double) * col);
        matrixOfV[i].val = (double *)malloc(sizeof(double) * col);
        matrixOfU[i].val = (double *)malloc(sizeof(double) * 1);
        result3[i].val[0] = i;
    }
    setTozero(square,col,col);
    productTwoMatrix(Transpoze,matrix1,square,col,row,col);
   // printMatrix(square,col,col);

    SVD(square,matrixOfV,Eigen,col);

    matrix matrixTemp[row];
    for (int l = 0; l < row; ++l) {
        matrixTemp[l].val = (double *)malloc(sizeof(double) * col);
    }



   /* productTwoMatrix(matrix1,matrixOfV,matrixTemp,row,col,col);
   /* printf("Temp Result\n");
    printMatrix(matrixTemp,col,col);
    printf(" Result\n");*/
  //  productTwoMatrix(matrixTemp,Eigen,matrixOfU,col,col,col);
   /* printMatrix(matrixTemp,col,col);
    printf(" Result\n");*/
   // productTwoMatrix(Eigen,matrixOfV,matrixTemp,col,col,col);
    //printMatrix(matrixTemp,col,col);

    //productTwoMatrix(matrixOfU,matrixTemp,matrixOfV,col,col,col);
   /* printf("Matrix of V \n");
    printMatrix(matrixOfV,col,col);*/
   // intverse(matrixOfV,col);
   /* printf("Matrix of V \n");
    printMatrix(matrixOfV,col,col);*/
    /*matrix result[row];
    for (int m = 0; m < row; ++m) {
        result[m].val = (double *)malloc(sizeof(double)*1);
    }
    productTwoMatrix(matrixOfV,matrixOfB,result,col,row,1);
    printf("Result of V \n");
    printMatrix(result,row,1);*/



    for (int j = 0; j < col; ++j) {
        free(Transpoze[j].val);
        free(square[j].val);
        free(Eigen[j].val);
        free(matrixOfV[j].val);
        free(matrixOfU[j].val);


    }
    for (int n = 0; n < row; ++n) {
        free(matrixTemp[n].val);
        //free(result[n].val);
    }
}
/*
 * 2 matrixin farkını alır
 */
void difference(matrix matrix1[],matrix matrix2[],matrix result[],int row,int col){
    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < col; ++j) {
            result[i].val[j] = matrix1[i].val[j] - matrix2[i].val[j];
        }
    }


}
/*
 * verilen formüler gore
 * hata hesabını yapıp ilgili err ye aktarır
 */
void calculateError(matrix result1[],matrix result2[],matrix result3[],double *er1,double *er2,double *er3,int row,int col){
    matrix temp1[row],temp2[row];
    for (int i = 0; i < row; ++i) {
        temp1[i].val = (double *)malloc(sizeof(double)*1);
        temp2[i].val = (double *)malloc(sizeof(double)*1);
    }
   productTwoMatrix(matrixOfA,result1,temp1,row,col,1);

    difference(temp1,matrixOfB,temp2,row,1);

   matrix Transpoze[1];
   Transpoze[0].val = (double *)malloc(sizeof(double)*row);
   for (int k = 0; k < row; ++k) {
       Transpoze[0].val[k] = temp2[k].val[0];
   }

   matrix error[1];
   error[0].val = (double *)malloc(sizeof(double)*1);
   productTwoMatrix(Transpoze,temp2,error,1,row,1);

   *er1 = sqrt(error[0].val[0]);
     /** *   *   *   *   *   *Birinci deger icin hesap   *   *   *   *       **/


    setTozero(temp1,row,1);
  productTwoMatrix(matrixOfA,result2,temp1,row,col,1);

    difference(temp1,matrixOfB,temp2,row,1);

    for (int k = 0; k < row; ++k) {
        Transpoze[0].val[k] = temp2[k].val[0];
    }

    error[0].val = (double *)malloc(sizeof(double)*1);
    productTwoMatrix(Transpoze,temp2,error,1,row,1);

    *er2 = sqrt(error[0].val[0]);
/** /*  *   *   *   *   *   * ikinci deger icin hesap   *   *   *   *       **/
/** *   *   *   *   *   *Birinci deger icin hesap   *   *   *   *       **/


    setTozero(temp1,row,1);
    productTwoMatrix(matrixOfA,result3,temp1,row,col,1);

    difference(temp1,matrixOfB,temp2,row,1);

    for (int k = 0; k < row; ++k) {
        Transpoze[0].val[k] = temp2[k].val[0];
    }

    error[0].val = (double *)malloc(sizeof(double)*1);
    productTwoMatrix(Transpoze,temp2,error,1,row,1);


    *er3 = sqrt(error[0].val[0]);
    /* printf(" %lf - - \n",*er2); */
/** /*  *   *   *   *   *   * ikinci deger icin hesap   *   *   *   *       **/



    for (int j = 0; j < row; ++j) {
        free(temp1[j].val);
    }
   // free(Transpoze[0].val);
    //free(error[0].val);





}
/**
 * worker pool için oluşturulan thread methodu
 * @param args
 * @return
 * gelen requestin port numarası Queueya atılır
 * ve oradan client işini bitirince
 * socket degerini alır
 */
void *worker(void* args){

    int newSocket;
    while(1) {
        while (1) {
            if (Queusize > 0) {

                newSocket = dequeue();

                break;
            }
            usleep(200);
        }


        dataOfMatrix data;


        while (1) {
            int recSize = (int) recv(newSocket, &data, sizeof(data), 0);
            if (recSize > 0) {
               // fprintf(stderr, "okunan %d  row %d col %d\n", recSize, data.row, data.col);
               // fprintf(stderr, "thred gelen %ld  - %ld  \n", data.threadId, pthread_self());

                serveceInc();
                calculatesMatrixes(data.row,data.col,newSocket,data.threadId,data.pid);
                serveceDec();
                close(newSocket);
                break;

            }
        }
    }





    return NULL;
}
/*
 * servece degerini azaltır
 * client işini bitirince
 */
void serveceDec(){
    pthread_mutex_lock(&Mservece);
    kill(mainPid,SIGUSR1);
    pthread_mutex_unlock(&Mservece);

}
void serveceInc(){
    pthread_mutex_lock(&Mservece);
    kill(mainPid,SIGUSR2);
    pthread_mutex_unlock(&Mservece);
}
/*
 * file ismini yazar
 */
void printFile(char *massge,matrix matrix1[],int row,int col){
    FILE *fp;
    char fName[256];
    char temp[256];

    sprintf(temp,"%d",getppid());
    strcpy(fName,"serverLog/");
    strcat(fName,temp);

    fp = fopen(fName,"a+");
    fprintf(fp,"%s \n",massge);
    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < col; ++j) {
            fprintf(fp,"%lf ",matrix1[i].val[j]);
        }
        fprintf(fp,"\n");
    }
    fprintf(fp,"\n");
    fclose(fp);
}
/*
 * Liste ismini yazar
 */
void writeList(int num){
    FILE *fp = fopen("pidList","a+");
    fprintf(fp,"%d\n",num);

    fclose(fp);


}
/*
 * clientlara kill sinyali gonderir
 */
void killClients(){
    FILE *fp = fopen("pidList","r");
    int num;
    while(!feof(fp)){
        fscanf(fp,"%d",&num);
        kill(num,SIGINT);

    }

    fclose(fp);
}
