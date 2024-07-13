#ifndef PLANIFICADORES_H
#define PLANIFICADORES_H

#include <utils.h>
#include <semaphore.h>
#include <kernel.h>
#include <semaforos.h>
#include <commons/temporal.h>
#include <conexiones.h>

//HILOS
//pthread_t hilo_largo_plazo;
//pthread_t hilo_corto_plazo;

int totalProcesosEnSistema();
void* planificadorNew();
void* planificadorReady();
void comportamientoFIFO();
void* manejadorDeQuantum(void *quantum);
void cambiarAReady(t_list* cola);
PCB* cambiarARunning(t_list* lista_READY);
PCB* cambiarAExitDesdeRunning(t_list* cola);
void comportamientoRR();


extern pthread_t hiloQuantum;
extern t_temporal* tiempoVRR;
extern int64_t tiempoEjecutando;

extern void inicializar_sem_planificadores();
extern void planificador_largo_plazo();
extern void planificador_corto_plazo();
extern void planificar_fifo();
extern void planificar_round_robin();
extern void planificar_virtual_round_robin();
extern void planificar();
extern void inicializar_planificadores();
extern int hilosPlanificadores(void);

#endif