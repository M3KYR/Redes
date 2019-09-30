#include "cabeceras-servidor.h"

/*----------------------------------------------------
	Funcion compruebaEntrada
	
Esta función se encarga de comprobar la opcion introducida por el cliente y ejecutará la función correspondiente a cada opción.
-----------------------------------------------------*/
void compruebaEntrada(char * buffer,struct clientes arrayClientes[], int * numClientes) {

	char aux[20], option[20];
	
	struct clientes * cliente = &arrayClientes[*numClientes -1];
 
	sscanf(buffer,"%s %s", option, aux);
	
	if (strcmp(option, "USUARIO") == 0)
		Usuario(cliente,arrayClientes,aux,*numClientes);
	else if (strcmp(option, "PASSWORD") == 0)
		Password(cliente,arrayClientes,aux,*numClientes);
	else if (strcmp(option, "REGISTRO") == 0)
		Registro(cliente,arrayClientes,*numClientes,buffer);
	else if (strcmp(option, "INICIAR-PARTIDA")== 0 && arrayClientes[*numClientes - 1].estado == 2) {
		send(cliente->socket,"+OK. Empieza la partida\n",100,0);
	}
	else if (strcmp(option, "SALIR") == 0)
		Salir(cliente,numClientes);
	else {
		send(cliente->socket,"-ERR\n",100,0);
	}
	
}

/*----------------------------------------------------
	Funcion Usuario
	
Se ejecutará si el cliente ha introducido "USUARIO" en el buffer. La función comprueba el estado del cliente, que debe ser 0 (usuario: False, contraseña: False),
si es correcto pasa a comprobar el nombre introducido, y si ambas cosas son correctas asignará el nombre de usuario introducido al cliente y
cambiará el estado del cliente a 1 (usuario: True, contraseña: False). En los dos casos se avisará al usuario del resultado.
-----------------------------------------------------*/
void Usuario(struct clientes * cliente, struct clientes arrayClientes[], char aux[], int numClientes) {

	if (cliente->estado == 0) {
		if (compruebaUsuario(aux, arrayClientes, numClientes) == true) {
			send(cliente->socket,"+OK. Usuario correcto\n",100,0);
			strcpy(cliente->usuario,aux);
			cliente->estado = 1;
		}
		else
			send(cliente->socket,"-ERR. Usuario incorrecto\n",100,0);
	}
	else
		send(cliente->socket,"-ERR\n",100,0);
	
}

/*----------------------------------------------------
	Funcion Password
	
Se ejecutará si el cliente ha introducido "PASSWORD" en el buffer. La función comprueba el estado del cliente, que debe ser 1 (usuario: True, contraseña: False),
si es correcto pasa a comprobar el password introducido, y si ambas cosas son correctas asignará el password introducido al cliente y
cambiará el estado del cliente a 2 (usuario: True, contraseña: True). En los dos casos se avisará al usuario del resultado.
-----------------------------------------------------*/
void Password(struct clientes * cliente, struct clientes arrayClientes[], char aux[], int numClientes) {

	if (cliente->estado == 1) {
		if (compruebaPass(aux, *cliente, numClientes) == true) {
			send(cliente->socket,"+OK. Usuario validado\n",100,0);
			strcpy(cliente->password,aux);
			cliente->estado = 2;
		}
		else
			send(cliente->socket,"-ERR. Error en la validacion\n",100,0);
	}
	else
		send(cliente->socket,"-ERR\n",100,0);
              
}

/*----------------------------------------------------
	Funcion Registro
	
Se ejecutará si el cliente ha introducido "REGISTRO" en el buffer. La función comprueba el estado del cliente, que debe ser 0 (usuario: False, contraseña: False),
si es correcto pasa a comprobar los datos introducidos, y mandará un aviso al cliente informando si el registro se ha llevado a cabo con éxito o no.
-----------------------------------------------------*/
void Registro(struct clientes * cliente, struct clientes arrayClientes[], int numClientes, char buffer[]) {

	if (cliente->estado == 0) {
		char aux[20], aux1[20];
		sscanf(buffer,"REGISTRO -u %s -p %s", aux, aux1);
		if (registraUsuario(aux, aux1, arrayClientes, numClientes) == true)
			send(cliente->socket,"+OK. Usuario registrado\n",100,0);
		else
			send(cliente->socket,"-ERR. Error en el registro\n",100,0);
	}
	else
		send(cliente->socket,"-ERR\n",100,0);
          
}

/*----------------------------------------------------
	Funcion Salir
	
Se ejecutará si el cliente ha introducido "SALIR" en el buffer. La función comprueba el estado del cliente, según el cual realizará una de las siguientes acciones:
· Si el cliente estaba esperando para una partida, se le eliminará de la cola de espera.
· Si el cliente estaba jugando, se anulará la partida, se eliminará toda su información y se avisará a los demás jugadores.
En cualquiera de los casos se avisará al cliente de la desconexión.
-----------------------------------------------------*/
void Salir(struct clientes * cliente,int * numClientes) {

	send(cliente->socket,"+OK. Desconexión procesada\n",100,0);
	close(cliente->socket);
	
}

/*----------------------------------------------------
	Funcion compruebaUsuario
	
Se ejecutará en la función Usuario(). La función comprueba que el usuario introducido exista en el fichero de clientes "usuario.txt" y, si es así,que no esté en uso por ningún otro cliente.
Si ambas cosas se cumplen devuelve True (usuario logeado con éxito), mientras que si una de ellas no se cumple devuelve False (fallo en el login).
-----------------------------------------------------*/
bool compruebaUsuario(char usuario[], struct clientes arrayClientes[], int numClientes) {

	FILE * f;
	char leido[20], aux[20];
	int j;
	bool bandera;

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
	
Se ejecutará en la función Password(). La función comprueba que el password introducido exista en el fichero de clientes "usuario.txt" y, si es así,que no esté en uso por ningún otro cliente.
Si ambas cosas se cumplen devuelve True (usuario logeado con éxito), mientras que si una de ellas no se cumple devuelve False (fallo en el login).
-----------------------------------------------------*/
bool compruebaPass(char password[], struct clientes cliente, int numClientes) {

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

bool registraUsuario(char usuario[],char password[], struct clientes arrayClientes[], int numClientes) {

	if (compruebaUsuario(usuario,arrayClientes,numClientes) == true || strlen(usuario) < 2 || strlen(password) < 2)
		return false;

	FILE * f;

	f = fopen("usuario.txt", "a");

	fprintf(f,"%s\t%s\n", usuario,password);
  
	fclose(f);
  
	return true;
  
}

void desconectaClientes(struct clientes arrayclientes[], int numClientes) {

	int i;
	
	for(i=0;i<numClientes;i++) {
		close(arrayclientes[i].socket);
	}

}
