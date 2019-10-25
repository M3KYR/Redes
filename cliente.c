#include "cabeceras-cliente.h"

int main (int argc, char * argv[])
{

	/*----------------------------------------------------
		Descriptor del socket y buffer de datos
	-----------------------------------------------------*/
	int sd, rv;
	struct sockaddr_in sockname;
	char buffer[250];
	socklen_t len_sockname;
	fd_set readfds,auxfds;
	struct timeval tv = {10,0};
	int fin = 0;

	if(argc < 2) {
		printf("Error. El programa debe llamarse de la forma %s <ip-servidor>.\n",argv[0]);
		exit(-1);
	}
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
	sockname.sin_port = htons(2050);
	sockname.sin_addr.s_addr =  inet_addr(argv[1]);

	/* ------------------------------------------------------------------
		Se solicita la conexión con el servidor
	-------------------------------------------------------------------*/
	len_sockname = sizeof(sockname);

	if (connect(sd, (struct sockaddr *)&sockname, len_sockname) == -1)
	{
		perror ("Error de conexion");
		exit(1);
	}
	
    FD_ZERO(&auxfds);
    FD_ZERO(&readfds);
    
    FD_SET(0,&readfds);
    FD_SET(sd,&readfds);
	/* ------------------------------------------------------------------
		Se transmite la información
	-------------------------------------------------------------------*/
	system("clear");
	do
	{
		auxfds = readfds;

		rv = select(sd + 1, &auxfds, NULL, NULL, NULL);
		if (rv == -1)
			perror("Error en la operacion de select");
		else if (rv == 0) {
			printf("Tiempo de espera agotado. El servidor no envio informacion en 10 segundos\n");
			exit(-1);
		}
		else {
			if(FD_ISSET(sd, &auxfds)){
				bzero(buffer,sizeof(buffer));
				recv(sd,buffer,sizeof(buffer),0);
				if(strstr(buffer,"TABLERO"))
					system("clear");
				printf("%s",buffer);
				if(strcmp(buffer,"Numero maximo de clientes alcanzado. Intentalo mas tarde\n") == 0)
					fin = 1;
				if(strcmp(buffer,"+OK. Desconexion procesada\n") == 0)
					fin = 1;
			}
			else {
				if(turnoEspera(buffer) == 0) {
					if(FD_ISSET(0,&auxfds)){
						bzero(buffer,sizeof(buffer));
						fgets(buffer,sizeof(buffer),stdin);
						send(sd,buffer,sizeof(buffer),0);
					}
				}
			}
		}
    }while(fin == 0);

	close(sd);
	return 0;
}

int turnoEspera(char * buffer) {

	if(strcmp(buffer,"+OK. Peticion recibida. Quedamos a la espera de mas jugadores\n") == 0)
		return 1;
	if(strcmp(buffer,"+OK. Empieza la partida\n") == 0)
		return 1;
	if(strcmp(buffer,"+OK. Turno del otro jugador\n") == 0)
		return 1;
	if(strstr(buffer,"TABLERO"))
		return 1;
	if(strstr(buffer,"FICHAS"))
		return 1;
	return 0;
}