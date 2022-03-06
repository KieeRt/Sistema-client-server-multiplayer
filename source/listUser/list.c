
#include "list.h"


// gcc -o lista list.c array.h array.c ../checkInput.h ../checkInput.c ../lib/inputReader.h ../lib/inputReader.c 

/*********************** FUNZIONI PER LA STRUTTURA User *************************/
User initNodeUser(char* username, int x, int y, char* address, int coloreAssegnato, char letteraAssegnata, int sd_client){
    User L = (User)malloc(sizeof(struct TUser));
    L->username = username;
    L->x = x;
    L->y = y;
    L->nTerritoriPosseduti = 0 ; 
    L->coloreAssegnato = coloreAssegnato ; 
    L->letteraAssegnata = letteraAssegnata ;
    L->address = address ;  
    L->sd_client = sd_client;
    L->game = NULL;
    L->next = NULL;
    return L;
}

User appendNodeUser(User head, User newUser) {
    if (head != NULL) {
            head->next = appendNodeUser(head->next, newUser);
    }
    else {
        return newUser;
    }
    
return head;
} 
User removeUserByUsername(User head, char* username) {
    if (head != NULL) {
        if (strcmp(head->username, username)==0) {
            User tmp = head->next;
            free(head);
            return tmp;
        }
        head->next = removeUserByUsername(head->next, username);
    }
    return head;
}


void freeUserList(User head) {
    if (head != NULL) {
        freeUserList(head->next);
        free(head);
    }
}
void printUserList(User head) {
    if (head != NULL) {
        printf("username:%s\n", head->username);
        printf("x:%d\n", head->x);
        printf("y:%d\n", head->y);
        printf("Numero territori posseduti:%d\n", head->nTerritoriPosseduti);
        printf("colore assegnato:%d\n", head->coloreAssegnato);
        printf("lettera assegnata:%c\n", head->letteraAssegnata);
        printf("Ip adress:%s\n", head->address);
        printf("file sd_client:%d\n\n", head->sd_client);
        printUserList(head->next);
    }
}
int updateUserTerritoriByLettera(User head, char letteraAssegnata, int cambiamento){
    User tmp = head;
    while(tmp != NULL){
        if(tmp->letteraAssegnata != 0 && tolower(tmp->letteraAssegnata) == tolower(letteraAssegnata)){
            tmp->nTerritoriPosseduti = tmp->nTerritoriPosseduti + cambiamento;
            return 1;
        }
        tmp = tmp->next;
    }
    return 0;
}
int isLogged(User head, char* username){
    User tmp = head;
    while(tmp != NULL){
        if(tmp->username!=NULL && strcmp(tmp->username, username) == 0){
            // trovato username connesso
            return 1;
        }
  
        tmp = tmp->next;
    }
   
    return 0;
}

char* getUsernameByLetteraNelGioco(User head, char lettera, int idGame){
    User tmp = head;
    char* username = malloc(sizeof(char)*36);
    while(tmp){
       
        if(tolower(tmp->letteraAssegnata) == tolower(lettera) && tmp->game->idGame == idGame){
            
            strcpy(username, tmp->username);
            break;
        }
        tmp = tmp->next;
    }
    return username;
}
void rimozioneUtenteDallaMappa(User localUser){
    char letteraUser = tolower(localUser->letteraAssegnata);

    for (int i = 0; i < 19; i++)
    {
        for (int j = 0; j < 19; j++)
        {
            if (tolower(localUser->game->mappa[i][j]) == letteraUser)
            {
                localUser->game->mappa[i][j] = '-';
            }
        }
    }
}

/*******************************     FINE     ***********************************/

/*********************** FUNZIONI PER LA STRUTTURA Game *************************/

Game initNodeGame(unsigned int tempo_secodi, int numeroDiTerritoriDaConquistare){
    //srand(time(NULL));
    Game G = (Game)malloc(sizeof(struct TGame));
    G->count = 0;
    inizializaMappa(G->mappa);
    initColoriLettere(G->arrayLettera,G->arrayColori, G->arrayDisp);
    G->idGame = 1 + rand()% RAND_MAX;
    G->partitaInCorso = 0;
    G->tempo_gioco = tempo_secodi;
    G->numeroDiTerritoriDaConquistare = numeroDiTerritoriDaConquistare;
    G->vincitore = malloc(sizeof(char)*64);
    G->utenti = NULL;
    G->next = NULL;
    return G;
}
Game appendNodeGame(Game head, Game G){
    if (head != NULL) {
            head->next = appendNodeGame(head->next, G);
    }
    else {
        return G;
    }
    return head;
}
Game removeNodeGameById(Game G, int idGame){
   if (G != NULL) {
        if (G->idGame == idGame){
            Game tmp = G->next;
            freeListUserIG(G->utenti);
            free(G->vincitore);
            free(G);
            return tmp;
        }
        G->next = removeNodeGameById(G->next, idGame);
    }
    return G;   
}
Game findGameDisponibile(Game G){
    Game tmp = G;
    while(tmp != NULL){
        if(tmp->partitaInCorso == 1){
            return tmp;
        }
        tmp = tmp->next;
    }
    return NULL;
}
void startGameTimer(Game G, pthread_mutex_t* mutex, pthread_cond_t* cond){

    if (G->partitaInCorso == 0)
    {
        pthread_mutex_lock(mutex);
  
        G->partitaInCorso = 1;
     
        pthread_cond_signal(cond);
        pthread_mutex_unlock(mutex);
       
    }
}

void printGameInfo(Game G) {
    if (G != NULL) {
        printf("\n\n***** INFORMAZIONI SUL GIOCO *****\n");
        stampaMappa(G->mappa);
        
        for(int i = 0; i < 26; i++){
            printf("%c\t", G->arrayLettera[i]);
        }
        printf("\n");
        for(int i = 0; i < 26; i++){
            printf("%d\t", G->arrayColori[i]);
        }
        printf("\n");
        for(int i = 0; i < 26; i++){
            printf("%d\t", G->arrayDisp[i]);
        }
        printf("\n");
        printf("[idGame]:%d\n", G->idGame);
        printf("[paritaInCorso]:%d\n", G->partitaInCorso);
        printf("[numeroGiocato]:%d\n", G->count);
        printf("[tempo gioco]:%d\n", G->tempo_gioco);
        printf("[Numero di Territori da conquistare]:%d\n", G->numeroDiTerritoriDaConquistare);
        printf("[Vincitore]:%s\n", G->vincitore);
        printAllUserIG(G->utenti);
        printf("\n\n");
    }
}
void printAllGameInfo(Game G) {
    if(G != NULL){
        printGameInfo(G);
        printAllGameInfo(G->next);
    }
}


/*******************************     FINE     ***********************************/


/*********************** FUNZIONI PER LA STRUTTURA UserIG *************************/

UserIG initNodeUserIG(char* username){
    UserIG G = (UserIG)malloc(sizeof(struct TUserInsideGame));
    G->username = malloc(sizeof(char)*32);
    strcpy(G->username, username);
    G->nTerritoriPosseduti = 0;
    G->next = NULL;
}

UserIG appendNodeUserIG(UserIG head, UserIG G){
    if (head != NULL) {
        head->next = appendNodeUserIG(head->next, G);
    }
    else {
        return G;
    }
    return head;
}

UserIG removeNodeUserIGbyUsername(UserIG head, char* username){
   if (head != NULL) {
        if (strcmp(head->username,username)== 0){
            UserIG tmp = head->next;
            free(head);
            return tmp;
        }
        head->next = removeNodeUserIGbyUsername(head->next, username);
    }
    return head;   
}

void freeListUserIG(UserIG head){
    if(head != NULL){
        freeListUserIG(head->next);
        free(head->username);
        free(head);
    }
}

void incrementaNumTerritoriPosseduti(UserIG head, char* username){
    UserIG tmp = head;
    while(tmp){
        if (strcmp(tmp->username,username)== 0){
            tmp->nTerritoriPosseduti++;
            break;
        }
        tmp = tmp->next;
    }
}
void decrementaNumTerritoriPosseduti(UserIG head, char* username){
    UserIG tmp = head;
    while(tmp){
        if (strcmp(tmp->username,username)== 0){
            tmp->nTerritoriPosseduti--;
            break;
        }
        tmp = tmp->next;
    }
}

void MergeSort(UserIG* headRef){
    UserIG head = *headRef;
    UserIG a;
    UserIG b;

    if((head == NULL) || (head->next == NULL)){
        return;
    }
    FrontBackSplit(head, &a, &b);

    MergeSort(&a);
    MergeSort(&b);

    *headRef = ordinaPerNumeroTerritori(a,b);
}

UserIG ordinaPerNumeroTerritori(UserIG a, UserIG b){
    UserIG result = NULL;

    if(a == NULL)
        return (b);
    else if(b == NULL)
        return (a);

    if(a->nTerritoriPosseduti >= b->nTerritoriPosseduti){
        result = a;
        result->next = ordinaPerNumeroTerritori(a->next, b);
    }
    else{
        result = b;
        result->next = ordinaPerNumeroTerritori(a, b->next);
    }
    return result;
}

void FrontBackSplit(UserIG source, UserIG* frontRef, UserIG* backRef){
    UserIG fast;
    UserIG slow;
    slow = source;
    fast = source->next;

    while(fast != NULL){
        fast = fast->next;
        if(fast != NULL){
            slow = slow->next;
            fast = fast->next;
        }
    }

    *frontRef = source;
    *backRef = slow->next;
    slow->next = NULL;
}
void printAllUserIG(UserIG head){
    if(head != NULL){
        printf("[username]:%s\n", head->username);
        printf("[numero territori conquistati]:%d\n", head->nTerritoriPosseduti);
        printAllUserIG(head->next);
    }
}
char* getConnectedUsersInGame(UserIG head){
    UserIG tmp = head;
    int first = 1; // flag per descriminare la forma del messaggio
    char* buf = malloc(sizeof(char)*1024);
    char* numTerr = malloc(sizeof(char)*3);
    char lettera;
    while(tmp != NULL){
        if(tmp->username != NULL){
          sprintf(numTerr, "%d", tmp->nTerritoriPosseduti);
            if(first == 1){
                buf = concatenation(tmp->username, concatenation(":", numTerr));
                memset(numTerr, 0, 3);
                first = 0;
            }
            else{
                buf = concatenation(buf, "-");
                buf = concatenation(buf, concatenation(tmp->username, concatenation(":", numTerr)));
                memset(numTerr, 0, 3);  
            }
        }
            
        tmp = tmp->next;
    }
    // user1:10-user2:20-user3:12

    return buf;
}


/*******************************     FINE     ***********************************/

/*********************** FUNZIONI PER LA STRUTTURA Client *************************/
Client initNodeClient(char* adress, int sd_client){
    Client C = (Client)malloc(sizeof(struct TClient));
    C->adress = adress;
    C->sd_client = sd_client;
}
/*******************************     FINE     ***********************************/



