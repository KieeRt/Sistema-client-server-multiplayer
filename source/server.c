#include <stdlib.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "file.h"
#include "listUser/list.h"
#include "listUser/array.h"
#include "checkInput.h"
#define __USE_GNU
int err_th = -1;
char ok[2] = "1\0";
char error[2] = "0\0";
char msLogged[2] = "3\0";

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


pthread_mutex_t mutexListaUser = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexListaGame = PTHREAD_MUTEX_INITIALIZER;

Game infGioco;
/*
*Codice per compilare
*gcc -o server server.c file.h file.c listUser/array.h listUser/array.c listUser/list.h listUser/list.c checkInput.h checkInput.c lib/inputReader.h lib/inputReader.c -lpthread
*gcc -w server.c file.h file.c list.h list.c checkInput.h checkInput.c lib/inputReader.h lib/inputReader.c -lpthread
*/

User head;
/*
*Gestisco rottura della pipe con terminazione del thread
*/
void gestioneSIGPIPE(int tid);

void *gestisci(void *);
void *gestisciTempo(void *val);

/*
*Inizializza la lettera e il colore del nuovo utente e li invia al client
*ritorna la lettera assegnata
*/
char initColoreLetteraUtente(User localUser);

/*
*mando la mappa al client
*ritorna 1 nel caso di successo
*/
int sendMappa(User localUser, char mappa[20][20]);
/*
*manda la lista di utente al client sotto forma di una stringa
*primoGiocatore:n-secondoGiocatore:m-terzoGiocatore ...
*/
int sendGiocatori(User localUser, UserIG *listaUtentiConnessi);
/*
*mando il tempo rimanente alla chiusura della partita all'user
*/
int sendTime(User localUser, unsigned int tempo_gioco);
/*
*mando il numero di territori da conquistare
*/
int sendTerritoriDaConquistare(User localUser, int numeroDiTerritoriDaConquistare);
/*
*gestisce tutto riguardante utente loggato
*/
void prePartita(User localUser);
/*
*imita la nuova partita
*/
int nuovaPartita(User localUser);
/*
*esegue logout del utente, controllando se e' presente nella lista di giocatori e rimuovendolo 
*controllando se ha dei possedimenti nella mappa, cancella tutte le sue conquiste dalla mappa
*/
void LogOut(User localUser);
/*
*setta i valori di localUser con i valori di default
*/
void esciDallaPartita(User localUser);
/*
*Funzione di procedura, esegue:
1)Cerca lettera e colore disponibile (initCOloreLetteraUtente)
2)Assegna il colore e lettere al utente e lo comunica al client (initCOloreLetteraUtente)
3)Assegna la posizione (x,y ) iniziale disponibile al utente getPosizoneInizialeCodificata() 
4)Salva utente nella matrice di rappresentazione mappa[20][20]
5)invia la mappa al client se ha trovato posizioni iniale disponibile, altrimenti manda un messaggio di errrore
*RETURN: ritona 1 nel caso in cui ha effettuato tutto correttamente -1 altrimenti
*/
int inizializzaUtenteNelGioco(User localUser);

/*
*controlla se può attuale User può effettuare il movimento "aswd".
*controllo e' suddiviso in:
1)controllo se il movimento non porta fuori dalla mappa
2)controllo se il movimento non porta sulla posizione nella quale attualmente e' presente un'altro utente
3)controllo se non e' stata soddisfatta la condizione fine gioco (un utente ha conquistato il numero di territori richiesti o e' scaduto il tempo)
*ritorna 1 nel caso il movimento non e' permesso 0 altrimenti
*ritorna -1 nel caso di errore della lettura di movimento
*/
int isBlocked(User localUser, int newX, int newY);
/*
*[Prerequisito]:la nuova casella non e' occupata da un'altro giocatore (isBlocked)
*controlla se la nuova casella non e' sotto proprieta' di un'altro utente
*ritorna 1 nel caso e' posseduto da un'altro utente 0 altrimenti
*ritorna -1 nel caso di errore della lettura di movimento
*/
int isOwnedByEnemy(User localUser, int newX, int newY);
/*
*ritorna 1 se il territorio su cui si vuole fare il passaggio e' gia' in possesso all'attuale utente
*ritorna 0 altrimenti
*ritorna -1 nel caso di errore della lettura di movimento
*/
int isMine(User localUser, int newX, int newY);
/*
*[Prerequisito]:deve esiste la disponibilita' per effettuare questo movimento(altro utente, limite mappa), funzione non esegue nessun controllo
*Eseguo movimento verso la nuova casella, nella vecchia posizione lascio tolower(localUser->letteraAssegnata),
*nella nuova posizione scrivo localUser->letteraAssegnata
*il numero di conquiste rimane invariato
*ritorna 1 nel caso in cui il procedimento e' andato a buon fine, 0 altrimenti
*/
int eseguiMovimentoSenzaConquista(User localUser, int newX, int newY);
/*
*Prerequisito]:deve esiste la disponibilita' per effettuare questo movimento(altro utente, limite mappa), funzione non esegue nessun controllo
*funzione richiama un'altra funzione eseguiMovimentoSenzaConquista(..) e aumenta il numero di conquiste del localUser
*NOTA BENE: se user ha raggiunto obiettivo, funzione impostera partitaInCorso = 0
*ritorna 1 nel caso in cui il procedimento e' andato a buon fine, 0 altrimenti
*/
int eseguiMovimentoConConquista(User localUser, int newX, int newY);
/*
*imita due lanci di dadi, uno per attacco un'altro per la difesa. Se attacco > diffesa funzione restituisce 1, 0 altrimenti
*/
int riuscitoConquistareTerritorio();
/*
*ritona il valore flag da 0 a 5
*/
int eseguiComando(User localUser, int newX, int newY);
/*
*Manda mappa, lista utenti, tempo rimamente e nel caso anche vincitore della partita a client
*ritorna 1 se ha ricevuto flag = 5
*/
int sendUpdateToUser(User localUser, int flag);

int sendTerritoriPosseduti(User localUser, int nTerritoriPosseduti);

int sendError(int sd_client,User* headUser, Game* headGame, User localUser);
int confermaWrite(int sd_client,User* headUser, Game* headGame, User localUser);
int confermaRead(int sd_client,User* headUser, Game* headGame, User localUser);
void cancellaUserDalServer(User* headUser, Game* headGame, User localUser);
int main(int argc, char *argv[])
{

    int sd_server, sd_client, porta_ingresso;
    struct sockaddr_in server_addr, client_addr;
    pthread_t tid;
    socklen_t client_len;
    char *ip_adress;

    if (argc != 2)
    {
        perror("argc"), exit(-1);
    }

    porta_ingresso = atoi(argv[1]);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(porta_ingresso);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //specifico adress che voglio accettare, in questo caso accetto un address qualsiasi

    sd_server = socket(PF_INET, SOCK_STREAM, 0);
    if (sd_server < 0)
    {
        perror("socket"), exit(-1);
    }
    bind(sd_server, (struct sockaddr *)&server_addr, sizeof(server_addr));

    listen(sd_server, 100);

    while (1)
    {
        client_len = sizeof(client_addr);
        if ((sd_client = accept(sd_server, (struct sockaddr *)&client_addr, &client_len)) < 0)
            perror("accept"), exit(-1);
     
        ip_adress = inet_ntoa(client_addr.sin_addr);

       
        log_write_message("Accesso:\n");
        log_write_access(NULL, ip_adress);
        
        write(STDOUT_FILENO, "Nuovo accesso",13);
        

        Client client = initNodeClient(ip_adress, sd_client);

        pthread_create(&tid, NULL, gestisci, (void *)client);
    }

    return 0;
}

void *gestisciTempo(void *val)
{
    Game tmp = val;

    pthread_mutex_lock(&mutex);
    while (tmp->partitaInCorso != 1)
    {
        pthread_cond_wait(&cond, &mutex);
    }
    printf("La partita id:%d e' iniziata\n", tmp->idGame);

    while (tmp->tempo_gioco > 0)
    {
        sleep(1);
        tmp->tempo_gioco--;
        if (tmp->partitaInCorso != 1)
        {
            break;
        }
    }

    if (tmp->tempo_gioco == 0)
    {
        tmp->partitaInCorso = 2;
        printf("Il tempo del gioco della parita id:%d e' terminato\n", tmp->idGame);
    }
    pthread_mutex_unlock(&mutex);
}

void gestioneSIGPIPE(int tid)
{

    printf("Uccido thread: %ld\n", pthread_self());
    pthread_cancel(pthread_self());
}

void *gestisci(void *sd)
{
    signal(SIGPIPE, gestioneSIGPIPE);

    int esito = 0 ; 
    Client localClient = sd;
    User localUser = NULL;
    char buf[1024];
    int choice_client;
    int n, condUsicta = 0;
    char userName[36], password[36];
    int nuovaOperazione = 0; // 0 se deve scegliere di nuovo utente, altrimenti il numero dell'operazione da effettuare
    // Mi sono appena connesso con client

    do
    {
        //Aspetto utente che sceglie operazione da eseguire
        if (nuovaOperazione == 0 && (n = read(localClient->sd_client, buf, 1)) <= 0)
            perror("read benvenuto"), pthread_exit(&err_th);
        choice_client = atoi(buf);
        switch (choice_client)
        {
        case 1:
            /****************** ...LOGIN... ************************/
            /* RICEVO USERNAME */
            if ((n = read(localClient->sd_client, userName, 512)) <= 0){
                perror("read username login");
                pthread_exit(&err_th);
            }

            if (strcmp(userName, "q") == 0)
            {
                nuovaOperazione = 0;
                break;
            }

            

            confermaWrite(localClient->sd_client, &head, &infGioco, localUser);

            /* RICEVO PASSWORD */
            if ((n = read(localClient->sd_client, password, 36)) <= 0)
                perror("read password login"), pthread_exit(&err_th);
            if (strcmp(password, "q") == 0)
            {
                nuovaOperazione = 0;
                break;
            }


            pthread_mutex_lock(&mutexListaUser);
            
            esito = isLogged(head, userName); 
       
            pthread_mutex_unlock(&mutexListaUser);
            

            if (checkCredenziali(userName, password) && !esito)
            {
                printf("Utente loggato:%s\n", userName);
                confermaWrite(localClient->sd_client, &head, &infGioco, localUser);
                nuovaOperazione = 0;

                //Login effettuato con successo, inizializzo il nuovo utente
                localUser = initNodeUser(userName, 0, 0, localClient->adress, 0, 0, localClient->sd_client);

                pthread_mutex_lock(&mutexListaUser);
                head = appendNodeUser(head, localUser);
                pthread_mutex_unlock(&mutexListaUser);

                /* Scrivo nel log file evento utente loggato */
                
                log_write_message("Accesso:\n");
                log_write_access(localUser->username, localUser->address);
                
                prePartita(localUser);
                //A questo punto utente entra nella partita, gli devo mandare la matrice con tutti i dati
            }
            else
            {
                if (checkCredenziali(userName, password)){// se mi trovo nel else e utente esiste nel db => utente gia' connesso
                   if((write(localClient->sd_client, msLogged, 2))<=0){
                       perror("write checkCredinziali");
                       cancellaUserDalServer(&head, &infGioco, localUser);
                   }
                } 
                else{
                    if((write(localClient->sd_client, error, 2))<=0){
                        perror("write checkCredinziali");
                        cancellaUserDalServer(&head, &infGioco, localUser);
                    }
                }
                    
                nuovaOperazione = 1; //Resto sulla schermata login e aspetto username
            }
            //pulisco le variabili
            memset(userName, 0, sizeof(userName));
            memset(password, 0, sizeof(password));
            break;
        case 2:
            /****************** ...REGISTRAZIONE... ************************/
           

            /* RICEVO USERNAME */
            if ((n = read(localClient->sd_client, userName, 36)) <= 0)
                perror("read username registrazione"), pthread_exit(&err_th);
            if (strcmp(userName, "q") == 0)
            {
                nuovaOperazione = 0;
                break;
            }
           

            // invio la conferma
            confermaWrite(localClient->sd_client, &head, &infGioco, localUser);

            /* RICEVO PASSWORD */
            if ((n = read(localClient->sd_client, password, 36)) <= 0)
                perror("read passowrd registrazione"), pthread_exit(&err_th);
            if (strcmp(password, "q") == 0)
            {
                nuovaOperazione = 0;
                break;
            };
           

            if (!userExist(userName))
            {
                printf("Utente:%s e' stato registrato\n", userName);

                //Utente non esiste nel db, lo registro e mando il segnale al client
                registraUser(userName, password);
                confermaWrite(localClient->sd_client, &head, &infGioco, localUser);
                nuovaOperazione = 0;

                /* Scrivo nel log file evento utente registrato */
                log_write_message("Utente registrato:\n");
                log_write_access(userName, localClient->adress);
                //Ritorno alla schermata di login
            }
            else
            {
                //Utente e' gia' registrato
                sendError(localClient->sd_client, &head, &infGioco, localUser);
                //write(localClient->sd_client, error, 2);
                nuovaOperazione = 2;
                //Resto sulla schermata login e non faccio niente
            }
            //pulisco le variabili
            memset(userName, 0, sizeof(userName));
            memset(password, 0, sizeof(password));
            break;
        case 3:
            /*** USITA DAL SERVER ***/
            printf("Chiudo connessione con il client:%s\n", localClient->adress);
            close(localClient->sd_client);

            free(localClient);

            condUsicta = 1;

            break;
        }
    } while (condUsicta != 1);

    return 0;
}

void prePartita(User localUser)
{
    Game tmp;
    int ret;
    pthread_t tid;
    do
    {   
        pthread_mutex_lock(&mutexListaGame);
        tmp = findGameDisponibile(infGioco);
        pthread_mutex_unlock(&mutexListaGame);

        if (tmp != NULL)
        {
            
            localUser->game = tmp;
        }
        else
        {
            localUser->game = initNodeGame(120, 20);

            pthread_mutex_lock(&mutexListaGame);           
            infGioco = appendNodeGame(infGioco, localUser->game);
            pthread_mutex_unlock(&mutexListaGame);

            pthread_create(&tid, NULL, gestisciTempo, (void *)localUser->game);
            
        }

        UserIG tmpUser = initNodeUserIG(localUser->username);
        
        pthread_mutex_lock(&mutexListaGame);                       
        localUser->game->utenti = appendNodeUserIG(localUser->game->utenti, tmpUser);
        pthread_mutex_unlock(&mutexListaGame);
        ret = nuovaPartita(localUser);
    } while (ret != 0);
}

int nuovaPartita(User localUser)
{
    int ret = 0;
    int size_buf = 2;
    char *buf = malloc(sizeof(char) * size_buf);
    char carr;
    int n, choice_client, flag = -1;
    if (inizializzaUtenteNelGioco(localUser) != 1)
    {
        pthread_exit(&err_th);
    }

    //si può' giocare
    do
    {
        int newX = localUser->x, newY = localUser->y;
        if ((n = read(localUser->sd_client, buf, 2)) <= 0)
        {
            perror("read scelta mossa");
            cancellaUserDalServer(&head, &infGioco, localUser);
        }
        carr = *buf;

        if (carr == 'q')
        {
            LogOut(localUser);
            free(buf);
            return 0;
        }
        else if (carr == 'w')
        {
            newX = localUser->x - 1;
            flag = eseguiComando(localUser, newX, newY);
        }
        else if (carr == 's')
        {
            newX = localUser->x + 1;
            flag = eseguiComando(localUser, newX, newY);
        }
        else if (carr == 'a')
        {
            newY = localUser->y - 1;
            flag = eseguiComando(localUser, newX, newY);
        }
        else if (carr == 'd')
        {
            newY = localUser->y + 1;
            flag = eseguiComando(localUser, newX, newY);
        }
        else
        {
            break;
        }

        //MANDO I DATI AGGIORNATI
        sendUpdateToUser(localUser, flag);

        if(flag == 5)
            return 1;

       

    } while (1);

    memset(buf, 0, size_buf);
    free(buf);

    return ret;
}

int inizializzaUtenteNelGioco(User localUser)
{
    char letteraUser;
    int posizioneInizialeCodificata, x, y, indice;
  
    startGameTimer(localUser->game, &mutex, &cond);

    indice = getIndiceDispForLetteraColore(localUser->game->arrayDisp);
    posizioneInizialeCodificata = getPosizoneInizialeCodificata(localUser->game->mappa);

    if (posizioneInizialeCodificata != -1 && indice != -1)
    {
       
        letteraUser = initColoreLetteraUtente(localUser);
        

        x = (posizioneInizialeCodificata / 20);
        y = (posizioneInizialeCodificata % 20);

        localUser->game->mappa[x][y] = letteraUser;
        localUser->game->count++;
        localUser->x = x;
        localUser->y = y;
        localUser->nTerritoriPosseduti = 1;
        incrementaNumTerritoriPosseduti(localUser->game->utenti, localUser->username);

        pthread_mutex_lock(&mutexListaGame);
        sendMappa(localUser, localUser->game->mappa);
        sendGiocatori(localUser, &(localUser->game->utenti));
        pthread_mutex_unlock(&mutexListaGame);
        confermaRead(localUser->sd_client, &head, &infGioco, localUser);
        sendTime(localUser, localUser->game->tempo_gioco);
        confermaRead(localUser->sd_client, &head, &infGioco, localUser);
        sendTerritoriDaConquistare(localUser, localUser->game->numeroDiTerritoriDaConquistare);
        
        return 1;
    }
    else
    {
        
        sendError(localUser->sd_client, &head, &infGioco, localUser);
        return -1;
    }
}

char initColoreLetteraUtente(User localUser)
{

    int indiceDisp, coloreUser, letti, scritti;
    char coloreUserChar[128], letteraUser[128], buf[128];

    //recupero primo indice disponibile dall'array di colori/lettere
    indiceDisp = getIndiceDispForLetteraColore(localUser->game->arrayDisp);

    // prendo i-esima lettera
    letteraUser[0] = localUser->game->arrayLettera[indiceDisp];
    letteraUser[1] = '\0';
    localUser->letteraAssegnata = localUser->game->arrayLettera[indiceDisp];

    //prendo i-esimo colore
    coloreUser = localUser->game->arrayColori[indiceDisp];
    localUser->coloreAssegnato = localUser->game->arrayColori[indiceDisp];

    sprintf(coloreUserChar, "%d", coloreUser);
    coloreUserChar[strlen(coloreUserChar)] = '\0';

    // rendo i-esimo colore/lettera non disponibile
    localUser->game->arrayDisp[indiceDisp] = 1;

    

 
    if((scritti=write(localUser->sd_client, letteraUser, 128)) <= 0){
        perror("write invia lettera");
        cancellaUserDalServer(&head, &infGioco, localUser);
    }
    confermaRead(localUser->sd_client, &head, &infGioco, localUser);
    
    if((write(localUser->sd_client, coloreUserChar, 128)) <= 0){
        perror("write invia colore");
        cancellaUserDalServer(&head, &infGioco, localUser);
    }
    confermaRead(localUser->sd_client, &head, &infGioco, localUser);

    //ritorna lettera del utente
    return letteraUser[0];
}


void LogOut(User localUser)
{

   

    rimozioneUtenteDallaMappa(localUser);

    //rendo disponibile nella matrice delle lettere e colori
    localUser->game->arrayDisp[(int)localUser->letteraAssegnata - 65] = 0;
    

    pthread_mutex_lock(&mutexListaGame);
    localUser->game->utenti = removeNodeUserIGbyUsername(localUser->game->utenti, localUser->username);
    
    localUser->game->count--;

    if (localUser->game->count == 0 && localUser->game->partitaInCorso == 2)
    {
        localUser->game->tempo_gioco = 0; // per terminare il thread che sta decrementando il tempo e tiene mutex
        infGioco = removeNodeGameById(infGioco, localUser->game->idGame);
    }
    pthread_mutex_unlock(&mutexListaGame);

    // controllo che si può' evitare, per adesso lo lascio
    log_write_logout(localUser->username, localUser->address);

    // rimuovo utente dalla lista
    pthread_mutex_lock(&mutexListaUser);
    head = removeUserByUsername(head, localUser->username);
    pthread_mutex_unlock(&mutexListaUser);
    

}
void esciDallaPartita(User localUser)
{
    
    // rimuovo utente dalla lista
    localUser->coloreAssegnato = 0;
    localUser->letteraAssegnata = 0;
    localUser->nTerritoriPosseduti = 0;
    localUser->x = 0;
    localUser->y = 0;
    
    pthread_mutex_lock(&mutexListaGame);
    localUser->game->count--;
    if (localUser->game->count == 0)
    { 
        infGioco = removeNodeGameById(infGioco, localUser->game->idGame);
    }
    localUser->game = NULL;
    pthread_mutex_unlock(&mutexListaGame);
}



int eseguiComando(User localUser, int newX, int newY)
{
    int flag;
    pthread_mutex_lock(&mutexListaUser);
    pthread_mutex_lock(&mutexListaGame);
        
    if (!(isBlocked(localUser, newX, newY)))
    {
        if (isOwnedByEnemy(localUser, newX, newY))
        { //
            if (riuscitoConquistareTerritorio())
            {
                flag = 2;
                char letteraGiocatoreInDifesa = localUser->game->mappa[newX][newY];
                char *usernameDifesa; // riferimento ad un stringa dinamica interna alla funzione
                usernameDifesa = getUsernameByLetteraNelGioco(head, letteraGiocatoreInDifesa, localUser->game->idGame);
                
                eseguiMovimentoConConquista(localUser, newX, newY);
                updateUserTerritoriByLettera(head, letteraGiocatoreInDifesa, -1);
                decrementaNumTerritoriPosseduti(localUser->game->utenti, usernameDifesa);

                log_write_conquista(localUser->username, newX, newY);
                free(usernameDifesa);
            }
            else
            {
                flag = 3; //resto sul posto
               
            }
        }
        else if (isMine(localUser, newX, newY))
        {
            flag = 4;
            eseguiMovimentoSenzaConquista(localUser, newX, newY);
        }
        else
        { // casella e' libera
            flag = 1;
            eseguiMovimentoConConquista(localUser, newX, newY);
            log_write_conquista(localUser->username, newX, newY);
        }
    }
    else
    {
        flag = 0;
    }

    if (localUser->game->partitaInCorso == 2)
        flag = 5;
    // close mutex
        
    pthread_mutex_unlock(&mutexListaGame);
    pthread_mutex_unlock(&mutexListaUser);

    return flag;
}

int sendUpdateToUser(User localUser, int flag)
{
    int size_flagBuf = 2;
    char *flagBuf = malloc(sizeof(char) * size_flagBuf);
  
    /* flag 0, movimento bloccato,
        flag 1 movimento sul nuovo territorio +1 conq, 
        flag 2 movimento conquista del territorio nemico
        flag 3 movimento sul territorio nemico senza la conquista, rimani sullo stesso posto
        flag 4 movimento sul proprio territorio
        flag 5 e' finito il gioco invio la notifica al utente con i risultati
    */

    sprintf(flagBuf, "%d", flag);

    if((write(localUser->sd_client, flagBuf, size_flagBuf))<=0){
        perror("write flag");
        cancellaUserDalServer(&head, &infGioco, localUser);
    }

    

    confermaRead(localUser->sd_client, &head, &infGioco, localUser);
    
    pthread_mutex_lock(&mutexListaGame);
    sendMappa(localUser, localUser->game->mappa);
    confermaRead(localUser->sd_client, &head, &infGioco, localUser);
    sendGiocatori(localUser, &(localUser->game->utenti));
    confermaRead(localUser->sd_client, &head, &infGioco, localUser);
    sendTime(localUser, localUser->game->tempo_gioco);
    pthread_mutex_unlock(&mutexListaGame);

    confermaRead(localUser->sd_client, &head, &infGioco, localUser);

    pthread_mutex_lock(&mutexListaUser);
    sendTerritoriPosseduti(localUser, localUser->nTerritoriPosseduti);
    pthread_mutex_unlock(&mutexListaUser);

    if (flag == 5)
    {
        confermaRead(localUser->sd_client, &head, &infGioco, localUser);
        esciDallaPartita(localUser);
    }
    free(flagBuf); 
}


int sendError(int sd_client, User* headUser, Game* headGame, User localUser)
{
    if (write(sd_client, "0\0", 2) <= 0)
    { 
        perror("Errore invio errore");
        cancellaUserDalServer(headUser, headGame, localUser);
        return -1;
    }
    return 1;
}


int confermaWrite(int sd_client,  User* headUser, Game* headGame, User localUser)
{
    if (write(sd_client, "1\0", 2) <= 0)
    { 
        perror("Errore invio conferma");
        cancellaUserDalServer(headUser, headGame, localUser);
        return -1;
    }
    return 1;
}


int confermaRead(int sd_client,  User* headUser, Game* headGame, User localUser)
{
    char *buffer = malloc(sizeof(char) * 32);
    if (read(sd_client, buffer, 2) <= 0)
    { 
        perror("Errore ricezione conferma");
        free(buffer);
        cancellaUserDalServer(headUser, headGame, localUser);
        return -1;
    }
    return 1;
    free(buffer);
}


int sendMappa(User localUser, char mappa[20][20])
{
    char buf[800];
    int k = 0;
    for (int i = 0; i < 20; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            buf[k++] = mappa[i][j];
        }
    }
    buf[k] = '\0';
    if((write(localUser->sd_client, buf, strlen(buf)))<=0){
        perror("write sendMappa");

        cancellaUserDalServer(&head, &infGioco, localUser);
    }
    return 1;
}

int sendGiocatori(User localUser, UserIG *listaUtentiConnessi)
{
    char *buffer;
    //  buffer = getConnectedUserInString(head);
    // N.B qualsiasi cosa passo e' una copia di riferimento
    MergeSort((listaUtentiConnessi));
    buffer = getConnectedUsersInGame(*listaUtentiConnessi);



    // INVIO LISTA DI GIOCATORI AL CLIENT
    if((write(localUser->sd_client, buffer, strlen(buffer) + 1))<=0){
        perror("write sendGiocatori");
        cancellaUserDalServer(&head, &infGioco, localUser);
    }
    free(buffer);
}

int sendTime(User localUser, unsigned int tempo_gioco)
{
    int tempo_rimanente = tempo_gioco;
    int minuti = 0, secondi = 0;
    char *tmpMin = malloc(sizeof(char) * 4);
    char *tmpSec = malloc(sizeof(char) * 3);
    char *tempoString = malloc(sizeof(char) * 32);
    while (tempo_rimanente != 0)
    {
        if (tempo_rimanente >= 60)
        {
            minuti++;
            tempo_rimanente -= 60;
        }
        else
        {
            secondi = tempo_rimanente;
            tempo_rimanente = 0;
        }
    }

    sprintf(tmpMin, "%d", minuti);
    sprintf(tmpSec, "%d", secondi);
    tempoString = concatenation(tmpMin, concatenation("m:", (concatenation(tmpSec, "s"))));

    // INVIO IL TEMPO  "Xm:Ys" al client

    if((write(localUser->sd_client, tempoString, 32))<=0){
        perror("write sendTime");
        cancellaUserDalServer(&head, &infGioco, localUser);
    }
}

int sendTerritoriDaConquistare(User localUser, int numeroDiTerritoriDaConquistare)
{
    int size_tmpNumeroTerritoriDaConquistare = 4;
    char *tmpNumeroTerritoriDaConquistare = malloc(sizeof(char) * size_tmpNumeroTerritoriDaConquistare);
    sprintf(tmpNumeroTerritoriDaConquistare, "%d", numeroDiTerritoriDaConquistare);

    // INVIO NUMERO DI TERRITORI DA CONQUISTARE
    if((write(localUser->sd_client, tmpNumeroTerritoriDaConquistare, size_tmpNumeroTerritoriDaConquistare))<=0){
        perror("write sendTerritoriDaConquistare");
        cancellaUserDalServer(&head, &infGioco, localUser);
    }
}

int sendTerritoriPosseduti(User localUser, int nTerritoriPosseduti)
{
    char *localBuf = malloc(sizeof(char) * 32);
    sprintf(localBuf, "%d", nTerritoriPosseduti);

    if (write(localUser->sd_client, localBuf, 32) <= 0)
    {
        perror("write sendTerritoriPosseduti");
        free(localBuf);
        cancellaUserDalServer(&head, &infGioco, localUser);
        return -1;
    }

    free(localBuf);

    return 1;
}

int isBlocked(User localUser, int newX, int newY)
{
    if (newX > 19 || newY > 19 || newX < 0 || newY < 0)
    {
        return 1;
    }
    // se il valore della nuova casella e' una lettera maiuscola allora e' un'altro giocatore
    // non c'e' il bisogno di discriminare attuale utente perché utente può' trovarsi solo in un posto allo stesso tempo
    // se utente attuale ha fatto un passo e' sicuro che non trovera' se stesso.
    if (65 <= localUser->game->mappa[newX][newY] && localUser->game->mappa[newX][newY] <= 90)
    { 
        return 1;
    }
    // controllo se non e' stata soddisfata la condizione del fine gioco
    // adesso le condizioni di uscita sono: 0 partita non init, 1 partita in corso, 2 partita terminata
    if (localUser->game->partitaInCorso != 1)
    {
        
        // dovrei mandare un messaggio al client dicendo che e' finita la partita'
        return 1;
    }

    //se nessuno dei primi if e' stato soddisfatto => ho la posibilita' di fare il movimento
    return 0;
}
int isOwnedByEnemy(User localUser, int newX, int newY)
{

    if (97 <= localUser->game->mappa[newX][newY] && localUser->game->mappa[newX][newY] <= 122 && localUser->game->mappa[newX][newY] != tolower(localUser->letteraAssegnata))
    {
        return 1;
    }

    // se if non e' verificato allora il territorio non e' posseduto da altri utenti
    // puo' essere libero o appartenere all'attuale utente
    return 0;
}

int isMine(User localUser, int newX, int newY)
{

    if (97 <= localUser->game->mappa[newX][newY] && localUser->game->mappa[newX][newY] <= 122 && localUser->game->mappa[newX][newY] == tolower(localUser->letteraAssegnata))
    {
        return 1;
    }

    // il territorio su quale voglio fare il passaggio non e' di mia proprieta'
    return 0;
}


int eseguiMovimentoSenzaConquista(User localUser, int newX, int newY)
{
    localUser->game->mappa[newX][newY] = localUser->letteraAssegnata;
    localUser->game->mappa[localUser->x][localUser->y] = tolower(localUser->letteraAssegnata);
    localUser->x = newX;
    localUser->y = newY;
}


int eseguiMovimentoConConquista(User localUser, int newX, int newY)
{
    eseguiMovimentoSenzaConquista(localUser, newX, newY);
    incrementaNumTerritoriPosseduti(localUser->game->utenti, localUser->username);
    localUser->nTerritoriPosseduti++;
    //   printAllUserIG(localUser->game->utenti);
    if (localUser->nTerritoriPosseduti >= localUser->game->numeroDiTerritoriDaConquistare)
    {
        localUser->game->partitaInCorso = 2;
        strcpy(localUser->game->vincitore, localUser->username);
        //localUser->game->vincitore = localUser->username;
    }
}

int riuscitoConquistareTerritorio()
{
    srand(time(NULL));
    int attacco = 1 + rand() % 6;
    int difesa = 1 + rand() % 6;

    if (attacco > difesa)
        return 1;
    else
        return 0;
}


void cancellaUserDalServer(User* headUser, Game* headGame, User localUser){
    
    User tmpUser = *headUser;
    Game tmpGame = *headGame;
    int err_th = -1 ; 
    if(localUser == NULL){
       
        pthread_mutex_unlock(&mutexListaGame);
        pthread_mutex_unlock(&mutexListaUser);
        pthread_exit(&err_th);
    }
    if(localUser->username == NULL){
      
        pthread_mutex_unlock(&mutexListaGame);
        pthread_mutex_unlock(&mutexListaUser);
        pthread_exit(&err_th);
    }   
    
    if(localUser->game == NULL){
        pthread_mutex_unlock(&mutexListaGame);
        pthread_mutex_unlock(&mutexListaUser);
        pthread_exit(&err_th);
    }
    rimozioneUtenteDallaMappa(localUser);
    localUser->game->arrayDisp[(int)localUser->letteraAssegnata - 65] = 0;

    localUser->game->utenti = removeNodeUserIGbyUsername(localUser->game->utenti, localUser->username);
    localUser->game->count--;
    if(localUser->game->count == 0 && localUser->game->partitaInCorso == 2){
        infGioco = removeNodeGameById(infGioco,localUser->game->idGame);
        printAllGameInfo(infGioco);
    }
    close(localUser->sd_client);
    head = removeUserByUsername(head,localUser->username);
    pthread_mutex_unlock(&mutexListaGame);
    pthread_mutex_unlock(&mutexListaUser);
    pthread_exit(&err_th);
}
