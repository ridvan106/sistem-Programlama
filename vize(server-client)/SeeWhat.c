/*
	Rıdvan Demirci
		141044070
		Sistem Programlama 



*/

#define _POSIX_SOURCE
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/limits.h>
#include <time.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/time.h>

// matrix i alma icin struct
typedef  struct {
    double *satirnum;

}matrix;
int GlobalCounter;
#define FIFO_PERM ( S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
/* timeServer.c deki methodlarla aynı */
double hesapla(matrix matrix1[],int size){
    double sonuc = 1;
    for (int i = 0; i < size; ++i) {
        //printf("%.2lf ",matrix1[i].satirnum[i])
        sonuc *= matrix1[i].satirnum[i];

    }
    return sonuc;
}
int gauss(int line,int row,matrix valueOfmatrix[]){
    int sline = line;
    for (int i = line+1; i < row; ++i) {
        if( valueOfmatrix[sline].satirnum[sline] == 0){
            return 0;
        }
        double katsayi = valueOfmatrix[i].satirnum[sline] / valueOfmatrix[sline].satirnum[sline];  //E1(e2*e1

        for (int j = 0; j < row; ++j) {
            // matrix degerlerini duzenleme
            valueOfmatrix[i].satirnum[j]= valueOfmatrix[i].satirnum[j]- (valueOfmatrix[sline].satirnum[j])*katsayi;

        }
        // cout << "katsayı " << katsayi << endl;
    }
}
double determinant(matrix matrix1[],int size);
/****   *   *   *   *       *   *   * */
/* http://bilgisayarkavramlari.sadievrenseker.com/2008/11/19/matrisin-tersinin-alinmasi-mantrix-inverse/
 *
 * Gauss-jordan yonntemi kullanılılarak matrix tersi alınır
 * */

void intverse(matrix deneme[],int detS);
// her parça için ters alınır ve yerlerine yazılır
void shiftedInverse(matrix matrix1[],int size);
// client pipe name
char str[PATH_MAX]; // olusturulacak yeni pipe adı
// oldurulme sinyali alır ise diger programları da oldurur
void yaz(char message[]){
	FILE *fp;
    char fileName[PATH_MAX];
    strcpy(fileName,"");
    strcat(fileName,"log/SW");
    char pidd[PATH_MAX];
    char countt[PATH_MAX];
    sprintf(countt,"%d",GlobalCounter);
    strcat(fileName,"_");
    strcat(fileName,countt);
    strcat(fileName,".log");
    fp = fopen(fileName,"a+");
    fprintf(fp,"\n%s",message);
    fclose(fp);


}

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
// olme sinyali alırsa
void cathSignal(int no){

    perror("öldürme sinyali aldım");
    yaz("öldürme sinyali aldım");
    killAll(getpid());
    char deletePipe[PATH_MAX];
    sprintf(deletePipe,"%d",getpid());
    unlink(deletePipe);
    unlink(str);
    exit(1);
}
// gelen matrixi Matlab formatında log/SW_Counter.log formatında dosyalara yazar
void printMatrix(matrix matrix1[],int size,int pid,int count,char matrixName[]){
    /*      SWLog/pid_conter.log formatında SeeWhat Dosyalarının olusturulması  */
    FILE *fp;
    char fileName[PATH_MAX];
    strcpy(fileName,"");
    strcat(fileName,"log/SW");
    char pidd[PATH_MAX];
    char countt[PATH_MAX];
    sprintf(countt,"%d",count);
    strcat(fileName,"_");
    strcat(fileName,countt);//timeServerdan gelen counter
    strcat(fileName,".log");
    fp = fopen(fileName,"a+");
    /*  *   *   *   *   *       *   */
    /* matlab formatında log dosyasına yazılması */
    fprintf(fp,"%s = [",matrixName);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            fprintf(fp," %lf ",matrix1[i].satirnum[j]);
        }
        if(i != size-1)
        fprintf(fp,";");
    }
    fprintf(fp,"]\n");
    fclose(fp);
    /*          acilan Log dosyalarının kapatılması     */
}
// Matrixleri Ekrana yazma
void PMatrix(matrix matrix1[],int size){
    int i,j;
    for (i = 0; i < size; ++i) {
        for (j = 0; j < size; ++j) {
            printf("%lf    ",matrix1[i].satirnum[j]);
        }
        printf("\n");
    }

}
/*
 * http://www.songho.ca/dsp/convolution/convolution.html
 * sitesinden aldıgım convolution hesabı
 * */
void convolve2DSlow(matrix data[],matrix kernel[], int kernelSizeX, int matrixSize);
/*
 * convolutionı alınan matrixin determinantı alınması
 */
double convDeterminant(matrix matrix1[],int size,int client_pid,int count);
/*
 * Baska program tarafından olme sinyali alırsa
 */
void geldi(int no){
    printf("baskası tarafından  oldurme sinyali geldi");
    char deletePipe[PATH_MAX];
    sprintf(deletePipe,"%d",getpid());
    unlink(deletePipe);
    yaz("baskası tarafından  oldurme sinyali geldi");
    exit(1);
}

int main(int argc,char *argv[]) {
    /*
     * Client pidsinin alınması
     */
    int client_pid = getpid();
    printf("client pid: %d\n",client_pid);

    signal(SIGINT,cathSignal); // ölme sinyali alırsa
    signal(SIGUSR2,geldi);     // baska programı tarafından olme gelirse
    char *mainPipe = argv[1];   // main pipe adının alınmas
    int k = open(mainPipe,O_RDWR); // pipedan deger okur

    if(argc != 2){
        printf("usage: ./SeeWhat <main pipe name>");

    }
    else{
        int pid = 0;
        while(read(k, &pid, 4) <0); // Serverpid nin alınması
        write(k,&pid,4);    // tekrar main fifoya server pid yazılır
         printf("server pid %d \n", pid);
      //  close(k);
    /* Kendi pidsini pid listesine ekleme
     * showResult ta Listelemesi için
     * */
        FILE *Showresult;
        FILE *pidList;
        pidList = fopen("pidList.txt","a+");
        fprintf(pidList,"%d \n",getpid());
        fclose(pidList);
    /*  *   *   *   *   *   *       *   *   */

        
        while(1) {
            // ShowResult istenilen formatta yazması için pid formatı ile yazar
            Showresult = fopen("showLog.txt","a+");

            int  fd;
            // Clientın pid si
            int mypid = getpid();
            char ClientFifo[PATH_MAX];
            sprintf(ClientFifo, "%d", mypid);
            mkfifo(ClientFifo, FIFO_PERM);

            kill(pid, SIGUSR1); // server a matrix alma sinyali gonderir

            /** Signal gonderildikten sonra Serverdan oncelikle size bekler */
            fd = open(ClientFifo, O_RDONLY);

            int gelen;
            int size,count;
            read(fd,&count,sizeof(int));//SW_count
            GlobalCounter = count;
            read(fd,&size,sizeof(int)); // matrix size
            /** Size yazıldıktan sonra o degeri okur    */
            /* 2 tane Matrix olusturulur ve  Serverdan alınır */
            matrix *matrix1;
            matrix *matrix2;
            matrix1 = (matrix *)malloc(sizeof(matrix)*size);
            matrix2 = (matrix *)malloc(sizeof(matrix)*size);
            for (int i = 0; i < size; ++i) {
                matrix1[i].satirnum = (double *)calloc(sizeof(double),size);
                matrix2[i].satirnum = (double *)calloc(sizeof(double),size);
            }
            /*          Dosyadan Verileri Okuma ***********/
            for (int j = 0; j < size; ++j) {
                for (int i = 0; i <size ; ++i) {

                    read(fd, &matrix1[j].satirnum[i], sizeof(double));
                    matrix2[j].satirnum[i] = matrix1[j].satirnum[i];

                }
            }

            close(fd);
            // matrixi okuma işi biter
            /********************************************** */
          //  printf("size = %d count = %d\n",size,count);
            printMatrix(matrix1,size,getpid(),count,"Orgin");
            // Matrix okuma bittikten sonra SeeWhata MatLap formatında Orgin yazılır
           // printf("* ************ * ** **********************\n");
            unlink(ClientFifo); // client fifosu silinir

           	int pi[2];
           	pipe(pi);

            /*      shifted time zaman hesabı   */
            struct timeval tpstart1;
            gettimeofday(&tpstart1, NULL);
            long timestart1 = tpstart1.tv_usec*1000;



            /*    Shifted time */
            if(fork() == 0){ // shifted
                double detOrgin = determinant(matrix1,size);
                //
                // printf("orginal determinant %.2lf \n",detOrgin);
               // printf("shifted sonuc1 \n");

                shiftedInverse(matrix1,size);
                /*      shifted time zaman hesabı   */

                /*    Shifted time */

                //Log dosyasına matlab formatı ile yazımı
                printMatrix(matrix1,size,client_pid,count,"ShiftedInverse");

                /* Result1 pipe aracılıgı ile cocuk proccessden alınır */
                double detShifted = determinant(matrix1,size);
                double resul1 = detOrgin - detShifted;

                write(pi[1],&resul1,sizeof(double));
                close(pi[0]);
                close(pi[1]);
				
                   exit(1);
                /* cocuk programdan cıkar */
            }
            else{
                /* Main proccess tarafından cocukun ürettiği shifted
                 * sonucu alınır */
                double result1;
                
                read(pi[0],&result1,sizeof(double));

                close(pi[1]);
                close(pi[0]);
                pipe(pi);
                /*  *   *   *   *   *   *       *   *   *   *   */
                /* convolution için olusturulan timer */
                struct timeval tpstart2;
                gettimeofday(&tpstart2, NULL);
                long  timeResult2 =tpstart2.tv_usec*1000; // matrix olusturulan zaman

                if(fork() == 0){
                    double detOrgin = determinant(matrix2,size);

                    double result2 = convDeterminant(matrix2,size,client_pid,count);
                    result2 = detOrgin - result2; // result 2 hesabı
                    /* convolution sonucu tekrar pipe yazılır */
                    write(pi[1],&result2,sizeof(double));
                    close(pi[1]);
               		close(pi[0]);

                    exit(1);
                }
                double result2;
                /* convolution sonucu tekrar pipe dan okunur */
                read(pi[0],&result2,sizeof(double));
               // fprintf(stderr,"%lf\n",result2);
                close(pi[1]);
                close(pi[0]);
                /* 2 proccessde beklenir ve  zaman hesapları log dosyasına yazılır */
				while(wait(NULL) != -1);
                struct timeval tpend1;
                gettimeofday(&tpend1, NULL);
                long timeend1 = tpend1.tv_usec*1000;


                struct timeval tpend2;
                gettimeofday(&tpend2, NULL);
                long  EndtimeResult2 =tpend2.tv_usec*1000; // matrix olusturulan zaman

                fprintf(Showresult,"%d\n",client_pid);
                fprintf(Showresult,"result 1 = %lf timeFlag = %ld\n ",result1,timeend1 - timestart1);
                fprintf(Showresult,"result 2 = %lf timeFlag = %ld \n",result2,EndtimeResult2 -timeResult2);
                for (int i = 0; i < size; ++i) {
                    free(matrix1[i].satirnum);
                    free(matrix2[i].satirnum);
                }
                free(matrix1);
				free(matrix2);
                FILE *resultfile;
                resultfile = fopen("result.txt","a+");
                fprintf(resultfile,"pid -> %d result1 %lf  result2 %lf \n",getpid(),result1,result2);
                fclose(resultfile);
                unlink(ClientFifo);
                
            }
            fclose(Showresult);
			
        }

    }
    return 0;
}

/****   *   *   *   *       *   *   * */
/* http://bilgisayarkavramlari.sadievrenseker.com/2008/11/19/matrisin-tersinin-alinmasi-mantrix-inverse/
 *
 * Gauss-jordan yonntemi kullanılılarak matrix tersi alınır
 * */

void intverse(matrix deneme[],int detS){
    double  d,k;
    matrix birim[detS];
    for (int i = 0; i < detS; ++i) {

        birim[i].satirnum = (double *)malloc(sizeof(double)*detS);
    }
    for (int j = 0; j < detS; ++j) {
        for (int i = 0; i < detS; ++i) {

            if(i ==j)
                birim[i].satirnum[j] = 1.0;
            else
                birim[i].satirnum[j] = 0.0;
        }
    }
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
    for (int l = 0; l < detS; ++l) {
        for (int i = 0; i < detS; ++i) {
            deneme[l].satirnum[i] = birim[l].satirnum[i];
            
        }
    }
    for (int m = 0; m < detS; ++m) {
        free(birim[m].satirnum);
    }

}
// matrix 4 parçaya bolunur
void shiftedInverse(matrix matrix1[],int size){
    intverse(matrix1,size/2); // sol ust taraf tersi alınıp yerine yazılır
    matrix temp[size/2];        // n x n lik temp olusturulur
    for (int i = 0; i < size/2; ++i) {
        temp[i].satirnum = (double *)malloc(sizeof(double)*size/2);
    }
    /* ana matrixi sag ust kosesi temp matrixe alınır */
    for (int j = 0; j < size/2; ++j) {
        for (int i = size/2; i < size; ++i) {
            temp[j].satirnum[i - size/2] = matrix1[j].satirnum[i];
        }
        
    }
    // tempin tersi alınır
    intverse(temp,size/2);
    for (int j = 0; j < size/2; ++j) {
        for (int i = size/2; i < size; ++i) {
            matrix1[j].satirnum[i] = temp[j].satirnum[i - size/2];
        }

    }
    // sag ust taraf tekrar orginal matrixe yazılır

    /* ana matrixi sol alt kosesi temp matrixe alınır */
    for (int j = size/2; j < size; ++j) {
        for (int i = 0; i < size/2; ++i) {
            temp[j-size/2].satirnum[i] = matrix1[j].satirnum[i];
        }

    }
    // tempin tersi alınır
    intverse(temp,size/2);
    for (int j = size/2; j < size; ++j) {
        for (int i = 0; i < size/2; ++i) {
            matrix1[j].satirnum[i] = temp[j-size/2].satirnum[i];
        }

    }
    // sol alt taraf tekrar orginal matrixe yazılır

    /* ana matrixi sag alt kosesi temp matrixe alınır */
    for (int j = size/2; j < size; ++j) {
        for (int i = size/2; i < size; ++i) {
            temp[j-size/2].satirnum[i - size/2] = matrix1[j].satirnum[i];
        }

    }
    // tempin tersi alınır
    intverse(temp,size/2);
    for (int j = size/2; j < size; ++j) {
        for (int i = size/2; i < size; ++i) {
            matrix1[j].satirnum[i] = temp[j-size/2].satirnum[i - size/2];
        }

    }
    // sag alt taraf tekrar orginal matrixe yazılır

    for (int k = 0; k < size/2; ++k) {
        free(temp[k].satirnum);
    }


}
/* timeServerile aynı determinant alama */
double determinant(matrix matrix1[],int size){
    /*** yedek alınır ****/

    matrix * temp;
    temp = (matrix *)malloc(sizeof(matrix)*size);
    // Matrix size olusturma
    for (int j = 0; j < size; ++j) {
        temp[j].satirnum = (double *)malloc(sizeof(double)*size);
    }

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            temp[i].satirnum[j] = matrix1[i].satirnum[j];
        }

    }

    for (int k = 0; k <size ; ++k) {
        if(gauss(k,size,temp) == 0){
            return 0.0;
        }
    }

    double result = hesapla(temp,size);
    for (int l = 0; l < size; ++l) {
        free(temp[l].satirnum);
    }
    free(temp);
    return result;

}
// shifted inverse ile aynı mantık ile parça parça concolution alıp tekrar ana matrixe yazılır
double convDeterminant(matrix matrix1[],int size,int client_pid,int count){
   // PMatrix(matrix1,size);
    matrix kernel[3];
    matrix temp[size/2];
    for (int i = 0; i < 3; ++i) {
        kernel[i].satirnum = (double *)malloc(sizeof(double)*3);
    }

    kernel[1].satirnum[1] = 1;


    for (int i = 0; i < size/2; ++i) {
        temp[i].satirnum = (double *)malloc(sizeof(double)*size/2);
    }

    //sol ust
    for (int j = 0; j < size/2; ++j) {
        for (int i = 0; i < size /2; ++i) {
            temp[j].satirnum[i - size/2] = matrix1[j].satirnum[i];
        }

    }


    convolve2DSlow(temp,kernel,3,size/2);
    for (int j = 0; j < size/2; ++j) {
        for (int i = 0; i < size /2; ++i) {
            matrix1[j].satirnum[i] = temp[j].satirnum[i - size/2];
        }

    }
    /**                   sag ust      */
    for (int j = 0; j < size/2; ++j) {
        for (int i = size/2; i < size; ++i) {
            temp[j].satirnum[i - size/2] = matrix1[j].satirnum[i];
        }

    }
    convolve2DSlow(temp,kernel,3,size/2);

    for (int j = 0; j < size/2; ++j) {
        for (int i = size/2; i < size; ++i) {
            matrix1[j].satirnum[i] = temp[j].satirnum[i - size/2];
        }

    }

    /*   sol alt        */
    for (int j = size/2; j < size; ++j) {
        for (int i = 0; i < size/2; ++i) {
            temp[j-size/2].satirnum[i] = matrix1[j].satirnum[i];
        }

    }
    convolve2DSlow(temp,kernel,3,size/2);
    for (int j = size/2; j < size; ++j) {
        for (int i = 0; i < size/2; ++i) {
            matrix1[j].satirnum[i] = temp[j-size/2].satirnum[i];
        }

    }

    convolve2DSlow(temp,kernel,3,size/2);
    for (int j = size/2; j < size; ++j) {
        for (int i = size/2; i < size; ++i) {
            matrix1[j].satirnum[i] = temp[j-size/2].satirnum[i - size/2];
        }

    }

    // matrixi log dosyasına  yazar
    printMatrix(matrix1,size,client_pid,count,"ShiftedConvolution");


    //printf("determinant convolution %.2lf",determinant(matrix1,size));
    return determinant(matrix1,size);





}
//http://www.songho.ca/dsp/convolution/convolution.html
void convolve2DSlow(matrix data[],matrix kernel[], int kernelSizeX, int matrixSize)
{
    int i, j, m, n, mm, nn;
    int kCenterX, kCenterY;                         // center index of kernel
    float sum;                                      // temp accumulation buffer
    int rowIndex, colIndex;


    // find center position of kernel (half of kernel size)
    kCenterX = kernelSizeX / 2;

    kCenterY = kernelSizeX / 2;

    for(i=0; i < matrixSize; ++i)                // rows
    {
        for(j=0; j < matrixSize; ++j)            // columns
        {
            sum = 0;                            // init to 0 before sum
            for(m=0; m < kernelSizeX; ++m)      // kernel rows
            {
                mm = kernelSizeX - 1 - m;       // row index of flipped kernel

                for(n=0; n < kernelSizeX; ++n)  // kernel columns
                {
                    nn = kernelSizeX - 1 - n;   // column index of flipped kernel

                    // index of input signal, used for checking boundary
                    rowIndex = i + m - kCenterY;
                    colIndex = j + n - kCenterX;

                    // ignore input samples which are out of bound
                        sum += matrixSize * rowIndex + colIndex * kernel[ mm ].satirnum[nn];
                }
            }
                        data[i].satirnum[j] = (fabs(sum) + 0.5f);
        }
    }
    //PMatrix(data,matrixSize);
    //return sum
    //return true;
}
