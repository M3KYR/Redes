#include "cabeceras-cliente.h"

int main ( )
{

	/*----------------------------------------------------
		Descriptor del socket y buffer de datos
	-----------------------------------------------------*/
	int sd, n, rv;
	struct sockaddr_in sockname;
	char buffer[100];
	socklen_t len_sockname;
	fd_set readfds;
	struct timeval tv;
	tv.tv_sec = 10;

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



	/* ------------------------------------------------------------------
		Se rellenan los campos de la estructura con la IP del
		servidor y el puerto del servicio que solicitamos
	-------------------------------------------------------------------*/
	sockname.sin_family = AF_INET;
	sockname.sin_port = htons(2000);
	sockname.sin_addr.s_addr =  inet_addr("127.0.0.1");

	/* ------------------------------------------------------------------
		Se solicita la conexión con el servidor
	-------------------------------------------------------------------*/
	len_sockname = sizeof(sockname);

	if (connect(sd, (struct sockaddr *)&sockname, len_sockname) == -1)
	{
		perror ("Error de conexión");
		exit(1);
	}
	
	FD_SET(sd,&readfds);
	n = sd + 1;
	/* ------------------------------------------------------------------
		Se transmite la información
	-------------------------------------------------------------------*/

	rv = select(n, &readfds, NULL, NULL, &tv);
	if (rv == -1)
		perror("Error en la operación de select");
	else if (rv == 0) {
		printf("Tiempo de espera agotado. El servidor no envió información en 10 segundos\n");
		exit(-1);
	}
  	else {
		if(recv(sd, buffer, 100, 0) == -1)
    		perror("Error en la operación de recv");
  		else
    		compruebaRespuesta(buffer);
	}
	do
	{
		puts("Teclee el mensaje a transmitir");
		gets(buffer);

		if(send(sd,buffer,100,0) == -1)
			perror("Error enviando datos");

    	rv = select(n, &readfds, NULL, NULL, &tv);
		if (rv == -1)
			perror("Error en la operación de select");
		else if (rv == 0) {
			printf("Tiempo de espera agotado. El servidor no envió información en 10 segundos\n");
			exit(-1);
		}
  		else {
			if(recv(sd, buffer, 100, 0) == -1)
    			perror("Error en la operación de recv");
  			else
    			compruebaRespuesta(buffer);
		}
	}while(strcmp(buffer, "+OK. Desconexión procesada\n") != 0);

	close(sd);
	return 0;
}
