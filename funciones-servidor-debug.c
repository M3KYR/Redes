#include "cabeceras-servidor.h"

/*----------------------------------------------------
	Funcion compruebaEntrada

Esta función se encarga de comprobar la opcion introducida por el cliente y ejecutará la función correspondiente a cada opción.
-----------------------------------------------------*/
void compruebaEntrada(char * buffer, struct cliente arrayClientes[], int * numClientes, struct partida arrayPartidas[], int * numPartidas,struct cliente * cola[],int * nCola, int socket, fd_set * readfds) {

	char aux[20], option[20];
	int i;
	struct cliente * cliente;

	for(i=0; i<*numClientes; i++) {
		if(socket == arrayClientes[i].socket) {
			cliente = &arrayClientes[i];
		}
	}

	bzero(option,sizeof(option));
	bzero(aux,sizeof(aux));
	sscanf(buffer,"%s %s", option, aux);

	if (strcmp(option, "USUARIO") == 0)
		Usuario(cliente,arrayClientes,aux,*numClientes);
	else if (strcmp(option, "PASSWORD") == 0)
		Password(cliente,arrayClientes,aux,*numClientes);
	else if (strcmp(option, "REGISTRO") == 0)
		Registro(cliente,arrayClientes,*numClientes,buffer);
	else if (strcmp(option, "INICIAR-PARTIDA")== 0) {
		iniciaPartida(cliente,arrayClientes,*numClientes,arrayPartidas,numPartidas,cola,nCola);
	}
	else if (strcmp(option, "COLOCAR-FICHA")== 0 && cliente->estado == 4) {
		colocarFicha(cliente,buffer);
	}
	else if (strcmp(option, "ROBAR-FICHA")== 0 && cliente->estado == 4) {
		robarFicha(cliente);
	}
	else if (strcmp(option, "PASO-TURNO")== 0 && cliente->estado == 4) {
		pasoTurno(cliente);
	}
	else if (strcmp(option, "SALIR") == 0)
		Salir(cliente,arrayClientes,numClientes,readfds);
	else {
		send(cliente->socket,"-ERR\n",250,0);
	}

}

/*----------------------------------------------------
	Funcion Usuario

Se ejecutará si el cliente ha introducido "USUARIO" en el buffer. La función comprueba el estado del cliente, que debe ser 0 (usuario: False, contraseña: False),
si es correcto pasa a comprobar el nombre introducido, y si ambas cosas son correctas asignará el nombre de usuario introducido al cliente y
cambiará el estado del cliente a 1 (usuario: True, contraseña: False). En los dos casos se avisará al usuario del resultado.
-----------------------------------------------------*/
void Usuario(struct cliente * cliente, struct cliente arrayClientes[], char aux[], int numClientes) {

	if (cliente->estado == 0) {
		if (compruebaUsuario(aux, arrayClientes, numClientes) == true) {
			send(cliente->socket,"+OK. Usuario correcto\n",250,0);
			strcpy(cliente->usuario,aux);
			cliente->estado = 1;
		}
		else
			send(cliente->socket,"-ERR. Usuario incorrecto\n",250,0);
	}
	else
		send(cliente->socket,"-ERR\n",250,0);

}

/*----------------------------------------------------
	Funcion Password

Se ejecutará si el cliente ha introducido "PASSWORD" en el buffer. La función comprueba el estado del cliente, que debe ser 1 (usuario: True, contraseña: False),
si es correcto pasa a comprobar el password introducido, y si ambas cosas son correctas asignará el password introducido al cliente y
cambiará el estado del cliente a 2 (usuario: True, contraseña: True). En los dos casos se avisará al usuario del resultado.
-----------------------------------------------------*/
void Password(struct cliente * cliente, struct cliente arrayClientes[], char aux[], int numClientes) {

	if (cliente->estado == 1) {
		if (compruebaPass(aux, *cliente, numClientes) == true) {
			send(cliente->socket,"+OK. Usuario validado\n",250,0);
			strcpy(cliente->password,aux);
			cliente->estado = 2;
		}
		else
			send(cliente->socket,"-ERR. Error en la validacion\n",250,0);
	}
	else
		send(cliente->socket,"-ERR\n",250,0);

}

/*----------------------------------------------------
	Funcion Registro

Se ejecutará si el cliente ha introducido "REGISTRO" en el buffer. La función comprueba el estado del cliente, que debe ser 0 (usuario: False, contraseña: False),
si es correcto pasa a comprobar los datos introducidos, y mandará un aviso al cliente informando si el registro se ha llevado a cabo con éxito o no.
-----------------------------------------------------*/
void Registro(struct cliente * cliente, struct cliente arrayClientes[], int numClientes, char buffer[]) {

	if (cliente->estado == 0) {
		char aux[20], aux1[20];
		sscanf(buffer,"REGISTRO -u %s -p %s", aux, aux1);
		if (registraUsuario(aux, aux1, arrayClientes, numClientes) == true)
			send(cliente->socket,"+OK. Usuario registrado\n",250,0);
		else
			send(cliente->socket,"-ERR. Error en el registro\n",250,0);
	}
	else
		send(cliente->socket,"-ERR\n",250,0);

}

/*----------------------------------------------------
	Funcion iniciaPartida

Se ejecutará si el cliente ha introducido "INICIAR-PARTIDA" en el buffer. La función comprueba el estado del cliente, que debe ser 2 (usuario: True, contraseña: True),
si es correcto pasa a comprobar la cola de espera para partida:
	· Si se forma un grupo de dos jugadores, manda un mensaje a ambos indicando que la partida va a comenzar y se procede a empezar el juego.
	· Si no hay suficientes jugadores en la cola, manda un mensaje al cliente indicando que debe esperar a la conexión de otro jugador.
-----------------------------------------------------*/
void iniciaPartida(struct cliente * cliente, struct cliente arrayClientes[], int numClientes, struct partida arrayPartidas[], int * numPartidas, struct cliente * cola[], int * nCola) {

	if (cliente->estado == 0) {
		
		if (*nCola>0 && *numPartidas<MAX_MATCHES) {
			struct cliente * jugador1 = popCola(cola,nCola);
			creaPartida(jugador1,cliente,arrayPartidas,numPartidas);
			send(cliente->socket,"+OK. Empieza la partida\n",250,0);
			send(jugador1->socket,"+OK. Empieza la partida\n",250,0);
			if (cliente->estado == 4) {
				cambiaTurno(cliente);
				send(jugador1->socket,"+OK. Turno del otro jugador\n",250,0);
			}
			else {
				cambiaTurno(jugador1);
				send(cliente->socket,"+OK. Turno del otro jugador\n",250,0);
			}
		}
		else if((*nCola>0 && *numPartidas==MAX_MATCHES) || *nCola==0) {
			send(cliente->socket,"+OK. Peticion recibida. Quedamos a la espera de mas jugadores\n",250,0);
			cliente->estado = 3;
			pushCola(cliente,cola,nCola);
		}
	}
	
	else {
		send(cliente->socket,"-ERR\n",250,0);
	}
}

/*----------------------------------------------------
	Funcion colocarFicha

Se ejecutará si el cliente ha introducido "COLOCAR-FICHA" en el buffer. La función comprueba el estado del cliente, que debe ser 2 (usuario: True, contraseña: True),
si es correcto pasa a comprobar la cola de espera para partida:
	· Si se forma un grupo de dos jugadores, manda un mensaje a ambos indicando que la partida va a comenzar y se procede a empezar el juego.
	· Si no hay suficientes jugadores en la cola, manda un mensaje al cliente indicando que debe esperar a la conexión de otro jugador.
-----------------------------------------------------*/
void colocarFicha(struct cliente * cliente, char buffer[]) {
	
	int num1,num2,i,colocada = 0;
	
	char extremo[20];
	
	struct partida * partida = cliente->partida;
	
	int nFichas = cliente->nFichas;
	
	int tTablero = partida->tTablero;
	
	sscanf(buffer,"COLOCAR-FICHA |%d|%d|,%s",&num1,&num2,extremo);
	
	bool bandera = false;
	
	for(i=0;i<nFichas;i++) {
		if(num1==cliente->fichas[i].num1 && num2==cliente->fichas[i].num2) {
			bandera = true;
			break;
		}
	}

	if(bandera == true && (strcmp(extremo,"DERECHA") == 0 ||strcmp(extremo,"IZQUIERDA") == 0)) {
		if(tTablero>0) {
			if(strcmp(extremo,"DERECHA")==0 && bandera==true) {
				if(num1==partida->tablero[tTablero-1]) {
					partida->tablero[tTablero] = num1;
					partida->tablero[tTablero+1] = num2;
					colocada = 1;
				}
				else if(num2==partida->tablero[tTablero-1]) {
					partida->tablero[tTablero] = num2;
					partida->tablero[tTablero+1] = num1;
					colocada = 1;
				}
			}
			else if(strcmp(extremo,"IZQUIERDA")==0 && bandera==true) {
				if(num1==partida->tablero[0]) {
					correPosiciones(partida);
					partida->tablero[1] = num1;
					partida->tablero[0] = num2;
					colocada = 1;
				}
				else if(num2==partida->tablero[0]) {
					correPosiciones(partida);
					partida->tablero[1] = num2;
					partida->tablero[0] = num1;
					colocada = 1;
				}
			}
		}
		else {
					partida->tablero[0] = num1;
					partida->tablero[1] = num2;
					colocada = 1;
		}
	}

		if(colocada == 1) {
			cliente->nFichas -= 1;
			for(;i<cliente->nFichas;i++) {
				cliente->fichas[i] = cliente->fichas[i+1];
			}
			partida->tTablero += 2;
			char respuesta[250];
			imprimeTablero(respuesta,partida);
			if(partida->jugador1->estado == 4) {
				partida->jugador1->estado = 5;
				partida->jugador2->estado = 4;
				cambiaTurno(partida->jugador2);
				send(partida->jugador1->socket,respuesta,250,0);
				usleep(10000);
				send(partida->jugador1->socket,"+OK. Turno del otro jugador\n",250,0);
			}
			else {
				partida->jugador1->estado = 4;
				partida->jugador2->estado = 5;
				cambiaTurno(partida->jugador1);
				send(partida->jugador2->socket,respuesta,250,0);
				usleep(10000);
				send(partida->jugador2->socket,"+OK. Turno del otro jugador\n",250,0);
			}
			if (cliente->nFichas==0) {
				finPartida(cliente);
			}
		}
		else {
			send(cliente->socket,"-Err. La ficha no puede ser colocada\n",250,0);
		}
}

/*----------------------------------------------------
	Funcion robarFicha

Se ejecutará si el cliente ha introducido "ROBAR-FICHA" en el buffer. La función comprueba el estado del cliente, que debe ser 4 (turno del jugador),
si es correcto pasa a comprobar las fichas del jugador:
	· Si el jugador puede colocar alguna de sus fichas, se lo indicará al jugador.
	· En caso contrario se le asignará una ficha del montón al jugador.
-----------------------------------------------------*/
void robarFicha(struct cliente * cliente) {

	int i, elegida;
	
	struct partida * partida = cliente->partida;
	
	int tam = cliente->partida->tMonton;
	
	int nFich = cliente->nFichas;
	
	if(compruebaFichas(cliente) == true) {
		send(cliente->socket,"+Ok. No es necesario robar ficha\n",250,0);
	}
	else {
		elegida = rand() % tam;
		cliente->fichas[nFich] = partida->monton[elegida];
		cliente->nFichas += 1;
		char respuesta[20];
		bzero(respuesta,sizeof(respuesta));
		strcpy(respuesta,"FICHA |");
		char aux1[4],aux2[4];
		sprintf(aux1,"%d",partida->monton[elegida].num1);
		strcat(respuesta,"|");
		strcat(respuesta,aux1);
		sprintf(aux2,"%d",partida->monton[elegida].num2);
		strcat(respuesta,"|");
		strcat(respuesta,aux2);
		strcat(respuesta,"|");
		strcat(respuesta,"\n");
		send(cliente->socket,respuesta,100,0);
		cambiaTurno(cliente);
		for(i = elegida; i<tam-1; i++) {
			partida->monton[i] = partida->monton[i+1];
		}
		partida->tMonton -= 1;
	}
}

/*----------------------------------------------------
	Funcion pasoTurno

Se ejecutará si el cliente ha introducido "PASO-TURNO" en el buffer. La función comprueba el estado del cliente, que debe ser 4 (turno del jugador),
si es correcto pasa a comprobar si el jugador puede colocar alguna de sus fichas o si el montón tiene fichas:
	· Si se da alguno de los casos anteriores, se lo indicará al jugador que tendrá que volver a mandar un mensaje al servidor.
	· Si no se da ninguno de los casos, pasará turno y lo indicará a ambos jugadores.
-----------------------------------------------------*/
void pasoTurno(struct cliente * cliente) {

	int monton = cliente->partida->tMonton;
	
	if(compruebaFichas(cliente) == true || monton > 0) {
		send(cliente->socket,"+OK. No es necesario pasar turno\n",250,0);
	}
	else {
		if(compruebaFichas(cliente->partida->jugador1) == false && compruebaFichas(cliente->partida->jugador2) == false && cliente->partida->tMonton == 0) {
			finPartida(cliente);
		}
		cliente->estado = 5;
		if(cliente->partida->jugador1->socket == cliente->socket) {
			cliente->partida->jugador2->estado = 4;
			cambiaTurno(cliente->partida->jugador2);
		}
		else {
			cliente->partida->jugador1->estado = 4;
			cambiaTurno(cliente->partida->jugador1);
		}
		send(cliente->socket,"+OK. Turno del otro jugador\n",250,0);
	}
}
/*----------------------------------------------------
	Funcion Salir

Se ejecutará si el cliente ha introducido "SALIR" en el buffer. La función comprueba el estado del cliente, según el cual realizará una de las siguientes acciones:
· Si el cliente estaba esperando para una partida, se le eliminará de la cola de espera.
· Si el cliente estaba jugando, se anulará la partida, se eliminará toda su información y se avisará a los demás jugadores.
En cualquiera de los casos se avisará al cliente de la desconexión.
-----------------------------------------------------*/
void Salir(struct cliente * cliente, struct cliente arrayClientes[], int * numClientes, fd_set * readfds) {

	int i;

	send(cliente->socket,"+OK. Desconexion procesada\n",250,0);
	close(cliente->socket);
	FD_CLR(cliente->socket,readfds);

	for(i=0;i<*numClientes - 1;i++) {
		if(arrayClientes[i].socket == cliente->socket)
			break;
	}
	for(;i<*numClientes - 1;i++) {
		arrayClientes[i] = arrayClientes[i+1];
	}
	*numClientes = *numClientes - 1;
}

/*----------------------------------------------------
	Funcion compruebaUsuario

Se ejecutará en la función Usuario(). La función comprueba que el usuario introducido exista en el fichero de clientes "usuario.txt" y, si es así,que no esté en uso por ningún otro cliente.
Si ambas cosas se cumplen devuelve True (usuario logeado con éxito), mientras que si una de ellas no se cumple devuelve False (fallo en el login).
-----------------------------------------------------*/
bool compruebaUsuario(char usuario[], struct cliente arrayClientes[], int numClientes) {

	FILE * f;
	char leido[20], aux[20];
	int j;
	bool bandera = false;

	f = fopen("usuario.txt", "r");

	if (f==NULL)
		return false;

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

/*----------------------------------------------------
	Funcion compruebaPass

Se ejecutará en la función Password(). La función comprueba que el password introducido exista en el fichero de clientes "usuario.txt" y, si es así, que no esté en uso por ningún otro cliente.
Si ambas cosas se cumplen devuelve True (usuario logeado con éxito), mientras que si una de ellas no se cumple devuelve False (fallo en el login).
-----------------------------------------------------*/
bool compruebaPass(char password[], struct cliente cliente, int numClientes) {

	FILE * f;
	char leido[20], aux[20];
	int j;
	bool bandera;

	f = fopen("usuario.txt", "r");

	if (f==NULL) {
		return false;
	}

  while(fscanf(f,"%s\t%s\n",leido,aux)==2){
  	if(strcmp(leido,cliente.usuario) == 0) {
  		if(strcmp(aux,password) == 0)
  			return true;
  	}

  }
  return false;
}

/*----------------------------------------------------
	Funcion registraUsuario

Se ejecutará en la función Registro(). La función comprueba que el usuario introducido no exista en el fichero de clientes "usuario.txt" y que el usuario y el password introducidos
tengan un tamaño mínimo. Si ambas cosas se cumplen, guarda el usuario y password en el fichero "usuario.txt" y devuelve True (usuario registrado con éxito), mientras que si
una de ellas no se cumple devuelve False (fallo en el registro).
-----------------------------------------------------*/
bool registraUsuario(char usuario[], char password[], struct cliente arrayClientes[], int numClientes) {

	if (compruebaUsuario(usuario,arrayClientes,numClientes) == true || strlen(usuario) < 2 || strlen(password) < 2)
		return false;

	FILE * f;

	f = fopen("usuario.txt", "a");

	fprintf(f,"%s\t%s\n", usuario,password);

	fclose(f);

	return true;

}

/*----------------------------------------------------
	Funcion creaPartida

Se ejecutará en la función iniciaPartida(). La función crea una partida en el array de partidas con el último jugador que llamó a la función iniciaPartida() y el primer jugador de la cola.
Asigna un puntero de la partida a ambos jugadores, aumenta el número de partidas en uno y pasa a asignar las fichas de la partida. 
-----------------------------------------------------*/
void creaPartida(struct cliente * jugador1, struct cliente * jugador2, struct partida arrayPartidas[], int * numPartidas) {
	
	struct partida * partida = &arrayPartidas[*numPartidas];

	partida->jugador1 = jugador1;
	
	partida->jugador2 = jugador2;

	jugador1->partida = partida;

	jugador2->partida = partida;

	partida->tTablero = 0;
	
	*numPartidas = *numPartidas + 1;
	
	Fichas(jugador1,jugador2, partida);
}

/*----------------------------------------------------
	Funcion Fichas

Se ejecutará en la función creaPartida. La función creará las 28 fichas para la partida y las asignará aleatoriamente a los jugadores y al montón. Además, comprobará qué jugador
tiene el doble más alto y le asignará el primer turno de la partida. En caso de que ningún jugador tenga dobles, será el que tenga la ficha más alta.
-----------------------------------------------------*/
void Fichas(struct cliente * jugador1, struct cliente * jugador2, struct partida * partida) {

	int pos,i,j=0,max=0,aux=2;
	
	bool bandera = false;
	bool cogida[28] = {false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false};

	struct ficha fichas[28];
	
	srand(time(NULL));

	jugador1->fichas = (struct ficha *)malloc(sizeof(struct ficha)*21);
	jugador2->fichas = (struct ficha *)malloc(sizeof(struct ficha)*21);
	partida->monton = (struct ficha *)malloc(sizeof(struct ficha)*14);
	partida->tMonton = 14;
	jugador1->nFichas = 7;
	jugador2->nFichas = 7;

	int f=0;
	for(i=0; i<7; i++) {
		for(j=i;j<7;j++) {
			fichas[f].num1=i;
			fichas[f].num2=j;
			f++;
		}
	}
	for(i=0; i<7; i++) {
		while(bandera == false) {
			pos = rand() % 28;
			if (cogida[pos] == false) {
				bandera = true;
			}
		}
		jugador1->fichas[i] = fichas[pos];
		bandera = false;
		cogida[pos] = true;
	}
	
	for(i=0; i<7; i++) {
		while(bandera == false) {
			pos = rand() % 28;
			if (cogida[pos] == false) {
				bandera = true;
			}
		}
		jugador2->fichas[i] = fichas[pos];
		bandera = false;
		cogida[pos] = true;
	}
	
	j=0;
	for(i=0; i<28; i++) {
		if (cogida[i] == false) {
			partida->monton[j] = fichas[i];
			j++;
		}
	}

	for(i=0; i<7; i++) {
		if((jugador1->fichas[i].num1 == jugador1->fichas[i].num2) && (jugador1->fichas[i].num1 * 2) > max) {
			max = jugador1->fichas[i].num1 * 2;
			aux = 0;
		}
		if((jugador2->fichas[i].num1 == jugador2->fichas[i].num2) && (jugador2->fichas[i].num1 * 2) > max) {
			max = jugador2->fichas[i].num1 * 2;
			aux = 1;
		}
	}

	if(aux == 2) {
		for(i=0; i<7; i++) {
			if((jugador1->fichas[i].num1 + jugador1->fichas[i].num2) > max) {
				max = jugador1->fichas[i].num1 + jugador1->fichas[i].num2;
				aux = 0;
			}
			if((jugador2->fichas[i].num1 + jugador2->fichas[i].num2) > max) {
				max = jugador2->fichas[i].num1 + jugador2->fichas[i].num2;
				aux = 1;
			}
		}
	}

	if(aux == 0) {
		jugador1->estado=4;
		jugador2->estado=5;
	}
	else {
		jugador1->estado=5;
		jugador2->estado=4;
	}
		
}

/*----------------------------------------------------
	Funcion compruebaFichas

Se ejecutará en la función robarFichas(). La función devuelve true si el jugador puede colocar una de sus fichas por algún extremo del tablero. En caso contrario devuelve false.
-----------------------------------------------------*/
bool compruebaFichas(struct cliente * cliente) {
	
	int i;
	
	struct partida * partida = cliente->partida;
	
	int tam = partida->tTablero;
	
	int nFichas = cliente->nFichas;
	
	for(i=0; i<nFichas;i++) {
		if(cliente->fichas[i].num1 == partida->tablero[0] || cliente->fichas[i].num2 == partida->tablero[0] || cliente->fichas[i].num1 == partida->tablero[tam-1] || cliente->fichas[i].num2 == partida->tablero[tam-1]) {
			return true;
		}
	}
	
	return false;
	
}

/*----------------------------------------------------
	Funcion correPosiciones

Se ejecutará en la función colocarFicha(). La función desplaza todo el tablero dos posiciones a la derecha para poder introducir una ficha por la izquierda.
-----------------------------------------------------*/
void correPosiciones(struct partida * partida) {
	
	int i;
	
	int tam = partida->tTablero;
	
	for(i=tam;i>=0;i--) {
		partida->tablero[i+2]=partida->tablero[i];
	}
	
}

/*----------------------------------------------------
	Funcion popCola

Se ejecutará en la función iniciaPartida(). La función devuelve el primer jugador que entró a la cola y disminuye el tamaño de esta.
-----------------------------------------------------*/
struct cliente * popCola(struct cliente * cola[],int * nCola) {

	int i;
	
	struct cliente * cliente = cola[0];
	
	*nCola = *nCola - 1;
	
	for (i = 0; i < *nCola; i++) {
		cola[i] = cola[i+1];
	}
	
	return cliente;
	
}

/*----------------------------------------------------
	Funcion pushCola

Se ejecutará en la función iniciaPartida(). La función introduce un jugador en la cola y aumenta el tamaño de esta.
-----------------------------------------------------*/
void pushCola(struct cliente * cliente,struct cliente * cola[],int * nCola) {

	
	cola[*nCola] = cliente;
	
	*nCola = *nCola + 1;
	
}

/*----------------------------------------------------
	Funcion cambiaTurno

Se ejecutará al final de cada turno. La función genera un mensaje con el tablero de la partida y se lo envía al cliente. Luego repite la operación con las fichas, y finalmente le manda
un mensaje de "OK" al cliente que comienza su turno.
-----------------------------------------------------*/
void cambiaTurno(struct cliente * cliente) {

	char tablero[250];
	char fichas[250];

	bzero(tablero,sizeof(tablero));
	bzero(fichas,sizeof(fichas));
	imprimeTablero(tablero,cliente->partida);
	usleep(10000);
	send(cliente->socket,tablero,250,0);
	usleep(10000);
	imprimeFichas(fichas,cliente);
	send(cliente->socket,fichas,250,0);
	usleep(10000);
	send(cliente->socket,"+OK. Es tu turno\n\n",250,0);
	
}

/*----------------------------------------------------
	Funcion cambiaTurno

Se ejecutará al final de cada turno. La función genera un mensaje con el tablero de la partida y se lo envía al cliente. Luego repite la operación con las fichas, y finalmente le manda
un mensaje de "OK" al cliente que comienza su turno.
-----------------------------------------------------*/
void finPartida(struct cliente * cliente) {

	int resultado1, resultado2;
	char buffer[250];
	bzero(buffer,sizeof(buffer));

	struct partida * partida = cliente->partida;

	if(partida->jugador1->nFichas == 0) {
		sprintf(buffer,"+OK. Partida Finalizada. %s ha ganado la partida\n",partida->jugador1->usuario);
		send(partida->jugador1->socket,buffer,sizeof(buffer),0);
		send(partida->jugador2->socket,buffer,sizeof(buffer),0);
	}
	else if(partida->jugador2->nFichas == 0) {
		sprintf(buffer,"+OK. Partida Finalizada. %s ha ganado la partida\n",partida->jugador2->usuario);
		send(partida->jugador1->socket,buffer,sizeof(buffer),0);
		send(partida->jugador2->socket,buffer,sizeof(buffer),0);
	}
	else {
		resultado1 = cuentaFichas(partida->jugador1);
		resultado2 = cuentaFichas(partida->jugador2);
		if (resultado1 < resultado2) {
			sprintf(buffer,"+OK. Partida Finalizada. %s ha ganado la partida\n",partida->jugador1->usuario);
			send(partida->jugador1->socket,buffer,sizeof(buffer),0);
			send(partida->jugador2->socket,buffer,sizeof(buffer),0);
		}
		else {
			sprintf(buffer,"+OK. Partida Finalizada. %s ha ganado la partida\n",partida->jugador2->usuario);
			send(partida->jugador1->socket,buffer,sizeof(buffer),0);
			send(partida->jugador2->socket,buffer,sizeof(buffer),0);
		}
	}

}

/*----------------------------------------------------
	Funcion cuentaFichas

Se ejecutará en la función robarFichas(). La función devuelve true si el jugador puede colocar una de sus fichas por algún extremo del tablero. En caso contrario devuelve false.
-----------------------------------------------------*/
int cuentaFichas(struct cliente * cliente) {
	
	int i,sum=0;
	
	int nFichas = cliente->nFichas;
	
	for(i=0; i<nFichas;i++) {
		
		sum += cliente->fichas[i].num1 + cliente->fichas[i].num2;

	}
	
	return sum;
	
}
/*----------------------------------------------------
	Funcion imprimeTablero

La función modifica un array respuesta con el tablero de la partida.
-----------------------------------------------------*/
void imprimeTablero(char * respuesta,struct partida * partida) {

	int i;
	char aux[4];
	int tam = partida->tTablero;
	strcpy(respuesta,"TABLERO ");

	for(i=0;i<tam;i++) {
		sprintf(aux,"%d",partida->tablero[i]);
		strcat(respuesta,"|");
		strcat(respuesta,aux);
		if(i%2 != 0)
		strcat(respuesta,"|");
	}
	strcat(respuesta,"\n\n");

}

/*----------------------------------------------------
	Funcion imprimeFichas

La función modifica un array fichas con las fichas del cliente.
-----------------------------------------------------*/
void imprimeFichas(char * fichas,struct cliente * cliente) {

	int i;
	char aux1[4], aux2[4];
	int tam = cliente->nFichas;
	strcpy(fichas,"FICHAS ");

	for(i=0;i<tam;i++) {
		sprintf(aux1,"%d",cliente->fichas[i].num1);
		strcat(fichas,"|");
		strcat(fichas,aux1);
		sprintf(aux2,"%d",cliente->fichas[i].num2);
		strcat(fichas,"|");
		strcat(fichas,aux2);
		strcat(fichas,"|");
	}
	strcat(fichas,"\n\n");
	
}
/*----------------------------------------------------
	Funcion desconectaClientes

Se ejecutará al terminar el bucle principal del servidor (mediante Ctrl+C) y desconectará a todos los clientes conectados al servidor.
-----------------------------------------------------*/
void desconectaClientes(struct cliente arrayClientes[], int * numClientes, fd_set * readfds) {

	int i;

	for(i=0;i<*numClientes;i++) {
		Salir(&arrayClientes[i],arrayClientes,numClientes,readfds);
	}

}

/*----------------------------------------------------
	Funcion manejadorSeñal

Se ejecutará al presionar Ctrl+C y finalizará el bucle principal.
-----------------------------------------------------*/
void manejadorSenal(int sig) {

	stop = 1;

}
