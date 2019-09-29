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
/*
 * El servidor ofrece el servicio de incrementar un número recibido de un cliente
 */
 struct clientes {
   char usuario[20];
   char password[20];
   int socket;
   int estado;
 };

bool compruebaUsuario(char usuario[], struct clientes arrayClientes[], int numClientes);
bool compruebaPass(char password[], struct clientes arrayClientes[], int numClientes);
bool registraUsuario(char usuario[],char password[], struct clientes arrayClientes[], int numClientes);

int main ( )
{

	/*----------------------------------------------------
		Descriptor del socket y buffer de datos
	-----------------------------------------------------*/
	int sd, numClientes=0;;
	struct sockaddr_in sockname, from;
	char buffer[100], aux[20], aux1[20], option[20];
	socklen_t from_len;

	struct hostent * host;

  struct clientes arrayClientes[MAX_CLIENTS];

	/* --------------------------------------------------
		Se abre el socket
	---------------------------------------------------*/
  	sd = socket (AF_INET, SOCK_STREAM, 0);
	if (sd == -1)
	{
		perror("No se puede abrir el socket cliente\n");
    		exit (1);
	}


	sockname.sin_family = AF_INET;
	sockname.sin_port = htons(2000);
	sockname.sin_addr.s_addr =  INADDR_ANY;

	if (bind (sd, (struct sockaddr *) &sockname, sizeof (sockname)) == -1)
	{
		perror("Error en la operación bind");
		exit(1);
	}


   	/*---------------------------------------------------------------------
		Del las peticiones que vamos a aceptar sólo necesitamos el
		tamaño de su estructura, el resto de información (familia, puerto,
		ip), nos la proporcionará el método que recibe las peticiones.
   	----------------------------------------------------------------------*/
		from_len = sizeof (from);


		if(listen(sd,1) == -1){
			perror("Error en la operación de listen");
			exit(1);
		}

	/*-----------------------------------------------------------------------
		El servidor acepta una petición
	------------------------------------------------------------------------ */
		while(1){

			if((arrayClientes[numClientes].socket = accept(sd, (struct sockaddr *)&from, &from_len)) == -1){
				perror("Error aceptando peticiones");
				exit(1);
			}
      else {
        send(arrayClientes[numClientes].socket,"+OK. Usuario conectado\n",100,0);
        arrayClientes[numClientes].estado = 0;
        numClientes++;
      }
			do
			{
				if(recv(arrayClientes[numClientes - 1].socket, buffer, 100, 0) == -1)
					perror("Error en la operación de recv");

          sscanf(buffer,"%s %s", option, aux);
          if (strcmp(option, "USUARIO")== 0 && arrayClientes[numClientes - 1].estado == 0) {
            if (compruebaUsuario(aux, arrayClientes, numClientes) == true)
              send(arrayClientes[numClientes - 1].socket,"+OK. Usuario correcto\n",100,0);
            else
              send(arrayClientes[numClientes - 1].socket,"-ERR. Usuario incorrecto\n",100,0);
          }
          else if (strcmp(option, "PASSWORD")== 0 && arrayClientes[numClientes - 1].estado == 1) {
            if (compruebaPass(aux, arrayClientes, numClientes) == true)
              send(arrayClientes[numClientes - 1].socket,"+OK. Usuario validado\n",100,0);
            else
              send(arrayClientes[numClientes - 1].socket,"-ERR. Error en la validacion\n",100,0);
          }
          else if (strcmp(option, "REGISTRO")== 0 && arrayClientes[numClientes - 1].estado == 0) {
            sscanf(buffer,"REGISTRO -u %s -p %s", aux, aux1);
            if (registraUsuario(aux, aux1, arrayClientes, numClientes) == true)
              send(arrayClientes[numClientes - 1].socket,"+OK. Usuario registrado\n",100,0);
            else
              send(arrayClientes[numClientes - 1].socket,"-ERR. Error en el registro\n",100,0);
          }
          else if (strcmp(option, "INICIAR-PARTIDA")== 0 && arrayClientes[numClientes - 1].estado == 2) {
            send(arrayClientes[numClientes - 1].socket,"+OK. Empieza la partida\n",100,0);
          }
          else if (strcmp(option, "SALIR")== 0) {
            send(arrayClientes[numClientes - 1].socket,"+OK. Desconexión procesada\n",100,0);
          }
          else {
            send(arrayClientes[numClientes - 1].socket,"-ERR\n",100,0);
          }

			}while(strcmp(buffer, "FIN")!=0);

			close(arrayClientes[numClientes - 1].socket);
		}

		close(sd);
		return 0;

}

bool compruebaUsuario(char usuario[], struct clientes arrayClientes[], int numClientes) {

  FILE * f;
  char leido[20], aux[20];
  int j;
  bool bandera;

  f = fopen("usuario.txt", "r");

  if (f==NULL) {
    return false;
  }

  while(fscanf(f,"%s\t%s\n",leido,aux)==2){
    if(strcmp(usuario,leido)==0){
      fclose(f);
      for (j = 0; j < numClientes - 1; j++){
        if (strcmp(arrayClientes[j].usuario,usuario)==0){
          bandera=true;
            break;
          }
        }
      if(bandera==true)
        return false;
      else
        return true;
    }
  }

  fclose(f);
  return false;
}

bool compruebaPass(char password[], struct clientes arrayClientes[], int numClientes) {

  FILE * f;
  char leido[20], aux[20];
  int j;
  bool bandera;

  f = fopen("usuario.txt", "r");

  if (f==NULL) {
    return false;
  }

  while(fscanf(f,"%s\t%s\n",leido,aux)==2){
    if(strcmp(password,leido)==0){
      fclose(f);
      for (j = 0; j < numClientes - 1; j++){
        if (strcmp(arrayClientes[j].password,password)==0){
          bandera=true;
            break;
        }
      }
      if(bandera==true)
        return false;
      else
        return true;
    }
  }
  return false;
}

bool registraUsuario(char usuario[],char password[], struct clientes arrayClientes[], int numClientes) {

  if ((compruebaUsuario(usuario,arrayClientes,numClientes) == true) || strlen(usuario) < 2 || strlen(password) < 2)
    return false;

  FILE * f;

  f = fopen("usuario.txt", "a");

  fprintf(f,"%s\t%s\n", usuario,password);
  fclose(f);
  return true;
}
