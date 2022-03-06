#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "checkInput.h"


//FUNZIONI LOGIN-REGISTRAZIONE-BENVENUTO

int gestioneBenvenuto(int , int );
int gestioneUsernamePassowrd(int );

//FUNZIONI PARTITA 

int startGame(int);
int update(int , char [20][20] , char [26][100] , char* , int*);
int recuperaScelta(char* );

//FUNZIONI INIZIALIZZAZIONI VARIABILI

int initGame(int,char[20][20] , int*, char* , char [26][100] , char*, int*);
int initMappa(int , char[20][20]);
int initTimer(int , char*);
int initListUsers(int , char[26][100]);
int initUserColor(int , int*);
int initUserChar(int, char*);
int initTerritoriNecessari(int , int*);
void initColoriLettere();
int initTerritoriPosseduti(int , int* );

//FUNZIONI DI STAMPA

void stampaVincitore(char [26][100]);
void stampaFraseFinale(int);
void stampaMappa(char [20][20],int,char,char[26][100]);
void stampaLegenda(int , char );
void stampaStatistichePartita(int , char [],int,char*);
void stampaScelte();

//UTILITY

int conferma(int);
int coloraCarattere(int, char);
char sostituisciCasella(int , int , char[20][20], char);

//VAR GLOBALI

int arrayColori[26];
char arrayCaratteri[26];
char usernameGlobale [30];

//Per compilare... 
//gcc -o client client.c checkInput.h checkInput.c lib/inputReader.h lib/inputReader.c
//./client 79.42.150.62 18000
int main(int argc, char *argv[]){

    int sd_client = 0, n = 0, m = 0, retBenvenuto = 0 , retGame = 0;
    int scelta; 
    char recvBuff[1024] ; 
    
    struct sockaddr_in server_addr;
    char *ip_adress;
    int porta_ingresso;
    
    
    char msgBenvenuto[] = "Scegliere operazione:\n1.Login\n2.Registrazione\n3.Per chiudere la connessione\n\0";
    ssize_t lenBenvenuto = strlen(msgBenvenuto); 
       

    if (argc != 3){
        write(STDERR_FILENO, "Numero insufficiente di parametri\n", 34);
        exit(-1);
    }

    signal(SIGINT,SIG_IGN);
   
    ip_adress = argv[1]; //INIZIALIZZO L'IP A CUI CONNETTERMI
    porta_ingresso = atoi(argv[2]); //INIZIALIZZO LA PORTA A CUI CONNETTERMI 
    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htons(porta_ingresso);
    inet_aton(ip_adress, &server_addr.sin_addr);

    sd_client = socket(AF_INET, SOCK_STREAM, 0);
    if (sd_client < 0)
        perror("socket"), exit(-1);

    if (connect(sd_client, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("connect"), exit(-1);
    }
    system("clear");
    do{
        retGame = 0 ; 
        retBenvenuto = 0; 
        //STAMPO SU CONSOLE IL MESSAGIO DI BENVENUTO
        if ((n = write(STDOUT_FILENO, msgBenvenuto, lenBenvenuto)) < 0){
            perror("write benvenuto");
            return -1;
        }

        //LEGGO DA STDIN ED INVIO LA PRIMA RISPOSTA AL SERVER
        /**
             * 1 : LOGIN 
             * 2 : REGISTRAZIONE 
             * 3 : CHIUDERE PROGRAMMA 
        */

        //LEGGO LA PRIMA RISPOSTA
        scelta = doSceltaIntError("-> ", 3, "Errore scelta argomenti riprovare...\n");
        
        //CONVERTO LA PRIMA RISPOSTA IN UNA STRINGA
        sprintf(recvBuff, "%d", scelta);
        
        //INVIO LA PRIMA RISPOSTA AL SERVER
        if ((n = write(sd_client, recvBuff, 1)) < 0){
            perror("write prima rispostaServer");
            return -1;
        }

        retBenvenuto = gestioneBenvenuto(scelta, sd_client);
    
        if(retBenvenuto == -1){
            close(sd_client);
            return -1 ;
        }

        if(retBenvenuto == 1){//LOGIN OK  
            retGame = startGame(sd_client);            
            
        }

        if(retGame == -1){
            close(sd_client);
            return -1 ;
        }

            
    } while (retGame == 2 || retBenvenuto == 2);//TORNO ALLA SCHERMATA DI BENVENUTO

    

    close(sd_client);
    return 0;
}
/****************** FUNZIONI LOGIN-REGISTRAZIONI-BENVENUTO ********************/

/**SI OCCUPA DELLA GESTIONE DEL BENVENUTO DOPO CHE L'UTENTE HA EFFETTUATO UNA SCELTA CON IL
 * PARAMETRO SCELTA DOVE 1 INDICA IL LOGIN 2 INDICA LA REGISTRAZIONE E 3 INDICA LA CHIUSURA
 * DEL PROGRAMMA. LA FUNZIONE RITORNA 1 SE IL LOGIN E' ANDATO A BUON FINE, 2 NEL CASO SI 
 * VOGLIA RIPETERE LA SCHERMATA DI BENVENUTO, 3 SE SI VUOLE CHIUDERE IL PROGRAMMA E -1 IN CASO 
 * DI ERRORE
*/
int gestioneBenvenuto(int scelta, int sd_client){
    int esito = 0;
    
    
    switch (scelta){

    case 1: //LOGIN
        fprintf(stdout,"\n('q' per tornare alla schermata di benvenuto)\n\n");
        fprintf(stdout,"****LOGIN****\n");
            
        do{
            esito = gestioneUsernamePassowrd(sd_client);

            switch (esito){
            case 0:
                fprintf(stdout, "Errore Nome utente o Password non corretti\n\n");//RIPETO IL LOGIN
                break;
            case 1 : 
                fprintf(stdout, "Login OK\n\n");
                return 1;
                
            case 2 : 
                return 2; //TORNO ALLA SCHERMATA DI BENVENUTO

            case 3 :
                fprintf(stdout, "Utente già loggato :)\n\n");//RIPETO IL LOGIN            
                break ;   
            default:
                fprintf(stdout, "Errore Login rispostaServer : %d\n",esito);//ESCO
                return -1;
            }
                 
        } while (esito == 0 || esito == 3);
        
        break;

    case 2: //REGISTRAZIONE
        fprintf(stdout,"\n('q' per tornare alla schermata di benvenuto)\n\n");
        fprintf(stdout,"****REGISTRAZIONE****\n");
        do{
            esito = gestioneUsernamePassowrd(sd_client);

            switch (esito){
            case 0:
                fprintf(stdout, "Errore nome utente già registrato\n\n");//RIPETO LA REGISTRAZIONE
                break;
            case 1 : 
                system("clear");
                fprintf(stdout, "Registrazione OK\n\n");
                return 2; //TORNO ALLA SCHERMATA DI BENVENUTO
            
            case 2 : 
                return 2; //TORNO ALLA SCHERMATA DI BENVENTUO
            
            default:
                fprintf(stdout, "Errore registrazione rispostaServer : %d\n",esito);//ESCO
                return -1;
            }
                
        } while (esito == 0);
    break;

    case 3 : //CHIUSURA PROGRAMMA
        return 3;

    default : 
        fprintf(stdout,"Terminazione di default\n");
        return -1 ;
    }

    return 1;
}
/**
 * GESTIONE DEL RECUPERO DATI NEL CASO DI UN LOGIN O UNA REGISTRAZIONE
 * USERNAME MINIMO 4 CARATTERI MENTRE LA PASSWORD 6 CARATTERI 
 * RITORNA 
 * -1 IN CASO DI ERRORE 
 * 2 PER TORNARE ALLA SCHERMATA DI BENVENUTO (CON 'q' SI PUO TORNARE ALLA SCHERMATA DI BENVENUTO)
 * 0 PER DATI ERRATI
 * OPPURE UNA RISPOSTA DAL SERVER
*/
int gestioneUsernamePassowrd(int sd_client){

    int n = 0;
    int m = 0;
    char username[50];
    char password[50];
    char quit[] = "q";
    char localBuf[256];
    int rispostaServer = 0;
     
        
    //CONTROLLO DELL'USERNAME DEV'ESSERE 4 CARATTERI
    do{
        write(STDOUT_FILENO, "Username : ", 11);
        if ((n = read(STDIN_FILENO, username, 36)) < 0){
            perror("read username");
            return -1;
        }
        username[n-1]='\0';//SOSTITUISCO LO \N CON \0
        if(n < 5 || strcmp(quit,username) == 0){
            if(strcmp(quit,username) == 0){        
                write(sd_client,"q\0",2); //INVIO AL SERVER LA Q PREMUTA
                system("clear"); //PULISCO LA SCHERMATA
                printf("Rilevato 'q' , torno al benvenuto\n\n");
                return 2; //TORNO ALLA SCHERMATA DI BENVENUTO
            }
            else 
                fprintf(stdout,"Errore nome utente troppo corto... \n");
                
        }    
    }while (n<5);
    
    //CONTROLLO DELLA PASSWORD DEV'ESSERE 6 CARATTERI
    do{
        write(STDOUT_FILENO, "Password : ", 11);
        if ((m = read(STDIN_FILENO, password, 36)) < 0){
            perror("read password");
            return -1;
        }
        password[m-1] = '\0'; //SOSTITUISCO LO \N CON \0
        if(m < 7 || strcmp(quit,password) == 0){
            if(strcmp(quit,password) == 0){
                write(sd_client,"q\0",2); //INVIO AL SERVER LA Q PREMUTA
                system("clear"); //PULISCO LA SCHERMATA
                printf("Rilevato 'q' , torno al benvenuto\n\n");
 
                return 2; //TORNO ALLA SCHERMATA DI BENVENUTO
            }
            else 
                fprintf(stdout,"Errore password troppo corta... \n");
        }
    
    }while (m < 7);
    
    //INVIO USERNAME AL SERVER
    if (write(sd_client, username, n) < 0){
        perror("write username");
        return -1;
    }

    strcpy(usernameGlobale,username);
    
    //LEGGO LA CONFERMA DI RICEZIONE DEL NOME UTENTE DA PARTE DEL SERVER
    if (read(sd_client, localBuf, 128) < 0){
        perror("Read rispostaServer al login/registrazione username ricevuto");
        return -1;
    }

    
    //INVIO LA PASSWORD AL SERVER
    if (write(sd_client, password, m) < 0){
        perror("write password");
        return -1;
    }

    //LEGGO LA RISPOSTA DEL SERVER DOPO L'INVIO DEI DATI
    /**
     *  0 : UTENTE GIÀ REGISTRATO OPPURE DATI INCORRETTI 
     *  1 : LOGIN O REGISTRAZIONI ANDATE A BUON FINE 
     * -1 : ERRORE DI COMUNICAZIONE
     *  3 : UTENTE GIÀ LOGGATO 
    */

    //LEGGO LA RISPOSTA 
    if ((n = read(sd_client, localBuf, 128)) < 0){
        perror("Read rispostaServer al login/registrazione");
        return -1;
    }

    rispostaServer = atoi(localBuf);
    
    
    return rispostaServer;
}


/***************************  FUNZIONI PARTITA  ************************/

/**RITORNA
 * 2 PER INDICARE CHE SI VUOLE TORNARE ALLA SCHERMATA DI BENVENUTO
 * -1 PER UN ERRORE
 * 1 TERMINAZIONE OK MA NON PREVISTA
*/
int startGame(int sd_client){
    char * letteraUtente = malloc(sizeof(char));        //LETTERA UTENTE
    int * coloreUtente = malloc(sizeof(int));           //COLORE UTENTE
    char mappa[20][20];                                 //MAPPA DI GIOCO
    char * scelta = malloc(sizeof(char));               //MOVIMENTO SCELTO DALL' UTENTE
    int *territoriPosseduti = malloc(sizeof(int));      //N TERRITORI POSSEDUTI
    int *territoriDaConquistare = malloc(sizeof(int));  //N TERRITORI PER VINCERE
    int rispostaServer = 0 ;                            //FLAG RISPOSTA SERVER
    int retInitGame = 0 , retRecuperoScelta = 0 ;        //VAR DI CONTROLLO LOCALE
    char buf[128];                                      //BUFFER LOCALE
    char userUtenti [26][100];                          //LISTA UTENTI IN GIOCO
    char * time = malloc(sizeof(char) * 128);           //TEMPO DI GIOCO RIMANENTE
    char * conseguenzaUltimaAzione = malloc(sizeof(char) * 128);   //DESCRIZIONE DELLE CONSEGUENZE LEGATE ALL'ULTIMA AZIONE

    initColoriLettere();                                //INIZIALIZZO COLORI E LETTERE DEGLI AVVERSARI
    
    system("clear");
    
    do{
        *territoriPosseduti = 1 ;  //AD INIZIO PARTITA HAI 1 TERRITORIO
        retInitGame = initGame(sd_client,mappa,coloreUtente,letteraUtente,userUtenti,time,territoriDaConquistare);
        strcpy(conseguenzaUltimaAzione,"Iniziamo a giocare..");
        if(retInitGame<0){
            fprintf(stdout,"Errore initGame\n");
            return -1;
        }
        else if(retInitGame == 2){//IMPOSSIBILE ACCEDERE.. FINITE LE LETTERE DISPONIBILI
            return 2 ; //TORNO ALLA SCHERMATA DI BENVENUTO
        }
        
        else if(retInitGame == 1){//INIZIALIZZAZIONE OK 
            do{
                //SCHERMATA DI GIOCO
                stampaLegenda(*coloreUtente,*letteraUtente);
                stampaMappa(mappa,*coloreUtente,*letteraUtente,userUtenti); 
                stampaStatistichePartita(*territoriPosseduti,time,*territoriDaConquistare,conseguenzaUltimaAzione);
                stampaScelte();
                retRecuperoScelta = recuperaScelta(scelta); //PRENDO LA SCELTA DELL'UTENTE E LA SALVO IN SCELTA

                switch (retRecuperoScelta){
                    case 1://LETTERRA INSERITA CORRETTAMENTE
                        if(strcmp(scelta,"q") == 0){//RILEVATO Q FACCIO LOGOUT
                            if(write(sd_client,scelta,2)<=0){
                                perror("Errore write scelta 'q'");
                                return -1;
                            }
                            printf("Rilevato q\n\n");
                            system("clear");
                            return 2; //TORNO ALLA SCHERMATA DI BENVENUTO
                        }
                        else{
                            //INVIA AL SERVER LA RISPOSTA E AGGIORNA LA POSIZIONE
                            if(write(sd_client,scelta,2)<=0){//INVIO IL MOVIMENTO
                                perror("Errore write movimento");
                                return -1;                        
                            }
                            if(read(sd_client,buf,128)<=0){//LEGGO IL FLAG DEL SERVER
                                perror("Errore read rispostaServer al movimento");
                                return -1;
                            }
                            conferma(sd_client);

                            rispostaServer = atoi(buf);
                            system("clear");
                            update(sd_client,mappa,userUtenti,time,territoriPosseduti);
                                
                            switch (rispostaServer){
                                case 0: //movimento bloccato perchè fuori mappa o altro utente già occupa
                                    strcpy(conseguenzaUltimaAzione, "Movimento bloccato, out of range o scontro con altro player");
                                    break;

                                case 1: //movimento accettato su territorio di nessuno, posseduti +1
                                    strcpy(conseguenzaUltimaAzione, "Complimenti.. Hai un conquistato un territorio libero !!");

                                    break;

                                case 2: //movimento sul territorio nemico e vinto la conquista
                                    strcpy(conseguenzaUltimaAzione, "Complimenti.. Hai vinto la sfida e conquistato un territorio nemico !!");
                                    break;

                                case 3: //tentativo di conquista senza successo, rimani sul posto
                                    strcpy(conseguenzaUltimaAzione, "Peccato.. Hai perso la sfida per la conquistata :(");
                                    break;

                                case 4: //movimento sul proprio territorio
                                    strcpy(conseguenzaUltimaAzione, "Movimento sul proprio territorio");
                                    break;

                                case 5: //fine partita per tempo o arrivo ai territori necessari
                                    //conferma(sd_client);
                                    //read(sd_client,buf,128);//leggo  il vincitore
                                    fprintf(stdout, "PARTITA TERMINATA%93s%s\n\n", "", "CLASSIFICA FINALE");
                                    stampaMappa(mappa, *coloreUtente, *letteraUtente, userUtenti);
                                    stampaVincitore(userUtenti);
                                    stampaFraseFinale(sd_client);
                                    system("clear");

                                    retRecuperoScelta = 2;//RICOMINCIO UNA NUOVA PARTITA
                                    break;

                                default:
                                    fprintf(stdout, "Errore, Risposta del server sconosciuta : %d\n", rispostaServer);
                                    return -1;
                                    break;
                            }//FINE SWITCH RISPOSTA SERVER
                        }//FINE ELSE CASE 1 
                    break;//FINE CASE 1
                    
                    case 0: //LETTERA SBAGLIATA RIPETO LA SCELTA    
                        strcpy(conseguenzaUltimaAzione,"Mi sembra una mossa illegale... RIPROVA");
                        system("clear");
                    break;

                    case -1: //ESCO PER QUALCHE ERRORE 
                        return -1;                    
                    break;
                    
                    default:
                        printf("Errore recupero scelta, ret sconosciuto : %d\n",retInitGame);
                        return -1;
                    break;
                }    
                
            }while (retRecuperoScelta == 0 || retRecuperoScelta == 1);//RIPETO LA MOSSA
        }    
    } while (retRecuperoScelta == 2);//RICOMINCIO LA PARTITA
    
    free(letteraUtente);free(coloreUtente);free(scelta);
    free(territoriPosseduti);free(territoriDaConquistare);
    free(time);free(conseguenzaUltimaAzione);

    return 1 ;  
}

//LEGGE DAL SERVER E AGGIORNA LA MAPPA DI GIOCO, LA LISTA UTENTI CONNESSI E IL TEMPO DI GIOCO 
/** RESTITUISCE 
 *  1 TUTTO OK 
 *  -1 ERRORE
*/
int update(int sd_client, char map[20][20] , char listaUser[26][100] , char* time , int* territoriPosseduti){
char localbuf[1024];
    
    if(initMappa(sd_client,map) == -1)
        return -1;

    if(conferma(sd_client) == -1)
        return -1;

    if(initListUsers(sd_client,listaUser) == -1)
        return -1;

    if(conferma(sd_client) == -1)
        return -1;

    if(initTimer(sd_client,time) == -1)
        return -1;

    if(conferma(sd_client) == -1)
        return -1;

    if(initTerritoriPosseduti(sd_client,territoriPosseduti) == -1)
        return -1;


return 1 ; 
}

//LEGGE DA STDIN IL MOVIMENTO DELL'UTENTE E RESTITUSCE 
/** 0 SE HA INSERITO UN CARATTERE SBAGLIATO
 *  1 SE HA INSERITO UN CARATTERE VALIDO 
 *  -1 PER UN ERRORE
 * */
int recuperaScelta(char* carattere){
    int n = 0 ; 
    
    n = read(STDIN_FILENO,carattere,128);
    *(carattere+n-1) = '\0';
     
    
    if ( n <  0){
        fprintf(stdout,"Errore lettura rispostaServer\n");
        return -1; //ERRORE
    }

    if (strlen(carattere) > 1 || *carattere != 'w' && *carattere != 'a' && *carattere != 's' && *carattere != 'd' && *carattere!= 'q' ){
        printf("Errore carattere\n");
        return 0; //CARATTERE SBAGLIATO
    }
    
return 1 ; //TUTTO OK
}




/*************************  INIZIALIZZAZIONI  ****************************/

/**
 * INIZIALIZZA LE VARIABILI PER INIZIARE UNA NUOVA PARTITA
 * RESTITUSCE 
 * 2 PER INDICARE CHE SONO FINITE LE LETTERE E NON È POSSIBILE GIOCARE
 * 1 TUTTO OK
 * -1 ERRORE
*/
int initGame(int sd_client,char map[20][20] , int* colore , char* lettera , char listaUser[26][100], char* time , int* terreDaConquistare){
    
    
    if(initUserChar(sd_client,lettera) == -1)
        return -1;

    if(strcmp(lettera,"0") == 0){
        system("clear");
        printf("Lettere momentaneamente non disponibili per giocare... Riprovare tra qualche minuto\n\n");
        return 2 ; //TORNO ALLA SCHERMATA DI BENVENUTO
    }
   
    if(conferma(sd_client) == -1)
        return -1;

    if(initUserColor(sd_client,colore) == -1)
        return -1;
    
    if(conferma(sd_client) == -1)
        return -1;

    if(initMappa(sd_client,map) == -1)
        return -1;

    if(initListUsers(sd_client,listaUser) == -1)
        return -1;
    
    if(conferma(sd_client) == -1)
        return -1;
    
    if(initTimer(sd_client,time) == -1)
        return -1;
    
    if(conferma(sd_client) == -1)
        return -1;

    if(initTerritoriNecessari(sd_client,terreDaConquistare) == -1)
        return -1;
    
    
    return 1;
}


/*INIZIALIZZA L'ARRAY DEI CARATTERI E DEI COLORI PER STAMPA GLI AVVERSARI COLORATI*/
void initColoriLettere()
{
    int j= 21;
    for (int i = 0; i < 26; i++)
    {
        arrayCaratteri[i] = i + 65;
        
        if (i < 6)
            arrayColori[i] = i+9;
        else{
            arrayColori[i] = j;
            j += 10;
        }
             
    }

}

/*INIZIALIZZO LA MAPPA DI GIOCO*/
int initMappa(int sd_client,  char mappa[20][20]){
    
    char* localbuf = malloc(sizeof(char)*512);
    int i = 0 , j = 0 ; 

    if(read(sd_client,localbuf,400)<=0){//LEGGO LA MAPPA DI GIOCO
            perror("read mappa");
            return -1;
    }
    for(int count = 0 ; count < 400 ; count++){//SALVO LA MAPPA DI GIOCO 
        mappa[i][j] = localbuf[count] ;  
        j++;
        if(j==20){
            i++;
            j=0;
        }

    }

    free(localbuf);
    return 1;
}

/*INIZIALIZZO IL TIMER DI GIOCO*/
int initTimer(int sd_client, char *timer){
    
    char* localbuf = malloc(sizeof(char)*512);
    

    if(read(sd_client,localbuf,512)<=0){//LEGGO IL TIMER DI GIOCO
        perror("read timer");
        return -1;
    }
    
    strcpy(timer,localbuf);
    
    free(localbuf);
    return 1 ; 
}

/*INIZIALIZZO LA LISTA DEGLI UTENTI IN GIOCO*/
int initListUsers(int sd_client, char listaUser[26][100]){
     
    char* localbuf = malloc(sizeof(char)*1024);
    
    int i = 0 , j = 0 , t = 0 ;   

    if(read(sd_client,localbuf,1024)<=0){//LEGGO LA LISTA DI UTENTI IN GIOCO
        perror("read lista utenti");
        return -1;
    }

        while(localbuf[i] != '\0' ){
            if(localbuf[i] != '-'){
                listaUser[j][t] = localbuf[i];//COPIO IL CARATTERE
                i++;
                t++;
            }
            else if(localbuf[i] == '-'){        
                i++; //SALTO IL TRATTINO
                listaUser[j][t] = '\0'; //CHIUDO UNA RIGA CON \0
                j++; //AVANZO DI UNA RIGA
                t = 0 ; 
            } 
            
        }  
        
        if(localbuf[i] == '\0'){
            listaUser[j][t] = '\0';
        }

        while(++j<26){
              
            listaUser[j][0] = '\0';
        
        }
    
    free(localbuf);
    return 1 ; 
}

/*INIZIALIZZO IL COLORE DELL'UTENTE*/
int initUserColor(int sd_client, int* colore){

    char* localbuf = malloc(sizeof(char)*512);
    int n = 0 ; 

    if((n = read(sd_client,localbuf,512))<=0){//LEGGO IL COLORE CHE MI È STATO ASSEGNATO
        perror("Errore lettura colore assegnato");
        return -1;    
    }
    
    *colore = atoi(localbuf);

    free(localbuf);
    return 1;   
}

/*INIZIALIZZO LA LETTERA DELL'UTENTE*/
int initUserChar(int sd_client, char* carattere){

    char* localbuf = malloc(sizeof(char)*512);
    
    if(read(sd_client,localbuf,512)<=0){//LEGGO LA LETTERA CHE MI È STATA ASSEGNATA
        perror("Errore lettura lettera assegnata");
        return -1;    
    }
    
    *carattere = localbuf[0];
    *(carattere + 1) = '\0'; 
    
    free(localbuf);
    return 1 ; 
}

/*INIZIALIZZO IL NUMERO DI TERRITORI NECESSARI PER VINCERE*/
int initTerritoriNecessari(int sd_client, int* territoriNecessari){

    char* localbuf = malloc(sizeof(char)*512);
    
    
    if(read(sd_client,localbuf,512)<=0){//LEGGO IL NUMERO DI TERRE DA CONQUISTARE
        perror("read territori da conquista");
        return -1;
    }

    *territoriNecessari=atoi(localbuf);

    free(localbuf);
    return 1 ;     
}

int initTerritoriPosseduti(int sd_client, int* territoriPosseduti){
    char* localbuf = malloc(sizeof(char)*512);
    
    
    if(read(sd_client,localbuf,512)<=0){//LEGGO IL NUMERO DI TERRE POSSEDUTE
        perror("read territori da conquista");
        return -1;
    }

    *territoriPosseduti=atoi(localbuf);

    free(localbuf);
    return 1 ;
}

/*************************  STAMPE  *****************************/

/*STAMPA FINALE COL VINCITORE*/
void stampaVincitore(char listaVincitori[26][100]){ 
    int i = 0 ;
    fprintf(stdout,"\033[1;38;5;10m");    
    fprintf(stdout,"ABBIAMO FINITO LA PARTITA, COMPLIMENTI A TUTTI\n");
    fprintf(stdout,"\033[0m");
    fprintf(stdout,"La classifica dei vincitori \n\n");
    
    while(listaVincitori[i][0] != '\0' && i < 26){
        fprintf(stdout,"%d)%s\n",i+1,listaVincitori[i]);
        i++;
    }
    
    fprintf(stdout,"\n");
      
    
}

/*STAMPA DEL MSG DI FINE PARTITA CON ATTESA DI UN INPUT PER L'INIZIO DI UNA NUOVA PARTITA*/
void stampaFraseFinale(int sd_client){
    char buf[256];
    char * msg =malloc(sizeof(char) * 128);
    int n = 0 ; 
    strcpy(msg,"Invia 'n' partecipare in una nuova partita : ");
        
    do{
        write(STDOUT_FILENO,msg,strlen(msg));
        n = read(STDIN_FILENO,buf,256);
        buf[n-1] = '\0';
        fprintf(stdout,"\n");

    }while(strcmp(buf,"n") != 0);

    conferma(sd_client);
    free(msg);
    
}

/*STAMPA DEL MSG DI INTRO E LEGENDA*/
void stampaLegenda(int colore , char carattere){
    fprintf(stdout,"*************************************************************************\n");
    fprintf(stdout,"********   Benvenuto nella partita campione !! SEI LOGGATO CON : %s\n",usernameGlobale);
    fprintf(stdout,"********   Il colore che ti è stato assegnato : \033[48;5;%dm  \033[0m\n",colore);
    fprintf(stdout,"********   La lettera con cui giocherai questa partita : %c\n",carattere);
    fprintf(stdout,"*************************************************************************\n%126s\n","UTENTI COLLEGATI");
}

/*STAMPA DELLE STATISTICHE DELLA PARTITA IN CORSO E DELLE AZIONI DELL'UTENTE*/
void stampaStatistichePartita(int posseduti, char tempo[128],int territoriDaConquistare, char* ultimaAzione){
    
    fprintf(stdout,"Numero di territori posseduti : %d     \t\tNumero di territori da conquistare per vincere : %d\n",posseduti,territoriDaConquistare);
    fprintf(stdout,"Tempo di gioco rimanente : %s     \t\t%s\n\n",tempo,ultimaAzione);
}

/*STAMPA DELLA DESCRIZIONI DELLE MOSSE POSSIBILI E DELLA MODALITÀ DI LOGOUT*/
void stampaScelte(){
    fprintf(stdout,"Per muoversi nelle 4 direzioni basterà utilizzare le classiche W,A,S,D\n");
    fprintf(stdout,"L'azione corrispondente ad ogni lettera è il seguente :\n");    
    fprintf(stdout,"-  q -> LOGOUT\n\n");
    fprintf(stdout,"-  w -> NORD\n");
    fprintf(stdout,"-  a -> OVEST\n");
    fprintf(stdout,"-  s -> SUD\n");
    fprintf(stdout,"-  d -> EST\n");
    
    
}


//STAMPA LA MAPPA DI GIOCO CON LA LISTA UTENTI ACCANTO
void stampaMappa(char mappa[20][20],int coloreU, char carattereU, char utenti[26][100]){
    int cond = 0 ; 
    int k = 0;
     
    printf("\033[1m");

    printf("     ");
    for (int k = 0 ; k < 10 ; k ++){
        printf("  ");
        printf("%d",k+1);;
        printf("  ");
    } 
    for (int k = 10 ; k < 20 ; k ++){
        printf(" %d",k+1);
        printf("  ");
    }
        printf("\033[0m");

        printf("    %s\n",utenti[0]);
        printf("%-110s%s\n","",utenti[1]);
        
    
    for ( int i = 0 ; i < 20 ; i++){
        for (int j  = 0; j < 20 ; j++){
            if(j == 0){
                printf("\033[1m");

                if(i<9)
                    printf("  %d  ",i+1);
                else
                    printf(" %d  ",i+1);
                
                printf("\033[0m");

            }
            printf("  ");
            if(mappa[i][j]==carattereU){
                coloraCarattere(coloreU,carattereU);
            }
            else if(mappa[i][j]==tolower(carattereU)){
                coloraCarattere(coloreU,'-');
            }
            
            else if(mappa[i][j] == '-') 
                printf("%c",mappa[i][j]);
            
            else {
                k = 0 ; 
                cond = 0 ; 
                while( cond == 0 && k < 26 ){
                    if(mappa[i][j] == arrayCaratteri[k] /*&& arrayCaratteri[k] != carattereU && arrayColori[k] != coloreU*/){
                        coloraCarattere(arrayColori[k],arrayCaratteri[k]);
                        cond = 1 ; 
                    }
                    else if(mappa[i][j] == tolower(arrayCaratteri[k]) /*&& arrayCaratteri[k] != carattereU && arrayColori[k] != coloreU*/){
                        coloraCarattere(arrayColori[k],'-');
                        cond = 1 ;
                    }

                    k++;
                }
            }
            printf("  ");
        }
        printf("     %s\n",utenti[i+2]);
                
    }
    printf("%-110s%s\n","",utenti[22]);
    printf("%-110s%s\n","",utenti[23]);
    printf("%-110s%s\n","",utenti[24]);
    printf("%-110s%s\n","",utenti[25]);
    
}


/******  UTILITY  ******/
int conferma(int sd_client){
    if(write(sd_client,"1\0",2)<=0){//CONFERMA
        perror("Errore invio conferma");
        return -1;
    }

    return 1 ; 
}

//SOSTITUISCE LA POSIZIONE X Y DELLA MAPPA CON "CARATTERE" E RESTITUSCE IL CARATTERE 
//CHE C'ERA PRIMA DI ESSERE SOSTIUITO O '0' IN CASO DI ERRORE
char sostituisciCasella(int x, int y , char mappa [20][20], char carattere){
    char ret ; 
    if((x < 0 || x > 20)||(y < 0 || y > 20)){
        fprintf(stdout,("Errore x o y sballati\n"));
        return 0;
    }
    else{
        ret = mappa[x][y];
        mappa[x][y] = carattere;                
    }
return ret; 
}

//STAMPA IL CARATTERE COLORATO 
int coloraCarattere (int colore , char carattere){
    char tmp[30] ; 
    
    if(colore < 0 || colore > 255){
        fprintf(stdout,"Colore non corretto\n");
        return -1;
    }
    else{

        sprintf(tmp ,"\033[38;5;%dm",colore);
        fprintf(stdout,"%s",tmp);
        fprintf(stdout,"%c",carattere);
        fprintf(stdout,"\033[0m");   
    }
    return 1 ; 

}
