#ifndef List_h
#define List_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "list.h"
#include <string.h>
#include "array.h"
#include "../checkInput.h"
#include "../lib/inputReader.h"
#include <ctype.h>
#include <pthread.h>

struct TUser {
    char* username;
    int x;
    int y;
    int nTerritoriPosseduti;
    int coloreAssegnato;
    char letteraAssegnata;
    char* address;
    int sd_client;
    struct TGame* game;
    struct TUser* next;
};



struct TClient{
    int sd_client;
    char* adress;
};


struct TGame{
    char mappa[20][20];
    int count;
    char arrayLettera[26];
    int arrayColori[26];
    int arrayDisp[26]; // 0 per disp 1 per occupati
    int idGame;
    int partitaInCorso; // 0 la partita non e' mai iniziata, 1 partita e' in corso, 2 partita terminata
    unsigned int tempo_gioco; 
    int numeroDiTerritoriDaConquistare;
    char* vincitore;
    struct TUserInsideGame* utenti;
    struct TGame* next;
};

struct TUserInsideGame{
    char* username;
    int nTerritoriPosseduti;
    struct TUserInsideGame* next;
};

typedef struct TGame* Game;

typedef struct TUser* User;

typedef struct TClient* Client;

typedef struct TUserInsideGame* UserIG;



/*********************** FUNZIONI PER LA STRUTTURA User ***********************/

User initNodeUser(char* username, int x, int y, char* address, int coloreAssegnato, char letteraAssegnata, int sd_client);
User appendNodeUser(User head, User newUser);
User removeUserByUsername(User head, char* username);
void freeUserList(User head); // Dealloca la lista interamente
void printUserList(User head);
// cambiamento sta a indicare quanti territori sono stati conquistati (+1,2,3) o quanti ne'sono stati 
// persi (-1,-2....)
// ritorna 1 nel caso ha trovato e aggiornato user 0 altrimenti
int updateUserTerritoriByLettera(User head, char letteraAssegnata, int cambiamento);
int isLogged(User head, char* username); // return 1 se connesso 0 altrimenti
char* getUsernameByLetteraNelGioco(User head, char lettera, int idGame);
void rimozioneUtenteDallaMappa(User localUser);
/*******************************     FINE     ***********************************/

/*********************** FUNZIONI PER LA STRUTTURA Game *************************/
Game initNodeGame(unsigned int tempo_secodi, int numeroDiTerritoriDaConquistare);
Game appendNodeGame(Game head, Game G);
Game removeNodeGameById(Game head, int idGame);
Game findGameDisponibile(Game head); // cerca nella lista delle Partite la prima disponibile
void startGameTimer(Game G, pthread_mutex_t* mutex, pthread_cond_t* cond);
void printGameInfo(Game G);
void printAllGameInfo(Game head);

/*******************************     FINE     ***********************************/

/*********************** FUNZIONI PER LA STRUTTURA UserIG ***********************/
UserIG initNodeUserIG(char* username);
UserIG appendNodeUserIG(UserIG head, UserIG G);
UserIG removeNodeUserIGbyUsername(UserIG head, char* username);
void freeListUserIG(UserIG head);
void incrementaNumTerritoriPosseduti(UserIG head, char* username);
void decrementaNumTerritoriPosseduti(UserIG head, char* username);
void MergeSort(UserIG* headRef);
UserIG ordinaPerNumeroTerritori(UserIG a, UserIG b);
void FrontBackSplit(UserIG source, UserIG* frontRef, UserIG* backRef);
void printAllUserIG(UserIG head);
char* getConnectedUsersInGame(UserIG head);
/*******************************     FINE     ***********************************/

/*********************** FUNZIONI PER LA STRUTTURA Client ***********************/
Client initNodeClient(char* adress, int sd_client);
/*******************************     FINE     ***********************************/



#endif
