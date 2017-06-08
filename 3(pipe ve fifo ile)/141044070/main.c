/**********************************************************************
 *  Rıdvan Demirci 141044070                                          *
 *      Sistem Programlama HW3                                        *
 *          Proccesleri PIPE ve FIFO kullanarak Haberlerştirme        *
 *              Txt den file pipe                                     *
 *                  File dan File FIFO                                *
 *                                                                    *
 **********************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "found.h"
#define INTEGER 4
#define FIFO_PERM ( S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
void searchFiles(char *pathName,char *word,int size);
int main(int argc,char *argv[]) {
    /* usage  */
    if(argc != 3) {
        fprintf(stderr, "Usage: %s 'string' directory_name\n", argv[0]);
        return 1;
    }
    /* Kullanıcıdan gelen direktory argümanı alır */
    char dirName[PATH_MAX]="";
    strcat(dirName,argv[2]);
    /* ******************************************* */
    /* Aranacak kelimeyi word degiskenine alir */
    char * word = argv[1];
    int size = StringSize(argv[1]); /* gelen string boyutu */

    char * logFile ="log.log"; /* en son olusturacagi Log dosyasi */
    FILE *fp;

    /*
     * Log dosyasını sıfırlamak icin kullanılır
     * daha oncede calisacagi icin once log dosyasini temizler
     * yoksa acar
     */

    fp =fopen(logFile,"w+");
    fclose(fp); /* close file */
    pid_t pid;
    pid = fork();
    char pipeName[PATH_MAX] = "";
    /**
     * pipeAcmak için yeni ad lazım onun için normal dosyaların
     * sonu 'P' işareti konur
     */
    strcat(pipeName,dirName);
    strcat(pipeName,"P");
    mkfifo(pipeName,FIFO_PERM); // ana directory için FIFOs "denemeP"
    if(pid == 0){
        searchFiles(dirName,word,size); // Arama Yapıcak Fonksyon

    }
    else if (pid > 1){

        int t;
        t = open(pipeName,O_RDONLY); // aramadan sonra gelecek deger için o fifo acılır
        int val,count = 0;
        while ( read(t,&val,INTEGER)> 0 ){    // yazılacak olan tüm verileri okur

            if(val == -1){          //eger dosya olsuturma hatası
            	unlink(pipeName);
                return -1;
            }
            count += val;       // ve tümverileri toplar

        }
        fp =fopen("log.log","a+");
        fprintf(fp,"%d '%s' were founded",count,word);
        fclose(fp); /* close file */
        unlink(pipeName); // ilk olusan ana dizindeki pipe silinir
    }
    return 0;
}
void searchFiles(char *pathName,char *word,int size){

    DIR * dir;      // directory pointer
    struct  dirent *dirent1; // dirent stract



    /**
     * eger dosya olusturma hatası varsa FIFO ya
     * -1 yazar mainde o handle edilir
     */
    if((dir = opendir(pathName)) == NULL){
        printf("Dosya olusturma hatasi \n");
        char hataPipeName[PATH_MAX] = "";
        strcat(hataPipeName,pathName);
        strcat(hataPipeName,"P");
        int temp = open(hataPipeName,O_WRONLY);
        int hata = -1;
        write(temp,&hata,INTEGER);
        close(temp);
        return;
    }
    while((dirent1 = readdir(dir)) != NULL) {
        // ust dizin ve suanki dizin IGNORE edilir
        if (strcmp(".", dirent1->d_name) != 0 && strcmp("..", dirent1->d_name) != 0) {

            // regular file ise yani txt felan
            if (dirent1->d_type == DT_REG) {
                int fd[2];
                pipe(fd); // directory ile txt arası pipe acilir
                /**
                 * procces txt deki pipe kactane kelimeden bulundugunu dondurur
                 * maindeki directory ise onu okur ve bir ust procces e yazar
                 */
                pid_t pid = fork();
                if (pid == 0) {
                    /*
                     * aranacak olan txt yi search e yollar
                     * search fonksyonu ise kactane bulunacagini
                     * return eder.
                     */
                    char npath[PATH_MAX]="";
                    strcat(npath,pathName);
                    strcat(npath,"/");
                    strcat(npath,dirent1->d_name);
                    int adet=0;
                    adet = search(npath,word,size,dirent1->d_name);
                    close(fd[0]);
                    write(fd[1],&adet,INTEGER);// kactane dosya bulundugu pipe a yazilir
                    close(fd[1]);
                    break; // child proccesses dosyadan cikar gider
                } // main proccess yani txt lerin maini
                else{//DIRECTORY

                    int get=0;  // childden gelen veriyi okuycak
                    read(fd[0],&get,INTEGER);// parent ile child arasındaki pipe iletişimi

                    // okunan degeri Directory kendinden bir ust FIFO a aktarır

                    char pipeName[PATH_MAX] = "";
                    strcat(pipeName,pathName);
                    strcat(pipeName,"P"); // FIFO dosyalarının sonuna 'P' yazılır
                    // ust FIFO ya kactane bulundugunun  yazılması
                    int temp = open(pipeName,O_WRONLY);
                    write(temp,&get,INTEGER);

                }

            }
                // eger DIRectory ise
            else if(dirent1->d_type == DT_DIR) { // folder ise haberleşme FIFO ile olucak
                /********** Yeni directory adi olsuturma ********/
                char npath[PATH_MAX] = "";
                strcat(npath, pathName);
                strcat(npath, "/");
                strcat(npath, dirent1->d_name);
                /*************************************************/
                /*********** Yeni PipeName olusturma *************/
                char pipeName[PATH_MAX] = "";
                strcat(pipeName,npath); // pipe ismi olusturma
                strcat(pipeName,"P");   // dosya adıyla aynı olmasın diye sonuna P

                mkfifo(pipeName,FIFO_PERM); // isimli pipe olusturma
                /*************************************************/
                pid_t  pid = fork();
                if(pid == 0) {
                    //Directory nin ici için tekrar searc edilir
                    searchFiles(npath, word, size); // fifoya yazma işlemi burada yapılır

                    break; // sonra child donguden cikar gider
                }
                else{
                    // directory nin kendi altındaki txt leri veya childları okuması
                    /************** FIFO OKUma ***************** */
                    int buf,count = 0,temp;
                    temp = open(pipeName,O_RDONLY);
                    while(read(temp,&buf,INTEGER) > 0){ // tum yazılma işelri bitene kadar bekler
                        count += buf;
                    }
                    /************** End Read ******************* */
                    /*
                     * Bu bi directory oldugu icin aldıgı toplam veriyi
                     * kendi ust directory sine aktama islemi gene
                     * FIFO ile yapılır
                     */
                    char ustpipe[PATH_MAX] ="";
                    strcat(ustpipe,pathName);
                    strcat(ustpipe,"P"); // ust pipeın adı
                    int t;
                    t =open(ustpipe,O_WRONLY);
                    write(t,&count,INTEGER);  // integer yazıcagı için 4 byte



                }
                /**
                 * en son tum pipe işlemler bittikten sonra
                 * açılan FIFO lar silinir.
                 */
                unlink(pipeName);
            }
        }

    }
    /**
     * Açılan klasorler kapatılır Leak olusmaması için
     */
    while ((closedir(dir) == -1) && (errno == EINTR)) ;
}
