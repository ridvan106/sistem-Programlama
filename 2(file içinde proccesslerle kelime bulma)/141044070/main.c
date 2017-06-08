/* *********************************************************
 * Rıdvan Demirci 141044070                                *
 *      Sistem Programlama Homework 2                      *
 * ******************************************************* */

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "found.h"
#define Path 256   /* maximum alacagi path */


int isFile(char *str); /* Dosyanın file veya directory olup olmamasına bakar */
/*
 * proccess olarak ilk ödevdeki dosyalarda arama yapar eger directory ise
 * tekrar en basa recursive call yapilir
 */
void searcFiles(char * path,char word[],int size);

/*
 * En son file sonuna kaçtane string buldugunu yazar
 */
void totalSize(char *fileName,char *word);
/*
 * Main
 */
int main(int argc , char *argv[]) {
    /* usage  */
    if(argc != 3) {
        fprintf(stderr, "Usage: %s 'string' directory_name\n", argv[0]);
        return 1;
    }
    pid_t parent = getpid();
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
        searcFiles(dirName,word,size);

    }
    else if(pid >1) {
         wait(NULL);
            totalSize(logFile, word);

                  /* 1kactane bulunmus ise onu yazar */

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
    fprintf(fp,"%d '%s' were found in total  \n",totalSize,word );

    fclose(fp);

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
    struct dirent *direntp = NULL;
    DIR *dirp = NULL;
    if ((dirp = opendir(path)) == NULL) { /* eger yolda hata var ise */
        perror ("FAİLED read Path");  /* perrror hata mesajı */
        return;
    }
    while ((direntp = readdir(dirp)) != NULL){  /* bos olana kadar verileri okt */

    if(strcmp("..",direntp->d_name)==0 || strcmp(".",direntp->d_name)==0) {
            /* Junk  ust dizin veya bulundugu deger*/
    }
    else {
        /* eger file ise */
        if (isFile(direntp->d_name) == 1) {
            pid_t pid = fork();

            if (pid == 0) { /* child proccess ise dokuman yolunu search e yollar */
                char newPath[Path] = "";
                strcat(newPath, "");
                strcat(newPath, path);
                strcat(newPath, direntp->d_name);
                search(newPath, word, size, direntp->d_name);

                break;/*proccess isi bitirdikten sonra cikar */
            }
            else if(pid < 0){
                printf("fork olusturma hatası");
                exit(2);
            }
            else {
                /* hic birsey yapmadan devam eder */

            }
        }

            else{
                /* eger file ise mevcut proccess e fork yaparak torun olusturr */
            char newpath[256]="";
                strcat(newpath,path);
                strcat(newpath,direntp->d_name);
                strcat(newpath,"/");
                pid_t pid = fork();
                if(pid == 0) {
                    /* yol belirledikten sonra tekrar aynı fonksyona yollar */
                    searcFiles(newpath,word,size);
                    break;
                }
                    else if (pid < 0){
                    printf("Fork Eror");
                    exit(2);
                }
                else{

                   /* parent kısmı birşey yapmıyo */

                }

            }

        }


    }
    while(wait(NULL) != WAIT_ANY);
    /* tüm açılan klasörleri kapamak için
    / ders kitabında mevcut */

    while ((closedir(dirp) == -1) && (errno == EINTR)) ;

}