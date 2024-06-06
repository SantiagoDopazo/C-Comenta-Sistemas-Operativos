#ifndef KERNEL_H
#define KERNEL_H

#include <utils.h>
typedef struct
{		
    char* puerto_escucha; 
    char* ip_memoria;
    char* puerto_memoria; 
    char* ip_cpu;
    char* puerto_cpu_dispatch; 		
    char* puerto_cpu_interrupt; 				
    char* algoritmo_planificacion; 	
    int quantum;
    char** recursos;
    char** instancias_recursos;
    int grado_multiprogramacion;
   
} kernel_config;

kernel_config config_valores;
t_log* logger;
int memoria_fd;
int cpu_dispatch_fd;
int cpu_interrupt_fd;
int server_fd;
char* server_name = "kernel";

void cargar_configuracion(char* archivo_configuracion);


/*

static void procesar_conexion(void *void_args);
int server_escuchar(int fd_memoria);



// PCB
typedef enum {
    NEW,
    READY,
    RUNNING,
    BLOCKED,
    EXIT
} Estado;

typedef struct {
    char PC [5];
    char AX [2];
    char BX [2];
    char CX [2];
    char DX [2];
    char EAX [5];
    char EBX [5];
    char ECX [5];
    char EDX [5];
    char SI [5];
    char DI [5];
} CPU_Registers;


typedef struct {
    int PID; //id del proceso
    int pc; //direccionamiento
    int quantum;
    Estado estado; // duración del quantum 
    CPU_Registers* cpuRegisters; // puntero a cantidad de registros de la cpu (el valor lo tendría la cpu)
} PCB;

PCB t_pcb;

int obtener_pid(void) {
    t_pcb.PID;
}

int pid_counter = 1; //contador del pid, lo inicializo en 1
int quantum = 0; 
char* algoritmo_planificacion = NULL;

t_list* cola_de_procesos = NULL;

// LISTAS DE ESTADOS

t_list* lista_NEW = NULL;
t_list* lista_READY = NULL;
t_list* lista_EXIT = NULL;
t_list* lista_BLOCKED = NULL;
t_list* lista_RUNNING = NULL;

*/

#endif