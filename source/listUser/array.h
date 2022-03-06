#ifndef ARRAY_H
#define ARRAY_H

#include "list.h"
void inizializaMappa(char mappa[20][20]);
/*
*Inizializza arrayLettera con alfabeto (codice ASCCI da 65 a 90)
*Inizializza arrayColori per utilizzo colorazione ASCII a 8 bit
*Inizializza arrayDisp con valori 0, in questo modo rendendo tutte le lettere e i colori disponibili
*/
void initColoriLettere(char arrayLettera[26], int arrayColori[26], int arrayDisp[26]); 
/*
*ritorna il primo indice disponibile dal arrayDisp[26]
*Array usato per segnalare quali lettere e colore sono usate e quali sono libere.
*/
int getIndiceDispForLetteraColore(int arrayDisp[26]);
/*
*Ritorna un valore codificato che indica la posizione iniziale dell'utente nella mappa
*altrimenti -1 (nel caso in cui non ci sono posti disponibili)
*usare val/20 per decodificare coordinate x
*usare val%20 per decodificare coordinate y
*/
int getPosizoneInizialeCodificata(char mappa[20][20]);
/*
*popola array con posiozioni(x,y) libere sulla mappa codificate  
*ritorna la dimensione dell'array (il numero di posizioni libere), vedere getPosizoneInizialeCodificata per decodifica
*/
int riempiArrayConPosizioniLibereCodificate(char mappa[20][20], int *array);
void stampaMappa(char mappa[20][20]);
#endif
