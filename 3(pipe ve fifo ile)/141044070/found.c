/* *********************************************************
 * Rıdvan Demirci 141044070                                *
 *      Sistem Programlama Homework 3                      *
 * ******************************************************* */
#include "found.h"
/*
 * aranacak olan word kelimesinin boyutunu belirler
 */
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
/***
 * buldugu satır ve sutunlar log.log dosyasına yazar
 * return degerini de kaçtane buldugunu dondurur
 * sonra o deger de pipe veya FIFO ile kullanılır
 *
 */
int search(char *fileName,char *aranan,int size,char *onlyFileName){
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
                    fprintf(logfp,"%s: [%d, %d] found  '%s' first character\n",onlyFileName,row, col,aranan);
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
    return counter;
}