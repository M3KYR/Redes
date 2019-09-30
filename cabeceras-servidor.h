#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_CLIENTS 40
#define MAX_MATCHES 20

 struct clientes {
   char usuario[20];
   char password[20];
   int socket;
   int estado;
 };
 
void compruebaEntrada(char * buffer,struct clientes arrayClientes[], int * numClientes);
void Usuario(struct clientes * cliente, struct clientes arrayClientes[], char aux[], int numClientes);
void Password(struct clientes * cliente, struct clientes arrayClientes[], char aux[], int numClientes);
void Registro(struct clientes * cliente, struct clientes arrayClientes[], int numClientes, char buffer[]);
void Salir(struct clientes * cliente, int * numClientes);
bool compruebaUsuario(char usuario[], struct clientes arrayClientes[], int numClientes);
bool compruebaPass(char password[], struct clientes cliente, int numClientes);
bool registraUsuario(char usuario[],char password[], struct clientes arrayClientes[], int numClientes);
void desconectaClientes(struct clientes arrayClientes[], int numClientes);
