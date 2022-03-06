#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "array.h"
#include "list.h"
#include <string.h>
#include <ctype.h>

void inizializaMappa(char mappa[20][20])
{
    for (int i = 0; i < 20; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            mappa[i][j] = '-';
        }
    }
}


void initColoriLettere(char arrayLettera[26], int arrayColori[26], int arrayDisp[26]){
    int j = 21;
    for (int i = 0; i < 26; i++)
    {
        arrayLettera[i] = i + 65;
        arrayDisp[i] = 0;
        if (i < 6)
            arrayColori[i] = i + 9;
        else
        {
            arrayColori[i] = j;
            j += 10;
        }
    }
}

int getIndiceDispForLetteraColore(int arrayDisp[26]){
    for (int i = 0; i < 26; i++)
    {
        if (arrayDisp[i] == 0)
            return i;
    }
    return -1;
}


int getPosizoneInizialeCodificata(char mappa[20][20]){
    srand(time(NULL));
    int dim;
    int array[400];
    dim = riempiArrayConPosizioniLibereCodificate(mappa,array);

    if (dim == 0)
        return -1;

    int indice = rand() % (dim);
    int x, y;

    return array[indice];
}

int riempiArrayConPosizioniLibereCodificate(char mappa[20][20], int *array)
{
    int k = 0;
    for (int i = 0; i < 20; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            if (mappa[i][j] == '-')
            {
                *array = (i * 20) + j;
                array = array + 1;
                k++;
            }
        }
    }

    return k;
}


void stampaMappa(char mappa[20][20])
{
    for (int i = 0; i < 20; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            printf(" ");
            printf("%c", mappa[i][j]);
            printf(" ");
        }
        printf("\n");
    }
}