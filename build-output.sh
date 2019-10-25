#! /bin/sh
mkdir -p output
gcc cliente.c -o ./output/cliente
#gcc clienteDebug.c -o ./output/clienteDebug
gcc servidor.c funciones-servidor.c -o ./output/servidor
gcc servidor.c funciones-servidor-debug.c -o ./output/servidorDebug