#include "cabeceras-servidor.h"

int main ( )
{

	/*----------------------------------------------------
		Descriptor del socket y buffer de datos
	-----------------------------------------------------*/
	int sd, i, n, rv, numClientes=0;;
	struct sockaddr_in sockname, from;
	char buffer[100];
	socklen_t from_len;
	fd_set readfds;
	struct timeval tv;
	tv.tv_sec = 10;

	struct hostent * host;

  	struct clientes arrayClientes[MAX_CLIENTS];

	signal(SIGINT, manejadorSeñal);
	FD_ZERO(&readfds);
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
	do{

		if((arrayClientes[numClientes].socket = accept(sd, (struct sockaddr *)&from, &from_len)) == -1){
			perror("Error aceptando peticiones");
			exit(1);
		}
		else {
			FD_SET(arrayClientes[numClientes].socket,&readfds);
			n = arrayClientes[numClientes].socket + 1;
			send(arrayClientes[numClientes].socket,"+OK. Usuario conectado\n",100,0);
			arrayClientes[numClientes].estado = 0;
			numClientes++;
		}
		do
		{
			rv = select(n, &readfds, NULL, NULL, &tv);
			if (rv == -1)
				perror("Error en la operación de select");
			else if (rv == 0) {
				printf("Tiempo de espera agotado. Ningún cliente envió información en 10 segundos\n");
				desconectaClientes(arrayClientes,&numClientes);
			}
			else {
				for (i=0;i<numClientes;i++) {
					if (FD_ISSET(arrayClientes[i].socket, &readfds))
						recv(arrayClientes[i].socket, buffer, sizeof(buffer), 0); 
						compruebaEntrada(buffer,arrayClientes,&numClientes);
				}
			}


		}while(numClientes>0);

	}while(!stop && numClientes>0);

	desconectaClientes(arrayClientes,&numClientes);

	close(sd);
	
	return 0;

}
