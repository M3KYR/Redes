#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_CLIENTS 30
#define MAX_MATCHES 10
#define MAX_QUEUE 10

volatile sig_atomic_t stop;

struct cliente {
  char usuario[20];
  char password[20];
  int socket;
  int estado;//0->conectao 1->usuario introducido 2->password introducido 3->en cola 4->en partida
  struct partida * partida;
  struct ficha * fichas[21];
};

struct partida {
  struct cliente * jugador1;
  struct cliente * jugador2;
  struct ficha * monton[14];
  char tablero[200];
};

struct ficha {
  int num1;
  int num2;
};
struct ficha * montonTotal[28];
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void compruebaEntrada(char * buffer, struct cliente arrayClientes[], int * numClientes, struct partida arrayPartidas[], int * numPartidas, struct cliente cola[],int * nCola, int socket, fd_set * readfds);
void Usuario(struct cliente * cliente, struct cliente arrayClientes[], char aux[], int numClientes);
void Password(struct cliente * cliente, struct cliente arrayClientes[], char aux[], int numClientes);
void Registro(struct cliente * cliente, struct cliente arrayClientes[], int numClientes, char buffer[]);
void desconectaClientes(struct cliente arrayClientes[], int * numClientes, fd_set * readfds);
bool compruebaUsuario(char usuario[], struct cliente arrayClientes[], int numClientes);
bool compruebaPass(char password[], struct cliente cliente, int numClientes);
bool registraUsuario(char usuario[], char password[], struct cliente arrayClientes[], int numClientes);
void Salir(struct cliente * cliente, struct cliente arrayClientes[], int * numClientes,fd_set * auxfds);

void manejadorSenal(int sig);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void iniciaPartida(struct cliente * cliente, struct cliente arrayClientes[], int numClientes, struct partida arrayPartidas[], int * numPartidas, struct cliente cola[], int * nCola);
void creaPartida(struct cliente * jugador1, struct cliente * jugador2, struct partida arrayPartidas[], int * numPartidas,struct ficha * montonT[]);
void inicioFichas(struct ficha * montonT[]);
void Fichas(struct cliente * jugador1, struct cliente * jugador2,struct ficha * montonT[],struct partida arrayPartidas[], int * numPartidas);
void colocarFicha(struct cliente * cliente, char buffer[]);
void robarFicha(struct cliente * cliente);
void pasarTurno(struct cliente * cliente);


void eliminaFicha(struct cliente * cliente,int pos);
struct cliente * popCola(struct cliente cola[],int * nCola);
void pushCola(struct cliente * cliente,struct cliente * cola[],int * nCola);

