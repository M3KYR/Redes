#! /bin/sh
mkdir -p output
gcc cliente.c funciones-cliente.c -o ./output/cliente
gcc servidor.c funciones-servidor.c -o ./output/servidor
