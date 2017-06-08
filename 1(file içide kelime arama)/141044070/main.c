/* **************************************************************
 *  Rıdvan Demirci 141044070                                    *
 *   Sistem programlama Hw1         																												*
 * 			Eger editorun tab bosluk 1 karakter olursa 															*
 *					editor ile satir sutun ayni sonuc verir                  *
 ****************************************************************/

#include<stdio.h>
/* ************** SIGNATURES ******************* */

    // Stringin boyutunu return eder
int StringSize(char str[]);
    // file da string olup olmadigina bakar
int IshasString(FILE *StringFile,char * aranan,int size);
void search(FILE *file,char *aranan,int size);
/* *********************************************  */
int main(int argv,char *argc[]){

        // eger beklenen argumanlar gelmez ise
	if(argv != 3){

		printf("I searc String given File\n");
		return 1;
	}

	int size=0;//aranacak sitring size

    size = StringSize(argc[1]);
    char * FileName = argc[2];

    char * StrAranan = argc[1]; // Dosyada aranacak olan
	FILE * fpList;  //liste file pointer

	fpList = fopen(FileName,"r"); // dosya acma
	if(fpList == NULL)
	{
		printf("No found File\n");
		return 1;
	
	}
    search(fpList,StrAranan,size);  // Dosyayı arama fonksyonunu cagirma

	fclose(fpList);     // Dosya kapama
    fpList == NULL;     // pointer Null
return 0;
}

// Bu fonksyon gelen stringin boyutunu return eder
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

void search(FILE *file,char *aranan,int size){
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
                    printf("[%d, %d] found  '%s' first characte\n",row, col,aranan);
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
    printf("%d times '%s' founded \n",counter,aranan); // counter ekrana yazılır


}
