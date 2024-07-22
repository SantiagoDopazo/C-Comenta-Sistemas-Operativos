#ifndef CONSOLA_H
#define CONSOLA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <kernel.h>
#include <mostrarProcesos.h>
#include <planificadores.h>

void *manejadorDeConsola();
void ejecutarScript(char* path);
//void iniciar_proceso(char* path);
void finalizar_proceso(int pid);
void detener_planificacion();
void iniciar_planificacion();
void modificar_multiprogramacion(int valor);
void ejecutarScript(char* path);
void procesarLinea(char* linea,  const char* scriptBasePath);
char* construirPathCompleto(char* argumento);

void detener_cola_new(void* arg);
void detener_cola_ready(void* arg);
void detener_cola_exec(void* arg);
void detener_cola_blocked(void* arg);



#endif // CONSOLA_H

