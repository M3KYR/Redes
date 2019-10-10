#include "cabeceras-servidor.h"

int main ( )
{

	/*----------------------------------------------------
		Descriptor del socket y buffer de datos
	-----------------------------------------------------*/
	int sd, new_sd, i, rv, numClientes=0, numPartidas=0, nCola=0;
	struct sockaddr_in sockname, from;
	char buffer[250];
	socklen_t from_len;
    fd_set readfds, auxfds;
	struct timeval tv = {10,0};
	int on, ret;

	struct hostent * host;

  	struct cliente arrayClientes[MAX_CLIENTS];
	struct partida arrayPartidas[MAX_MATCHES];
	struct cliente cola[MAX_QUEUE];

	signal(SIGINT, manejadorSenal);
	/* --------------------------------------------------
		Se abre el socket
	---------------------------------------------------*/
  	sd = socket (AF_INET, SOCK_STREAM, 0);
	if (sd == -1)
	{
		perror("No se puede abrir el socket cliente\n");
    		exit (1);
	}

    on=1;
    ret = setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	sockname.sin_family = AF_INET;
	sockname.sin_port = htons(2050);
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

    FD_ZERO(&readfds);
    FD_ZERO(&auxfds);
    FD_SET(sd,&readfds);
    FD_SET(0,&readfds);
	/*-----------------------------------------------------------------------
		El servidor acepta una petición
	------------------------------------------------------------------------ */
	do{
		auxfds = readfds;
		rv = select(FD_SETSIZE, &auxfds, NULL, NULL, NULL);
		if (rv == -1)
			perror("Error en la operación de select");
		else if (rv == 0) {
			printf("Tiempo de espera agotado. Ningún cliente envió información en 10 segundos\n");
		}
		else {
			for (i=0;i<FD_SETSIZE;i++) {
				if (FD_ISSET(i, &auxfds)) {
					if(i==sd) {
						if((new_sd = accept(sd, (struct sockaddr *)&from, &from_len)) == -1){
							perror("Error aceptando peticiones");
							exit(1);
						}
						else {
							if(numClientes < MAX_CLIENTS) {
								arrayClientes[numClientes].socket = new_sd;
								FD_SET(new_sd,&readfds);
								send(new_sd,"+OK. Usuario conectado\n",100,0);
								arrayClientes[numClientes].estado = 0;
								numClientes++;
							}
							else {
								send(new_sd,"Número máximo de clientes alcanzado. Inténtalo más tarde\n",100,0);
								close(new_sd);
							}
						}
					}
					else {
						bzero(buffer,sizeof(buffer));
						if((recv(i, buffer, sizeof(buffer), 0)) > 0)
							compruebaEntrada(buffer,arrayClientes,&numClientes,arrayPartidas,&numPartidas,cola,&nCola,i,&readfds);
					}
				}
			}
		}

	}while(!stop && rv > 0);

	desconectaClientes(arrayClientes,&numClientes,&readfds);

	close(sd);

	return 0;

}
