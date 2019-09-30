#include "cabeceras-servidor.h"

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
