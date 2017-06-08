/*
	Rıdvan Demirci
		141044070
		Sistem Programlama 



*/
#define _POSIX_SOURCE
#define _BSD_SOURCE
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
FILE * fp,*getShowresult,*writeLog;
// olme veya oldurulme sinyali gelirse yapması gerekenler
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
// bsakası tarafından oldurulurse
void geldi(int no){
    printf("\nbaskası tarafından  oldurme sinyali geldi");
    fprintf(writeLog,"\n baskası tarafından  oldurme sinyali geldi");
    fclose(writeLog);
    fclose(getShowresult);
    fclose(fp);
    remove("result.txt");
    remove("showLog.txt");
    exit(1);
}
// tum programları oldurur
void signalKill(int SignalNo){
    perror("Oldur sinyali geldi");
     fprintf(writeLog,"\n Oldur sinyali geldi");
    fclose(writeLog);
    killAll(getpid());
    remove("result.txt");
    remove("showLog.txt");
    exit(1);
}

int main(){
    /** sinyallerin handle edilmesi */

    signal(SIGUSR2,geldi);
    signal(SIGINT,signalKill);
    /*  *   *       *   *       *   */
    /* Pid Listesine kendi pidsini yazma    */
    FILE *pidList;
    pidList = fopen("pidList.txt","a+");
    fprintf(pidList,"%d \n",getpid());
    fclose(pidList);
    /*  *   *   *   *   *   *   *   *   */
    /* Seewhatın yazdıgı txt dosyasından ekrana veya log dosyasına yazılacak formatı olusturma */

    getShowresult = fopen("showLog.txt","r");
    writeLog = fopen("log/ShowResult.log","a+");
    fp = fopen("result.txt","r");
    if(fp == NULL){
        fprintf(stderr,"once timeServer ve  SeeWhat calistirilmali");
        return 0;
    }
    else{
        char temp = ' ';
        char temp2 = ' ';
        int counter = 1;    //sıralama için yapılan counter
        int counter2 = 1;   // tum satırlar için
        // log dosyasına birinci satır için 1 yazar
        fprintf(writeLog,"%d ",counter++);
        while(1) {


            int status = fscanf(fp,"%c",&temp);
            // SeeWhat result dosyasına yazmıs ise
            if(status == 1)
            printf("%c", temp); // o formatı once ekrana yazar
            int status2 = fscanf(getShowresult,"%c",&temp2);    // eger getResulta yazılan deger var ise
            if(status2 == 1){
                if(temp2 == '\n'){  // alt satıra inme karakteri 3 satırda bir counter ekrana yazılır
                    fprintf(writeLog,"\n");
                    if(counter2 % 3 == 0)
                    fprintf(writeLog,"%d ",counter++);

                    counter2++;
                }
                else{
                    fprintf(writeLog,"%c",temp2);
                }

            }


        }


    }








    return 0;
}
