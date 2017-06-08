/*
	Rıdvan Demirci
		141044070
		Sistem Programlama 



*/
#define _POSIX_SOURCE
#define _BSD_SOURCE
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/limits.h>
#include <bits/siginfo.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#define PATH_MAX 4096
int t; /* filedescriptir*/
int mainpid; /* gelen main pid commendLine dan gelen */
int *a;     /* Queunun */
int Queusize=0;
int Quecapasity=10; /* baslangic icin 10
/Queuyu ilklendirme */
void initQuueu(){
    a =(int *) malloc(sizeof(int)*Quecapasity);
}
/* size kapasiteye yaklasinca eklstra yer alma */
void reallocate(){
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
}
/*Queue ya veri ekleme ve size artırma */
void enqueue(int data){
    if(Quecapasity - Queusize == 2){
        reallocate();
    }

        a[Queusize++] = data;
}
/* Queudan veri cikarma ve size azaltma */
int dequeue(){
    if(Queusize<=0)
        return -1;

   int result = a[0];
    int i=0 ;
    for (i = 0; i < Queusize; ++i) {
        a[i] = a[i+1];
    }
    Queusize--;
    return result;
}
/* gonderilecek olan matrix struct */
typedef  struct {
    double *satirnum;

}matrix;
/* Fifo izinler */
#define FIFO_PERM (S_IRUSR | S_IWUSR)
/* Determinant isleminde kullandıgım gauss fonksyonu
 * sayısal analiz ödevinden
 * */
int gauss(int line,int row,matrix valueOfmatrix[]);
/*commendLine ile gelen pipe adı*/
char mainPipe[PATH_MAX];
/* Comment Line da verilen matrix size */
char matrixsize[PATH_MAX];
/* verilen size da tersi alınabilen matrix olusturma    */
void setMatrix(matrix matrix1[],int size);
/* matrixi clientlara gonderme fonksyonu */
void sendMatrix(char  pipeName[],matrix matrix1[],int size);
/*matrixleri ekrana yazma */
void printMatrix(matrix matrix1[],int size){
    int i,j;
    for (i = 0; i < size; ++i) {
        for (j = 0; j < size; ++j) {
            printf("%lf    ",matrix1[i].satirnum[j]);
        }
        printf("\n");
    }
}
/* gauss ile kosegenlestirilen matrisin kosegenlerinin carpar */
double determinant(matrix *matrix1,int size);
/* kill sinyali alinca diger programları kapatır */
void killAll(int pid){
    FILE *pidList;
    int list;
    pidList = fopen("pidList.txt","r");
    while(1){
        int status = fscanf(pidList, "%d \n", &list);
        if(status == 1) {
            if (pid != list)
                kill(list, SIGUSR2);
        }else{
            fclose(pidList);
            remove("pidList.txt");
            return;
        }

    }
    fclose(pidList);
}
/* kill sinyalini yakalama */
void signalh(int no){
    perror("Kill Signali geldi\n");
    killAll(mainpid);
	FILE  *LogFile;
    LogFile = fopen("log/server.log","a+");
    fprintf(LogFile,"\nKill Signali geldi");
    fclose(LogFile);
    free(a);
    unlink(mainPipe);
    exit(1);
}
/* matrixin determinanatını hesaplar */
double hesapla(matrix matrix1[],int size){
    double sonuc = 1;
    int i;
    for (i = 0; i < size; ++i) {
        /*printf("%.2lf ",matrix1[i].satirnum[i]) */
        sonuc *= matrix1[i].satirnum[i];

    }
    return sonuc;
}
/*
 * https://www.linuxprogrammingblog.com/code-examples/sigaction
 * sinyali gonderen proccesin pid sini ogrenme kodu siteden alındı
 * yakaladıktan sonra Queue ya atar
 */

void yakala (int sig, siginfo_t *siginfo, void *context)
{
        sigset_t initmask;
        sigaddset(&initmask,SIGINT);
        sigprocmask(0,&initmask,NULL);
        int pid = siginfo->si_pid;
       // printf("sinyal geldi = %d \n", pid);
        enqueue(pid);
        sigprocmask(1,&initmask,NULL);
       /* printqueue();
        printf("şşş"); */


}
/* baska program tarafından oldurme sinyali gelince */
void geldi(int no){
    printf("\nbaskası tarafından  oldurme sinyali geldi");
    free(a);
    FILE  *LogFile;
    LogFile = fopen("log/server.log","a+");
    fprintf(LogFile,"\nbaskası tarafından oldurme sinyali geldi");
    fclose(LogFile);
    unlink(mainPipe);
    exit(1);
}
/* matrixin tersini almak icin yazdigim driver method */
/*
void deneme(){
    matrix deneme[detS];
    matrix birim[detS];
    for (int i = 0; i < detS; ++i) {
        deneme[i].satirnum = (double *)malloc(sizeof(double)*detS);
        birim[i].satirnum = (double *)malloc(sizeof(double)*detS);
    }
    FILE *fp;
    fp = fopen("input.txt","r");
    for (int j = 0; j < detS; ++j) {
        for (int i = 0; i < detS; ++i) {
            fscanf(fp,"%lf",&deneme[j].satirnum[i]);

            if(i ==j)
                birim[i].satirnum[j] = 1.0;
            else
                birim[i].satirnum[j] = 0.0;
        }
    }
    printMatrix(deneme,detS); printf("\n");
    double  d,k;
    for (int i = 0; i <detS ; ++i) {
        d = deneme[i].satirnum[i];

        for (int j = 0; j < detS; ++j) {
            deneme[i].satirnum[j] = deneme[i].satirnum[j] / d;
            birim[i].satirnum[j] = birim[i].satirnum[j] / d;
        }
            for (int x = 0; x < detS; ++x) {
                if(x != i){
                    k = deneme[x].satirnum[i];
                    for (int j = 0; j <detS ; ++j) {
                        deneme[x].satirnum[j] = deneme[x].satirnum[j] - (deneme[i].satirnum[j]*k);
                        birim[x].satirnum[j] = birim[x].satirnum[j] - (birim[i].satirnum[j]*k);
                    }
                }
                
            }

    }


    printMatrix(birim,detS);

    printf("sonuc %.2lf \n",determinant(deneme,detS));

}
 */
/* Queuda request var ise buraya girer */
void request(int reqpid,int counter){
    char requestpid[PATH_MAX]; /* request olan pid char* a cevrilir */
    sprintf(requestpid,"%d",reqpid);
   /* printf("işlem var request pid %s\n",requestpid);
    / clientin fifosu acilir yazmak icin */
    int fd = open(requestpid,O_WRONLY);

    matrix *matrix1;

    /* global olan matrix size degerini integer yapma */
    int size = atoi(matrixsize) * 2;

    /* size kadar kare matrix olusturma */
    matrix1 = (matrix *)malloc(sizeof(matrix)*size);
    /*              Matrix size olusturma */
    int jj,i;
    for (jj = 0; jj < size; ++jj) {
        matrix1[jj].satirnum = (double *)malloc(sizeof(double)*size);
        }
    /* matrixi 4 e bolup determinantları icin tutulan degerler */
    double det,one,two,three,four;

    /* sub matrixler icin tutulan temp matrix */
    matrix temp[size/2];

    /* temp matrixe yer alma */
    for (i = 0; i < size/2; ++i) {
        temp[i].satirnum = (double*)malloc(sizeof(double)*size/2);
    }
    /** matrixi set etme **/
    while(1) {
        /* matrix raskele degerler ile doldurulur 1-100 */
        setMatrix(matrix1, size);

        /* determinantı alınır */
        det = determinant(matrix1, size);
         /* sol ust kosesi tempe atılır */
        int ii,jj;
        for (ii= 0; ii < size/2; ++ii) {
            for (jj = 0; jj < size/2; ++jj) {
                temp[jj].satirnum[ii - size/2] = matrix1[jj].satirnum[ii];
            }
        }
        /* atılan tempin determinanatı alınır ve one degerine atılır */
        one = determinant(temp,size/2);
        /* sag ust kose tempe alınır ve determinantı alınır*/
        int jjj,iii;
        for ( jjj = 0; jjj < size/2; ++jjj) {
            for (iii = size/2; iii < size; ++iii) {
                temp[jjj].satirnum[iii - size/2] = matrix1[jjj].satirnum[iii];
            }
        }
        /* sag ust degeri two degerine alınır */
        two = determinant(temp,size/2);
        /*   sol alt  kosesi tempe alınır ve determşnantı alınır      */
        for (jj = size/2; jj < size; ++jj) {
            for (ii = 0; ii < size/2; ++ii) {
                temp[jj-size/2].satirnum[ii] = matrix1[jj].satirnum[ii];
            }
        }
        /* sol alt degeri determinantı three degerine alınır */
        three = determinant(temp,size/2);
                /*      sag alt kosesi temp matrixe atılır */
        for (jj = size/2; jj < size; ++jj) {
            for (ii = size/2; ii < size; ++ii) {
                temp[jj-size/2].satirnum[ii - size/2] = matrix1[jj].satirnum[ii];
            }
        }
        /* sag alt determinant degeri four degiskenine atılır */
        four = determinant(temp,size/2);

        /* eger tum koseler ve genel determinant degerleri sıfır degil ise donguden cikilir */
        if((det != 0.0) && (one != 0.0) && (two != 0.0) && (three != 0.0) && (four != 0.0)){
            break;
        }


    }

         /*  printf("matirx size = %d \n", size);
            matrixin tam anlamıyla olusturulma zamanı */
    struct timeval tpstart;
    gettimeofday(&tpstart, NULL);
    long  time =tpstart.tv_sec*1000; /* matrix olusturulan zaman

    * Matrixin olusturulma zamanı Log dosyasına client pidsi ile yazılır
    */
    FILE  *LogFile;
    LogFile = fopen("log/server.log","a+");
    fprintf(LogFile,"logs the time %ld the %d x %d matrix generate , pid of the client %s , determinant "
            "%lf \n",time,size,size,requestpid,det);
           /* printMatrix(matrix1,size);
             printf("\ndeterminant = %lf \n", det);
    /* *    *   *   *   *       *   *   *   *   *       *   *   *
    olusturulan matrixin size degerini clientin fifosuna yazılır*/
    write(fd,&counter,sizeof(int));//kaçıncı client oldugunu yazarr
    write(fd,&size,sizeof(int));
   // fprintf(stderr,"\ngonderilen fd %d\n",fd);
    /* artdından olusturulan matrixi komple fifoya gonderilir */
    sendMatrix(requestpid,matrix1,size);
    /*  printf("client Id %s gonderilen deger  \n",requestpid);
     log dosyasının kapatılması */
    fclose(LogFile);

    /*temp matrixi free yapma
     * for (int i = 0; i < size/2; ++i) {
        free(temp[i].satirnum);
    } */
   /* alınan matrixi free yapma */
    free(matrix1);
}


int main(int argc,char *argv[]) {
    /* Time server kendi pid sinin calisan programlar pidListesine atar
     * procceslerin birbirini oldurmesi icin */
    FILE *pidList;
    pidList = fopen("pidList.txt","a+");
    fprintf(pidList,"%d \n",getpid());
    fclose(pidList);
    /*  *   *   *   *   *   *   *       *   *   *   * */
    signal(SIGUSR2,geldi); /* baska program tarafından oldurulme sinyali gelir ise */

    initQuueu(); /* Queue Listesinin ilklendirilmesi */
        
    mkdir("log",0777);/* Log Direktorysinin oluşturulması
     Interrupt ve Sıguser1 sinyalinin handle edilmesi */
    signal(SIGINT,signalh);
    struct sigaction act;
    act.sa_handler = yakala;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);
    /*   *  *   *   *   *   *   *   *   *   *   *       *   */
    if(argc != 4){
        printf("usage: ./timeServer <tick in miliseconds> <n> <main pipe name>");

    }
    else {
        /* mainPipe ,Matrix Size ve tickSeconds ın handle edilmesi */
        strcat(mainPipe,argv[3]);
        strcat(matrixsize,argv[2]);
        int milisecond = atoi(argv[1]) * 1000;
        /*  *   *   *   *   *   *   *   *   *   *   *   */
        if(mkfifo(mainPipe, FIFO_PERM)==-1){
            printf("fifo hatası");
        }
        /***************** FIFO OLUSturuldu ************ */

       printf("myID= %d\n",getpid());
        /*
         * main pipe acilmasi ve ona kendi pid degerinin yazilmasi
         */
        if ((t = open(mainPipe, O_WRONLY)) == -1) {
            printf("sıkıntı");
        }
        int pid = getpid();
        mainpid = pid;

            while(write(t, &pid, 4) <0);
            close(t);
            int count=0;
        /****   *   *   *   *       *   *   *   *   *      */
        for(; ; ){ /* bekleme ve maskeleme sorgulama yapılır */


            usleep(milisecond); /* ana proccesin milisaniye kadar beklemese

             * bekleme suresinde gelen sinyaller Queue ya atılır
             * eger Queue size > 0 ise sinyal gelmistir
             * proccess olusturup matrix olusturur
             */

            if(Queusize > 0){
             	count++;
                /* sıradaki pid Queuedan cikarilir */
                int reqpid = dequeue();
                if(fork() == 0){
                    request(reqpid,count);
                    exit(1);
                }
                else{
                    wait(NULL);

                }

            }



        }
    }
    return 0;
}

void sendMatrix(char  pipeName[],matrix matrix1[],int size){
/*  verilen pipadına verilen matrixi yazar */
    int fd,k,i;
    fd = open(pipeName,O_WRONLY);
    for ( k = 0; k < size; ++k) {
        for ( i = 0; i < size; ++i) {
          write(fd,&matrix1[k].satirnum[i],sizeof(double));
        }
    }
    close(fd);
}
void setMatrix(matrix matrix1[],int size){
    /* verilen matrixe random satılar ile doldurur */

    struct timeval tpstart1;
    gettimeofday(&tpstart1, NULL);
    long timestart1 = tpstart1.tv_usec*1000;

    srandom(timestart1);   /* microsecond cinsinden raskele sayılar uretilir */
    int i,j;
    for (i = 0; i < size; ++i) {
        for (j = 0; j < size; ++j) {
            matrix1[i].satirnum[j] = rand() % 99 + 1;
        }
    }
}
/*
 * matrixi tempe atar ve oradan gauss alır sonra kosegenleri carpae
 */
double determinant(matrix matrix1[],int size){
   matrix * temp;
    temp = (matrix *)malloc(sizeof(matrix)*size);
               /* Matrix size olusturma */
    int j,i;
    for (j = 0; j < size; ++j) {
        temp[j].satirnum = (double *)malloc(sizeof(double)*size);
    }
/*matrixin tempe atılması */
    for (i = 0; i < size; ++i) {
        for (j = 0; j < size; ++j) {
            temp[i].satirnum[j] = matrix1[i].satirnum[j];
        }
        
    }
/* hersatır icin tempe gauss yapılır */
    int k;
    for (k = 0; k <size ; ++k) {
        if(gauss(k,size,temp) == 0){
            return 0.0;
        }
    }

    double result = hesapla(temp,size);
    int l;
    for (l = 0; l < size; ++l) {
        free(temp[l].satirnum);
    }
    free(temp);
    return result;

}
int gauss(int line,int row,matrix valueOfmatrix[]){
    /* verilen ilgili line */
    int sline = line;
    /* pivot eleman sıfır ise determinant sıfır diye doner */
    int i,j;
    for (i = line+1; i < row; ++i) {
        if( valueOfmatrix[sline].satirnum[sline] == 0){
            return 0;
        }
        /* pivot elemanın satır baslarına bolunmesi */
        double katsayi = valueOfmatrix[i].satirnum[sline] / valueOfmatrix[sline].satirnum[sline];

        /* her sutundan cikarilmasi */
        for (j = 0; j < row; ++j) {
            /* matrix degerlerini duzenleme */
            valueOfmatrix[i].satirnum[j]= valueOfmatrix[i].satirnum[j]- (valueOfmatrix[sline].satirnum[j])*katsayi;

        }
    }
}


