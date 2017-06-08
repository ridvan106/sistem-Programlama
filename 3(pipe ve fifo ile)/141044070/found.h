//
// Created by oem on 07.03.2017.
//

#ifndef HW2_FOUND_H
#define HW2_FOUND_H

#include <stdio.h>
#include <stdlib.h>
int StringSize(char str[]);
// file da string olup olmadigina bakar
int IshasString(FILE *StringFile,char * aranan,int size);
int search(char *fileName,char *aranan,int size,char *onlyFileName);
#endif //HW2_FOUND_H
