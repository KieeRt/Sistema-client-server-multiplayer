#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <x86_64-linux-gnu/bits/fcntl-linux.h> //contiene le modalita di apertura per la funzione open()
#include <time.h>
#include "checkInput.h"
#include "pthread.h"
#define WRITE 1
#define READ 0
pthread_mutex_t mutexFileUtenti = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexFileLog = PTHREAD_MUTEX_INITIALIZER;

// compilazione: gcc -w file.c checkInput.h checkInput.c lib/inputReader.h lib/inputReader.c -lpthread
/*
*Scrive nel file log.txt nome e ip (se disponibili)
*ritorna 1 se inserimento e' avvenuto con succcesso -1 altrimenti
*Non controlla se i dati passati sono corretti ma solo se sono != NULL
*/
int log_write_access(char *nome, char *ip)
{
    pthread_mutex_lock(&mutexFileLog);
    int logfile;
    char *userMessaggio = "User name: ";
    char *addressMessaggio = "Ip: ";
    char dataMessaggio[30];
    time_t data;

    if (nome == NULL && ip == NULL)
        return -1;

    if ((logfile = open("log.txt", O_WRONLY | O_CREAT | O_APPEND)) < 0)
    {
        perror("open  log_write_access");
        return -1;
    }

    if (nome != NULL)
    {
        write(logfile, userMessaggio, strlen(userMessaggio));
        write(logfile, nome, strlen(nome));
        write(logfile, "\n", 1);
    }
    if (ip != NULL)
    {
        write(logfile, addressMessaggio, strlen(addressMessaggio));
        write(logfile, ip, strlen(ip));
        write(logfile, "\n", 1);
    }
    // GiornoSettimana Mese giornoMese ora:min:sec anno
    time(&data);
    ctime_r(&data, dataMessaggio);                        //salvo il tempo attuale nella stringa locale dataMessaggio
    write(logfile, dataMessaggio, strlen(dataMessaggio)); // contiene gia' \n
    close(logfile);
    pthread_mutex_unlock(&mutexFileLog);
    return 1;
}
int log_write_conquista(char *username, int x, int y)
{
    pthread_mutex_lock(&mutexFileLog);
    int logfile;

    char *userMessaggio = "username: ";
    char *coordinateMessaggio = "coordinate: ";
    char *conquistaMessaggio = "Conquista:";
    char *xString = malloc(sizeof(char) * 3);
    char *yString = malloc(sizeof(char) * 3);
    char dataMessaggio[30];
    time_t data;

    if (username == NULL)
        return -1;

    if ((logfile = open("log.txt", O_WRONLY | O_CREAT | O_APPEND)) < 0)
    {
        perror("open log_write_conquista");
        return -1;
    }
    
    sprintf(xString, "%d", x);
    sprintf(yString, "%d", y);

    write(logfile, conquistaMessaggio, strlen(conquistaMessaggio));
    write(logfile, "\n", 1);
    // username: nomeUser
    write(logfile, userMessaggio, strlen(userMessaggio));
    write(logfile, username, strlen(username));
    write(logfile, "\n", 1);
    // coordinate: x:y
    write(logfile, coordinateMessaggio, strlen(coordinateMessaggio));
    write(logfile, xString, strlen(xString));
    write(logfile, ":", 1);
    write(logfile, yString, strlen(yString));
    write(logfile, "\n", 1);
    // GiornoSettimana Mese giornoMese ora:min:sec anno
    time(&data);
    ctime_r(&data, dataMessaggio);                        //salvo il tempo attuale nella stringa locale dataMessaggio
    write(logfile, dataMessaggio, strlen(dataMessaggio)); // contiene gia' \n

    close(logfile);
    free(xString);
    free(yString);
    pthread_mutex_unlock(&mutexFileLog);
    return 1;
}

int log_write_logout(char* username, char *ip){

    log_write_message("Logout\n"); // ha gia' mutex
    log_write_access(username, ip); // ha gia' mutex

}
/*
*Scrive un messagio personalizzato nel file log.txt SENZA \n alla fine
*pensato per essere usato insieme(prima) alle altre funzioni come log_write_access(..) ecc
*ritorna 1 nel caso di successo e -1 altrimenti
*/
int log_write_message(char *messaggio)
{
    pthread_mutex_lock(&mutexFileLog);
    int logfile;
    if (messaggio == NULL)
    {
        return -1;
    }
    if ((logfile = open("log.txt", O_WRONLY | O_CREAT | O_APPEND)) < 0)
    {
        perror("open log_write_message");
        return -1;
    }
    write(logfile, messaggio, strlen(messaggio));
    close(logfile);
    pthread_mutex_unlock(&mutexFileLog);
    return 1;
}



int checkCredenziali(char *username, char *password)
{
    pthread_mutex_lock(&mutexFileUtenti);
    int p1[2];
    char buf[2];
    char *query;
    char *qUser;
    char *qPasw;
    int elementiTrovati;
    qUser = concatenation("@username: ", username);
    qPasw = concatenation(" @password: ", password);
    query = concatenation(qUser, qPasw);

    pid_t childID;

    if (pipe(p1) < 0)
    {
        perror("pipe");
        exit(0);
    }

    childID = fork();

    if (childID < 0)
    {
        perror("fork");
        exit(0);
    }
    else if (childID == 0)
    {
        close(p1[0]);

        dup2(p1[1], STDOUT_FILENO);
        close(p1[1]);

        execlp("grep", "grep", "-c", query, "utenti.txt", (char *)NULL);
        perror("exec");
        exit(0);
    }
    else
    {
        close(p1[1]);
        dup2(p1[0], STDIN_FILENO);
        close(p1[0]);

        read(STDIN_FILENO, buf, 2);
        elementiTrovati = atoi(buf);

        free(qUser);
        free(qPasw);
        free(query);
        pthread_mutex_unlock(&mutexFileUtenti);
        return elementiTrovati;
    }
}



int userExist(char *username)
{
    pthread_mutex_lock(&mutexFileUtenti);
     
    int p1[2];
    char buf[2];
    char *query;
    int elementiTrovati = 0;
    query = concatenation("@username: ", concatenation(username, " @"));
    

    pid_t childID;

    if (pipe(p1) < 0)
    {
        perror("pipe");
        exit(0);
    }

    childID = fork();

    if (childID < 0)
    {
        perror("fork");
        exit(0);
    }
    else if (childID == 0)
    {
        close(p1[0]);

        dup2(p1[1], STDOUT_FILENO);
        close(p1[1]);

        execlp("grep", "grep", "-c", query, "utenti.txt", (char *)NULL);
        perror("exec");
        exit(0);
    }
    else
    {
        close(p1[1]);
        dup2(p1[0], STDIN_FILENO);
        close(p1[0]);
        read(STDIN_FILENO, buf, 1);
        elementiTrovati = atoi(buf);
        
        free(query);
        pthread_mutex_unlock(&mutexFileUtenti);
        return elementiTrovati;
    }
}



int write_message_inFile(char *messaggio, char *nameFile)
{
    int tipoMutex = 0;
    if(strcmp(nameFile, "utenti.txt") == 0){
        tipoMutex = 1;
    }
    if(tipoMutex == 0){
        pthread_mutex_lock(&mutexFileLog);
    }else{
        pthread_mutex_lock(&mutexFileUtenti);
    }
    
    int logfile;
    if (messaggio == NULL)
    {
        return -1;
    }
    if ((logfile = open(nameFile, O_WRONLY | O_CREAT | O_APPEND)) < 0)
    {
        perror("open write_message_inFile");
        return -1;
    }
    write(logfile, messaggio, strlen(messaggio));
    close(logfile);

    if(tipoMutex == 0){
        pthread_mutex_unlock(&mutexFileLog);
    }else{
        pthread_mutex_unlock(&mutexFileUtenti);
    }
    return 1;
}
int registraUser(char *username, char *password)
{

    write_message_inFile(concatenation("@username: ", username), "utenti.txt");
    write_message_inFile(" ", "utenti.txt");
    write_message_inFile(concatenation("@password: ", password), "utenti.txt");
    write_message_inFile(" \n", "utenti.txt");

    return 1;
}
/*
int main(int argc, char const *argv[])
{   
   //log_write_conquista("dima", 10, 10);
   //checkCredenziali("mimmo", "esempio");
    userExist("mimmo");
    return 0;
}
*/