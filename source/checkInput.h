#ifndef CHECKINPUT_H_INCLUDED
#define CHECKINPUT_H_INCLUDED


//Controlla se la stringa ha una dimensione esatta
//
int controllaLunghezza(char* stringa, int dimensione_esatta, int dimensione_min, int dimensione_max);

int doSceltaInt(char* messaggio, int valore_max);

int doSceltaIntZero(char* messaggio, int valore_max);

int doSceltaIntError(char* messaggio, int valore_max,char * error);

int doSceltaIntZeroError(char* messaggio, int valore_max, char* error);

char* doSceltaStringZero(char* messaggio, int dimensione_esatta, int dimensione_min, int dimensione_max);



//Chiede in input una stringa e stampa il messaggio finche la stringa non soddisfa le dimensioni indicate, mettere 0 nel campo nel caso in cui un paramentro deve essere ignorato.
//Ritorna l'indirizzo della stringa che contiene l'input
char* doSceltaString(char* messaggio, int dimensione_esatta, int dimensione_min, int dimensione_max);
//Chiede in input una stringa e stampa il messaggio finche la stringa non soddisfa le dimensioni indicate, mettere 0 nel campo nel caso in cui un paramentro deve essere ignorato.
//Ritorna l'indirizzo della stringa che contiene l'input
//In pi� stampa il messaggio di errore
char* doSceltaStringError(char* messaggio, char* error, int dimensione_esatta, int dimensione_min, int dimensione_max);

void printTimeVolo(int minuti);
char* doCompare();
char* strremove(char *str, const char *sub);
void upperCase(char * str);

char * concatenation(char *a, char *b);
#endif // CHECKINPUT_H_INCLUDED

