#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include "./lib/inputReader.h"


int controllaLunghezza(char* stringa, int dimensione_esatta, int dimensione_min, int dimensione_max){
int dimensione_stringa = strlen(stringa);
    if(dimensione_esatta != 0){
        if( dimensione_stringa == dimensione_esatta)
            return 1;
        else
            return 0;
    }
    else if(dimensione_min != 0){
        if(dimensione_stringa >= dimensione_min){
            if(dimensione_stringa <= dimensione_max)
                return 1;
            else
                return 0;
        }
        else
            return 0;
    }
    else{
        printf("Errore controllo parametri funzione controllaLunghezza");
        return 0;
    }
}

int doSceltaInt(char* messaggio, int valore_max){
int input_locale;


    do{
        printf("%s", messaggio);
    }while(!getPositive(&input_locale) || input_locale == 0 || input_locale > valore_max);

    return input_locale;

}

int doSceltaIntError(char* messaggio, int valore_max,char * error){
int input_locale;
int i=0;

    do{

        if(i>0)
            printf("%s",error);
        i++;
        printf("%s", messaggio);
    }while(!getPositive(&input_locale) || input_locale == 0 || input_locale > valore_max);

    return input_locale;

}

int doSceltaIntZero(char* messaggio, int valore_max){
int input_locale;

    do{
    printf("%s", messaggio);
    }while(!getPositive(&input_locale)  || input_locale > valore_max);

    return input_locale;

}

double doSceltaDoubleZero(char* messaggio, int valore_max){
double input_locale=-1;

    do{
        printf("%s", messaggio);

    }while(!getPositiveDouble(&input_locale)  || input_locale > valore_max);
   // printf("AAA %f\n",input_locale);
    return input_locale;

}

int doSceltaIntZeroError(char* messaggio, int valore_max, char* error){
int input_locale;
int i = 0;
    do{
        if(i>0)
            printf("%s",error);
        i++;
        printf("%s", messaggio);
    }while(!getPositive(&input_locale)  || input_locale > valore_max);

    return input_locale;

}

char* doSceltaString(char* messaggio, int dimensione_esatta, int dimensione_min, int dimensione_max){
char buff[20];
//int* pt = &buff;
char* pt = buff;
    do{
        printf("%s", messaggio);
        scanf("%s", buff);
    }while(!controllaLunghezza(buff, dimensione_esatta, dimensione_min, dimensione_max));
   return pt;
}

char* doSceltaStringError(char* messaggio, char* error, int dimensione_esatta, int dimensione_min, int dimensione_max){
char buff[20];
//int* pt = &buff;
char* pt = buff;
int i = 0;

    do{
        if(i>0)
            printf("%s",error);
        i++;
        printf("%s", messaggio);
        scanf("%s", buff);

    }while(!controllaLunghezza(buff, dimensione_esatta, dimensione_min, dimensione_max));
   return pt;
}

char * doSceltaStringZero(char* messaggio, int dimensione_esatta, int dimensione_min, int dimensione_max){
char buff[20];
//int* pt = &buff;
char* pt = buff;


    do{
        printf("%s", messaggio);
        scanf("%s", buff);

        if( !strcmp(buff,"0"))
            break;
    }while(!controllaLunghezza(buff, dimensione_esatta, dimensione_min, dimensione_max));
   return pt;
}

char* doCompare () {
  char* buff;
  char* buff1;
  buff = (char*)malloc(sizeof(char)*20);
  buff1 = (char*)malloc(sizeof(char)*20);


    do{
        printf("Inserisci Password:");
        scanf("%s", buff);
        printf("Conferma Password:");
        scanf("%s", buff1);
    }while ( strcmp(buff, buff1) && controllaLunghezza(buff,0, 4, 16));

   return buff;

}

char* strremove(char *str, const char *sub) {
    char *p, *q, *r;
    if ((q = r = strstr(str, sub)) != NULL) {
        size_t len = strlen(sub);
        while ((r = strstr(p = r + len, sub)) != NULL) {
            memmove(q, p, r - p);
            q += r - p;
        }
        memmove(q, p, strlen(p) + 1);
    }
    str[strlen(str)-1]='\0';
    return str;
}






void printTimeVolo(int minuti){

    if ( (minuti / 60) == 1 ) /* display 'x' hour... */
        printf("%01d hour",minuti/60);
    else /* display 'x' hours .... */
        printf("%01d hours",minuti/60);

    if ( (minuti % 60) == 1) /* display 'x' minute... */
        printf(" %01d minute\n", minuti%60);
    else /* display 'x' minutes....*/
        printf(" %01d minutes\n",minuti%60);

}


char * concatenation(char *a, char *b){

char *c = malloc(strlen(a)+strlen(b)+2);
strcpy(c,a);
strcat(c,b);
return c;
}




void upperCase(char * str) {
  char * name;
  name = strtok(str,":");

  // Convert to upper case
  char *s = name;
  while (*s) {
    *s = toupper((unsigned char) *s);
    s++;
  }

}

































