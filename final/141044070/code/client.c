/**
 * Rıdvan Demirci
 *      141044070
 *      sistem Proframlama Final
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <memory.h>
#include <sys/stat.h>
#include <math.h>
#include <sys/time.h>

/**
 * Mutex Initilaze
 */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;

/**
 *  Servera Gönderilen degerler
 */
typedef struct{
    int row;
    int col;
    long int threadId;
    int pid;
}dataOfMatrix;
/**
 * *    *   *   *   *   *   *   *
 *
 * Gonderip Alınan matrix yapıları
 */
typedef struct {
    double *val;

}matrix;

/**
 * Client Threadleri için kullanılan yapı
 */
typedef struct{
    int socketNo;
    struct sockaddr_in temp;
}yapi;
/**
 * Global Variable
 */
int portNO;
int row;
int col;
int clientSize;
int portNo; // commandLinedan gelen portNo
sem_t sem;  //Clientların Connect olacagi Semaphore
/**
 * iki zaman Farkını alan method
 * http://bilgisayarkavramlari.sadievrenseker.com/2009/01/01/c-ile-zaman-islemleri/
 */
float getdiff(struct timeval endtv, struct timeval starttv);
/**
 * Verilen formülle Standart sapma hesabı
 */
double StandarSapma(double AvarageTime,int size);
/*
 * Clientın Log dosyasına verilen messagge ile beraber
 * verilen matrixi yazma
 */
void printFile(char *massge,matrix matrix1[],int row,int col,int pRocces);
/*
 * verilen Matrixi ekrana yazar
 */
void printMatrix(matrix matrix1[],int row,int col){
    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < col; ++j) {
            printf("%lf ",matrix1[i].val[j]);
        }
        printf("\n");
    }



}

/**
 *
 * Sockete connect olma methodu
 * semaphore kullanılır
 * yani kullanılacak port return edilir
 */
int connectToServer(int portNo,struct sockaddr_in *serverInfo){
    /**
     * Connetct olurken kullanılan semaphore
     */
    sem_wait(&sem);


    int serverSocket = socket(AF_INET,SOCK_STREAM,0);
    (*serverInfo).sin_family = AF_INET;
    (*serverInfo).sin_port = htons(portNo);
    (*serverInfo).sin_addr.s_addr = inet_addr("127.0.0.1");

    sem_post(&sem);
    /**
     * semaphore sonu
     */

    return serverSocket;

}
/*
 * Global Thread Methodu
 */
pthread_t *clientThreads;
int sayac = 0;  //Timeları eklemek için
/**
 * Her Clientın çalışma süresi tutulur
 */
double *TimeperClient;
double AvarageTime = 0;
/*
 * Threadlerin kullandığı connect methodu
 */
void * request(void *args){

    struct timeval starttv, endtv;

    gettimeofday(&starttv,NULL);

/**
 * socket için farklı sayılar
 * structlar
 */


    yapi *gelen =(yapi *)args;
    int serverSocket = gelen->socketNo;
    struct sockaddr_in serverInfo = gelen->temp;


    socklen_t ServerSize = sizeof(serverInfo);

    //thread ID ve socket NUmarası ekrana basılır
    printf("gelen thread %ld socket %d \n", pthread_self(), serverSocket);

    //Sockete Connect Olma
    int result = connect(serverSocket, (struct sockaddr *) &serverInfo, ServerSize);

    /*
     * Data structının doldurulması
     */
    dataOfMatrix data;
    data.row = row;
    data.col = col;
    data.threadId = pthread_self();
    data.pid = getpid();    // kill sinyali almak için
    /*
     * Data Structinin servera gonderilmesi
     */
    while(1) {
        int size = (int) send(serverSocket, &data, sizeof(data), 0);

        if (size > 0) {
            printf("gonderilen %d - %ld \n", size, pthread_self());
            break;
        }
    }
    /**
     * Senkranizasyon için
     * geçici temp deger gonderilir
     */

    int control = 1;
    send(serverSocket, &control, sizeof(int), 0);

    /**
     * A ve B matrixlerinin alınması için
     * matrixler oluşturulur
     */

    matrix matrixA[row],matrixB[row];
    for (int i = 0; i < row; ++i) {
        matrixA[i].val = (double *)malloc(sizeof(double)*col);
        matrixB[i].val = (double *)malloc(sizeof(double)*1);
    }
    /**
     * A matrixinin alınması
     */
    double val =0;
    for (int j = 0; j < row; ++j) {
        for (int i = 0; i < col; ++i) {
            while(1) {
                int size = (int) recv(serverSocket, &val, sizeof(val), 0);
                if (size > 0) {
                    matrixA[j].val[i] = val;
                    break;
                }
            }
        }
    }
    /***    *   *   *   *   *   *   *   *   *   *
     * B matrixinin alınması
     */

    for (int j = 0; j < row; ++j) {

            while(1) {
                int size = (int) recv(serverSocket, &val, sizeof(val), 0);
                if (size > 0) {
                    matrixB[j].val[0] = val;
                    break;
                }
            }

    }
    /***    *   *   *   *   *   *   *   *   *   *
     * result 1 in alınması
     */
matrix result1[col];
    for (int l = 0; l < col; ++l) {
        result1[l].val =(double *)malloc(sizeof(double)*1);
    }
    for (int j = 0; j < col; ++j) {

        while(1) {
            int size = (int) recv(serverSocket, &val, sizeof(val), 0);
            if (size > 0) {
                result1[j].val[0] = val;
                break;
            }
        }

    }

    /***    *   *   *   *   *   *   *   *   *   *
        * result 2 in alınması
        */
    matrix result2[col];
    for (int l = 0; l < col; ++l) {
        result2[l].val =(double *)malloc(sizeof(double)*1);
    }
    for (int j = 0; j < col; ++j) {

        while(1) {
            int size = (int) recv(serverSocket, &val, sizeof(val), 0);
            if (size > 0) {
                result2[j].val[0] = val;
                break;
            }
        }

    }

        /*
         * result 3 in alınması
        */
    matrix result3[col];
    for (int l = 0; l < col; ++l) {
        result3[l].val =(double *)malloc(sizeof(double)*1);
    }
    for (int j = 0; j < col; ++j) {

        while(1) {
            int size = (int) recv(serverSocket, &val, sizeof(val), 0);
            if (size > 0) {
                result3[j].val[0] = val;
                break;
            }
        }

    }

    /**
     * *    *   *   *   *   *       **
     * Errorrların alınması ******
     * Errorlar double Arraye alınır
     */
    double error[3];
    for (int n = 1; n <=3; ++n) {
        while(1) {
            int size = (int) recv(serverSocket, &error[n-1], sizeof(val), 0);
            if (size > 0) {
                printf("err %d %lf \n\n", n,error[n-1]);
                break;
            }
        }

    }

    /**
     * Alınan veriler Client directory sine
     * pid.txt cinsiyle yazılır
     */
    pthread_mutex_lock(&mutex3);
    FILE *fp;
    char fName[256];
    char temp[256];
    sprintf(temp,"%d",getpid());

    strcpy(fName,"ClientLog/");
    strcat(fName,temp);
    printf("fname = %s \n",fName);

    fp = fopen(fName,"a+");
    fprintf(fp,"\n newClient \nthreadNUm %ld \n",pthread_self());
    fclose(fp);

    /**
     * Alınan Matrixler Hem Ekrana Hemde
     * Log Dosyasına yazılır
     */



    printFile("A matrixi",matrixA,row,col,getpid());


    printMatrix(matrixA,row,col);
    printf("\n");
    printFile("B matrixi",matrixB,row,1,getpid());
    printMatrix(matrixB,row,1);

    printf("\n");

    printFile("result1",result1,col,1,getpid());

    printFile("result2",result2,col,1,getpid());

    printFile("result3",result3,col,1,getpid());

    fp = fopen(fName,"a+");
    fprintf(fp,"error 1 = %lf \n",error[0]);
    fprintf(fp,"error 2 = %lf \n",error[1]);
    fprintf(fp,"error 3 = %lf \n",error[2]);
    fclose(fp);

    pthread_mutex_unlock(&mutex3);
    /**
     * Matrixler Realloc edilir
     */
    for (int k = 0; k < row; ++k) {
        free(matrixA[k].val);
        free(matrixB[k].val);
    }
    for (int m = 0; m < col; ++m) {
        free(result1[m].val);
        free(result2[m].val);
        free(result3[m].val);
    }



        close(serverSocket);

    gettimeofday(&endtv, NULL);
    /**
     * Süre hesabı yapılır
     */

    TimeperClient[sayac]=getdiff(endtv, starttv);
    printf("Gecen Süre %f \n",TimeperClient[sayac]);
    AvarageTime += TimeperClient[sayac];
    sayac++;
    /**
     * hersürenin ekleneceği yer için tutulan sayac
     * */

    return (NULL);

}
int main(int argc ,char *argv[]) {
    if(argc != 5){
        printf("usage: ./client <row> <col> <clientSize> <portNo>\n");
        return 0;
    }
    /** ilklendirme *   *   *   *   *   */
     row = atoi(argv[1]);
     col = atoi(argv[2]);
     clientSize = atoi(argv[3]);
     portNo = atoi(argv[4]);
    sem_init(&sem,0,1);
    /** *   *   *   *   *   *   *   *   */

    mkdir("ClientLog",0777);/* Log Direktorysinin oluşturulması */
    TimeperClient = (double *)malloc(sizeof(double)*clientSize);

    /**
     * threadlerin olusturulması
     */
    clientThreads =(pthread_t *)malloc(sizeof(pthread_t) *clientSize);
    struct sockaddr_in serverInfo[clientSize];
    int socketNo[clientSize];

    yapi degerler[clientSize];
    /**
     * servera yapı ile beraber gonderilir
     * paralel threadler olusturulur
     */
        for (int i = 0; i < clientSize; ++i) {
           socketNo[i] = connectToServer(portNo,&serverInfo[i]);
            degerler[i].socketNo = socketNo[i];
            degerler[i].temp = serverInfo[i];
            pthread_create(&clientThreads[i], NULL, request ,&degerler[i]);

        }
    for (int j = 0; j < clientSize; ++j) {
        pthread_join(clientThreads[j], NULL);
    }
    /**
    * Olusan threadler beklenir
    */



    free(clientThreads);
    /*
     * Time hesabı ve hesaplanan timeların
     * Log dosyasına ve ekrana yazılması
     */
    AvarageTime =  AvarageTime/clientSize;
    FILE *fp;
    char fName[256];
    char temp[256];
    sprintf(temp,"%d",getpid());

    strcpy(fName,"ClientLog/");
    strcat(fName,temp);
    printf("fname = %s \n",fName);

    fp = fopen(fName,"a+");
    fprintf(fp,"ortalama zaman %lf \n",AvarageTime);
    fprintf(fp,"Standarsapma %lf\n",StandarSapma(AvarageTime,clientSize));
    printf("ortalama zaman %lf \n",AvarageTime);
    printf("Standarsapma %lf",StandarSapma(AvarageTime,clientSize));

    fclose(fp);

    free(TimeperClient);

    //  Sinyali denemek için
    //sleep(2);




    return 0;
}
/*
 * Gelen message ile matrixi Log dosyasına yazar
 */
void printFile(char *massge,matrix matrix1[],int row,int col,int pRocces){
    FILE *fp;
    /**
     * Log Dosyasının ismini olusturma
     */
    char fName[256];
    char temp[256];
    sprintf(temp,"%d",pRocces);

    strcpy(fName,"ClientLog/");
    strcat(fName,temp);
    printf("fname = %s \n",fName);
    /**
     *  *   *   *   *   *       *
     *
     */

    fp = fopen(fName,"a+");
    fprintf(fp,"%s \n",massge); //önce message yazılır
    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < col; ++j) {
            fprintf(fp,"%lf ",matrix1[i].val[j]);
        }
        fprintf(fp,"\n");
    }
    fprintf(fp,"\n");
    fclose(fp);
}

/**
 * iki zaman Farkını alan method
 * http://bilgisayarkavramlari.sadievrenseker.com/2009/01/01/c-ile-zaman-islemleri/
 */
float getdiff(struct timeval endtv, struct timeval starttv)
{
    float diff=0;
    diff=(endtv.tv_sec-starttv.tv_sec)*1000000+
         (endtv.tv_usec-starttv.tv_usec);
    return diff/1000;
}
/*
 * Verilen formül ile Standart sapma hesaplama
 */
double StandarSapma(double AvarageTime,int size){
    double sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += ( pow(TimeperClient[i],2))-(size*AvarageTime);
    }
    sum = sum/(size-1);
    sum = sqrt(sum);
    return sum;
    
}