 
#ifndef FILE_H
#define FILE_H




/*
*Scrive nel file log.txt nome e ip (se disponibili)
*ritorna 1 se inserimento e' avvenuto con succcesso -1 altrimenti
*/
int log_write_access(char* nome, char* ip);
/*
*scrive nel file log.txt
*Conquista:
*username: userUsername
*territorio: x y
*ora: time
*/
int log_write_conquista(char* username, int x, int y);
/*
*scrine le file log.txt
*Logout:
*usernmae: userNamename
*ip: ipUsername
*ora
*/
int log_write_logout(char* username, char *ip);
/*
*Scrive un messagio personalizzato nel file log.txt SENZA \n alla fine
*ritorna 1 nel caso di successo e -1 altrimenti
*/
int log_write_message(char *messaggio);
/*
*cerca nel file utenti.txt
*utente con il username e passowrd passati
*ritorna 1 nel caso in cui trova utente 0 altrimenti    
*/
int checkCredenziali(char* username, char* password);
/*
*cerca nel file utenti.txt
*utente con il username passato, usato per la registrazione, controllo se username non e' gia' registrato nel db
*rirona 1 nel caso in cui trova utente 0 altrimenti
*La funzione si aspetta che nel file sia presente un unico userName, infatti ritorna il numero di occorenze
*il suo corretto funzionamento e' garantito dalla unicita' del userName nel file utenti.txt
*/
int userExist(char* username);
/*
*registro utente, trascrivendo il suo nome e cognome 
*ritorna 1 nel caso registrazione e' avvenuta con successo -1 altrimenti
*/
int registraUser(char* username, char* password);
/*
*scrive il messaggio personalizzato dentro il nameFi  senza \n
*/
int write_message_inFile(char *messaggio, char* nameFile); 

#endif // FILE_H_INCLUDED

