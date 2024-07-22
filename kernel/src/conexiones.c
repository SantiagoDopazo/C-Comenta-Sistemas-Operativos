#include <conexiones.h>
#include <configs.h>

int memoria_fd;
int cpu_dispatch_fd=0;
int cpu_interrupt_fd=0;
int server_fd;
char* server_name = "kernel";
int socketCliente;

//TODO mover esto a donde este comodo
uint32_t leerValorDelRegistro(char *registroAleer,CPU_Registers registros){
	if (strcmp(registroAleer,"PC")==0){
		return registros.PC;
	}else if (strcmp(registroAleer,"AX")==0){
		return registros.AX;
	}else if (strcmp(registroAleer,"BX")==0){
		return registros.BX;
	}else if (strcmp(registroAleer,"CX")==0){
		return registros.CX;
	}else if (strcmp(registroAleer,"DC")==0){
		return registros.DX;
	}else if (strcmp(registroAleer,"EAX")==0){
		return registros.EAX;
	}else if (strcmp(registroAleer,"EBX")==0){
		return registros.EBX;
	}else if (strcmp(registroAleer,"ECX")==0){
		return registros.ECX;
	}else if (strcmp(registroAleer,"EDX")==0){
		return registros.EDX;
	}else if (strcmp(registroAleer,"SI")==0){
		return registros.SI;
	}else if (strcmp(registroAleer,"DI")==0){
		return registros.DI;
	}else {/*TODO ERROR*/}
}

////////////////////////////////////////////////////////// PROCESO CONEXION //////////////////////////////////////////////////////////
t_paquete* recibirPaquete(socket){
	t_paquete* paquete = NULL;
	paquete = malloc(sizeof(t_paquete));
	paquete->buffer = NULL;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->stream = NULL;

	recv(socket, &(paquete->codigo_operacion), sizeof(op_code), 0);
	recv(socket, &(paquete->buffer->size), sizeof(int), 0);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(socket, paquete->buffer->stream, paquete->buffer->size, 0);

	return paquete;
}

// INICIAR_PROCESO /home/utnso/tp-2024-1c-File-System-Fanatics/kernel/preliminares/script_solo_cpu.txt
void* conexionesDispatch()
{
	
	while (1)
	{
		t_paquete* paquete=recibirPaquete(cpu_dispatch_fd);
		void *stream = paquete->buffer->stream;
		PCB *procesoCPU;
		PCB* procesoKernel;
		switch (paquete->codigo_operacion)
		{
			case PROCESO_EXIT:
			{
				procesoCPU = recibirProcesoContextoEjecucion(stream);
				sem_wait(&semPlaniRunning);
				sem_post(&semPlaniRunning);
				pthread_mutex_lock(&mutexListaRunning);
				pthread_mutex_lock(&mutexListaExit);
				procesoKernel = list_remove(lista_RUNNING, 0);
				if(procesoCPU->PID == procesoKernel->PID){
					actualizarProceso(procesoCPU, procesoKernel);
				}
				else{
					log_error(loggerKernel,"los procesos que se quieren actualizar son distintos el de CPU:%d, Kernel:%d",procesoCPU->PID, procesoKernel->PID);

				}
				procesoKernel->estado = EXIT;
				list_add(lista_EXIT, procesoKernel); 
				//proceso = cambiarAExitDesdeRunning(lista_RUNNING);
				pthread_mutex_unlock(&mutexListaRunning);
				pthread_mutex_unlock(&mutexListaExit);
				sem_post(&semListaRunning);
				//mutex conexion memoria
				paquete_memoria_finalizar_proceso(procesoKernel->PID);

				log_info(loggerKernel, "Se elimino el proceso con pid: %d\n", procesoKernel->PID);

				//eliminarProceso(procesoKernel);
				break;
			}
			case INTERRUMPIR_PROCESO:
			{
				procesoCPU = recibirProcesoContextoEjecucion(stream);
				sem_wait(&semPlaniRunning);
				sem_post(&semPlaniRunning);
				pthread_mutex_lock(&mutexListaRunning);
				pthread_mutex_lock(&mutexListaExit);
				procesoKernel = list_remove(lista_RUNNING, 0);
				if(procesoCPU->PID == procesoKernel->PID){
					actualizarProceso(procesoCPU, procesoKernel);
				}
				else{
					log_error(loggerKernel,"los procesos que se quieren actualizar son distintos el de CPU:%d, Kernel:%d",procesoCPU->PID, procesoKernel->PID);

				}
				procesoKernel->estado = EXIT;
				list_add(lista_EXIT, procesoKernel); 
				//proceso = cambiarAExitDesdeRunning(lista_RUNNING);
				pthread_mutex_unlock(&mutexListaRunning);
				pthread_mutex_unlock(&mutexListaExit);
				sem_post(&semListaRunning);
				//mutex conexion memoria
				paquete_memoria_finalizar_proceso(procesoKernel->PID);

				log_info(loggerKernel, "Se elimino el proceso con pid: %d\n", procesoKernel->PID);
				break;

			}
			case PROCESO_INTERRUMPIDO_CLOCK:
			{
				procesoCPU = recibirProcesoContextoEjecucion(stream);
				sem_wait(&semPlaniRunning);
				sem_post(&semPlaniRunning);
				
				pthread_mutex_lock(&mutexListaRunning);
				pthread_mutex_lock(&mutexListaReady);
				sem_wait(&semPlaniReadyClock);
				sem_post(&semPlaniReadyClock);
				procesoKernel = list_remove(lista_RUNNING, 0);
				if(!strcmp(configuracionKernel.ALGORITMO_PLANIFICACION, "RR") || !strcmp(configuracionKernel.ALGORITMO_PLANIFICACION, "VRR")){
					actualizarProceso(procesoCPU, procesoKernel);
					procesoKernel->estado = READY;
					list_add(lista_READY, procesoKernel);
				}
				log_info(loggerKernel, "Se interrumpio el proceso con pid: %d por fin de QUANTUM\n", procesoKernel->PID);
				pthread_mutex_unlock(&mutexListaRunning);
				pthread_mutex_unlock(&mutexListaReady);
				sem_post(&semListaReady);
				sem_post(&semListaRunning);
				break;
			}
			case PROCESO_WAIT:
			{
				// obtener recurso
				int recursoLength;
				char *recursoRecibido;
                memcpy(&recursoLength, stream, sizeof(int));
                stream += sizeof(int);
                recursoRecibido = malloc(recursoLength);
                memcpy(recursoRecibido, stream, recursoLength);
				printf("Recurso recibido. Recurso: %s\n: ", recursoRecibido);
				Recurso *recursoEncontrado = NULL;
					for (int i = 0; i < list_size(configuracionKernel.RECURSOS); i++) {
    					Recurso *recursoActual = list_get(configuracionKernel.RECURSOS, i);
    					if (strcmp(recursoActual->nombre, recursoRecibido) == 0) {
       						recursoEncontrado = recursoActual;
        					break;
						}
					}
				// Si no encuentro el recurso -- Finaliza el proceso ok
	 			if (recursoEncontrado == NULL) {
					//enviarMensaje("RECURSO NO ENCONTRADO", cpu_dispatch_fd);
					printf("Recurso %s no encontrado. Terminando proceso %d\n");

					t_paquete* paquete2=recibirPaquete(cpu_dispatch_fd);
					void *stream2 = paquete2->buffer->stream;

					procesoCPU = recibirProcesoContextoEjecucion(stream2);
					pthread_mutex_lock(&mutexListaRunning);
					pthread_mutex_lock(&mutexListaExit);
					procesoKernel = list_remove(lista_RUNNING, 0);

					if(procesoCPU->PID == procesoKernel->PID){
						actualizarProceso(procesoCPU, procesoKernel);
					}
					else{
						log_error(loggerKernel,"los procesos que se quieren actualizar son distintos el de CPU:%d, Kernel:%d",procesoCPU->PID, procesoKernel->PID);
					}
					procesoKernel->estado = EXIT;
					list_add(lista_EXIT, procesoKernel); 
					pthread_mutex_unlock(&mutexListaRunning);
					pthread_mutex_unlock(&mutexListaExit);
					sem_post(&semListaRunning);
					paquete_memoria_finalizar_proceso(procesoKernel->PID);

					log_info(loggerKernel, "Se elimino el proceso con pid: %d\n", procesoKernel->PID);

   				}

				// Si el recurso existe y tiene instancias
				if (recursoEncontrado->instancias > 0) {
					recursoEncontrado->instancias--;
					enviar_resultado_recursos(WAIT_SUCCESS, cpu_dispatch_fd);
				} 

				// Si el recurso existe pero no hay instancias del mismo se bloquea el proceso
				else {
					// Bloquear el proceso 
					t_paquete* paquete3=recibirPaquete(cpu_dispatch_fd);
					void *stream3 = paquete3->buffer->stream;
					enviar_resultado_recursos(WAIT_BLOCK, cpu_dispatch_fd);
					procesoCPU = recibirProcesoContextoEjecucion(stream3);
					pthread_mutex_lock(&mutexListaRunning);
					pthread_mutex_lock(&mutexListaBlockedRecursos);
					procesoKernel = list_remove(lista_RUNNING, 0);
					actualizarProceso(procesoCPU, procesoKernel);
					procesoKernel->estado = BLOCKED;
					procesoKernel->recursoBloqueante = recursoRecibido;
					list_add(lista_BLOCKED_RECURSOS, procesoKernel);
					pthread_mutex_unlock(&mutexListaRunning);
					pthread_mutex_unlock(&mutexListaBlockedRecursos);
					sem_post(&semListaRunning);
					printf("Proceso %d bloqueado por falta de instancias del recurso %s\n", procesoKernel->PID, recursoRecibido);
					
				}
				break;
			}
			case PROCESO_SIGNAL:
			{
				// obtener recurso
				int recursoLength;
				char *recursoRecibido;
                memcpy(&recursoLength, stream, sizeof(int));
                stream += sizeof(int);
                recursoRecibido = malloc(recursoLength);
                memcpy(recursoRecibido, stream, recursoLength);
				printf("Recurso recibido. Recurso: %s\n: ", recursoRecibido);
				Recurso *recursoEncontrado = NULL;
					for (int i = 0; i < list_size(configuracionKernel.RECURSOS); i++) {
    					Recurso *recursoActual = list_get(configuracionKernel.RECURSOS, i);
    					if (strcmp(recursoActual->nombre, recursoRecibido) == 0) {
       						recursoEncontrado = recursoActual;
        					break;
						}
					}
				// Si no encuentro el recurso -- Finaliza el proceso ok
	 			if (recursoEncontrado == NULL) {
					//enviarMensaje("RECURSO NO ENCONTRADO", cpu_dispatch_fd);
					printf("Recurso %s no encontrado. Terminando proceso %d\n");

					t_paquete* paquete2=recibirPaquete(cpu_dispatch_fd);
					void *stream2 = paquete2->buffer->stream;

					procesoCPU = recibirProcesoContextoEjecucion(stream2);
					pthread_mutex_lock(&mutexListaRunning);
					pthread_mutex_lock(&mutexListaExit);
					procesoKernel = list_remove(lista_RUNNING, 0);

					if(procesoCPU->PID == procesoKernel->PID){
						actualizarProceso(procesoCPU, procesoKernel);
					}
					else{
						log_error(loggerKernel,"los procesos que se quieren actualizar son distintos el de CPU:%d, Kernel:%d",procesoCPU->PID, procesoKernel->PID);
					}
					procesoKernel->estado = EXIT;
					list_add(lista_EXIT, procesoKernel); 
					pthread_mutex_unlock(&mutexListaRunning);
					pthread_mutex_unlock(&mutexListaExit);
					sem_post(&semListaRunning);
					paquete_memoria_finalizar_proceso(procesoKernel->PID);

					log_info(loggerKernel, "Se elimino el proceso con pid: %d\n", procesoKernel->PID);

   				}
				// Si el recurso existe
				if (recursoEncontrado != NULL) {
					recursoEncontrado->instancias++;
					// busco si hay algún proceso esperando el recurso
					for (int i=0; i<list_size(lista_BLOCKED_RECURSOS); i++) {
						PCB *procesoBloqueado = list_get(lista_BLOCKED_RECURSOS, i);
						if (strcmp(procesoBloqueado->recursoBloqueante, recursoRecibido) == 0) {
							// lo muevo a ready
							pthread_mutex_lock(&mutexListaBlockedRecursos);
							pthread_mutex_lock(&mutexListaReady);
							procesoKernel = list_remove(lista_BLOCKED_RECURSOS, i);
							procesoKernel->estado = READY;
							list_add(lista_READY, procesoKernel);
							pthread_mutex_unlock(&mutexListaBlockedRecursos);
							pthread_mutex_unlock(&mutexListaReady);
							sem_post(&semListaReady);
							printf("Proceso %d desbloqueado por señal de recurso %s\n", procesoKernel->PID, recursoRecibido);
						}
					}
					enviar_resultado_recursos(SIGNAL_SUCCESS, cpu_dispatch_fd);
				}
				break;
			}
			case RESIZE_ERROR:
			{
				break;
			}
			// INSTRUCCIONES I/O
			case IO_GEN_SLEEP:
			{
				sem_wait(&semPlaniRunning);
				sem_post(&semPlaniRunning);
				procesoCPU = recibirProcesoContextoEjecucion(stream);
				stream += 8*sizeof(uint32_t) + 4*sizeof(uint8_t); // como recibe mas cosas se suma el el registri DI
				sem_wait(&semPlaniBlocked);
				sem_post(&semPlaniBlocked);
				procesoKernel = desalojarProceso(procesoKernel, procesoCPU);
				log_info(loggerKernel, "El proceso con pid:%d se interrumpio por IO_GEN_SLEEP\n", procesoKernel->PID);

				Peticion_Interfaz_Generica interfazGenerica;
				Tipos_Interfaz tipoDeInterfazEncontrada;
				int pathLength;
				
				memcpy(&pathLength, stream, sizeof(uint32_t));
				stream+=sizeof(uint32_t);
				interfazGenerica.nombre_interfaz = malloc(pathLength);
				memcpy(interfazGenerica.nombre_interfaz, stream, pathLength);
				stream+=pathLength;
				memcpy(&interfazGenerica.unidades_de_trabajo, stream, sizeof(uint32_t));
				


				interfazGenerica.PID=procesoKernel->PID;
				//MUTEX
				int socketClienteInterfaz = existeInterfaz(interfazGenerica.nombre_interfaz,&tipoDeInterfazEncontrada);
				if (tipoDeInterfazEncontrada!=T_GENERICA){
					//error no es una intruccion compatible //TODO
				}
				sem_wait(&semIOGEN);
				if(socketClienteInterfaz){
					t_paquete* paqueteIOGen=crear_paquete(IO_GEN_SLEEP);
					agregar_a_paquete(paqueteIOGen,&interfazGenerica.unidades_de_trabajo,sizeof(int));
					agregar_a_paquete(paqueteIOGen,&interfazGenerica.PID,sizeof(int));
					agregar_a_paquete(paqueteIOGen,interfazGenerica.nombre_interfaz,pathLength);					
					enviar_paquete(paqueteIOGen,socketClienteInterfaz);
					eliminar_paquete(paqueteIOGen);
					bloquearProceso(procesoKernel);
				}
				else{
					terminarProceso(procesoKernel);
				}
				free(interfazGenerica.nombre_interfaz);
				break;
			}

			case IO_STDIN_READ:
			{
				procesoCPU = recibirProcesoContextoEjecucion(stream);
				stream += 8*sizeof(uint32_t) + 4*sizeof(uint8_t); // como recibe mas cosas se suma el el registri DI
				procesoKernel = desalojarProceso(procesoKernel, procesoCPU);
				log_info(loggerKernel, "El proceso con pid:%d se interrumpio por IO_GEN_SLEEP\n", procesoKernel->PID);

				Peticion_Interfaz_STDIN interfazsSTDIN;
				Tipos_Interfaz tipoDeInterfazEncontrada;
				int pathLength;
				char* registroAleer;
				int registroLen;

				memcpy(&pathLength, stream, sizeof(uint32_t));
				stream+=sizeof(uint32_t);
				interfazsSTDIN.nombre_interfaz = malloc(pathLength);
				memcpy(interfazsSTDIN.nombre_interfaz, stream, pathLength);
				stream+=pathLength;
				//primer registro
				memcpy(&registroLen, stream, sizeof(uint32_t));
				stream+=sizeof(uint32_t);
				registroAleer=malloc(registroLen);
				memcpy(registroAleer, stream, registroLen);
				stream += registroLen;
				interfazsSTDIN.direccion=leerValorDelRegistro(registroAleer,procesoKernel->cpuRegisters);
				free(registroAleer);
				//segundoRegistro
				memcpy(&registroLen, stream, sizeof(uint32_t));
				stream+=sizeof(uint32_t);
				registroAleer=malloc(registroLen);
				memcpy(registroAleer, stream, registroLen);
				interfazsSTDIN.tamanio=leerValorDelRegistro(registroAleer,procesoKernel->cpuRegisters);
				free(registroAleer);

				interfazsSTDIN.PID=procesoKernel->PID;

				//MUTEX
				int socketClienteInterfaz = existeInterfaz(interfazsSTDIN.nombre_interfaz,&tipoDeInterfazEncontrada);
				if (tipoDeInterfazEncontrada!=T_STDIN){
					//error no es una intruccion compatible //TODO
				}
				
				if(socketClienteInterfaz){
					t_paquete* paqueteIOSTDIN=crear_paquete(IO_STDIN_READ);
					agregar_entero_a_paquete32(paqueteIOSTDIN,interfazsSTDIN.direccion);
					agregar_entero_a_paquete32(paqueteIOSTDIN,interfazsSTDIN.tamanio);
					agregar_entero_a_paquete32(paqueteIOSTDIN,interfazsSTDIN.PID);
					agregar_a_paquete(paqueteIOSTDIN,interfazsSTDIN.nombre_interfaz,pathLength);					
					enviar_paquete(paqueteIOSTDIN,socketClienteInterfaz);
					eliminar_paquete(paqueteIOSTDIN);
					
					bloquearProceso(procesoKernel);
				}
				else{
					terminarProceso(procesoKernel);
					//eliminarProceso(proceso); //TODO
				}
				free(interfazsSTDIN.nombre_interfaz);

				break;
			}
			case IO_STDOUT_WRITE:
			{
				/*procesoCPU = recibirProcesoContextoEjecucion(stream);
				if(!strcmp(configuracionKernel.ALGORITMO_PLANIFICACION, "RR") || !strcmp(configuracionKernel.ALGORITMO_PLANIFICACION, "VRR")){
					terminarHiloQuantum();
				}
				pthread_mutex_lock(&mutexListaRunning);
				pthread_mutex_lock(&mutexListaBlocked);
				procesoKernel = list_remove(lista_RUNNING, 0);
				actualizarProceso(procesoCPU, procesoKernel);
				if(!strcmp(configuracionKernel.ALGORITMO_PLANIFICACION, "VRR")){
					temporal_stop(tiempoVRR);
					tiempoEjecutando = temporal_gettime(tiempoVRR);
					temporal_destroy(tiempoVRR);
					procesoKernel->quantum -= tiempoEjecutando;
				}
				procesoKernel->estado = BLOCKED;
				list_add(lista_BLOCKED, procesoKernel);
				pthread_mutex_unlock(&mutexListaRunning);
				pthread_mutex_unlock(&mutexListaBlocked);
				sem_post(&semListaRunning);
				log_info(loggerKernel, "El proceso con pid:%d se interrumpio por IO_GEN_SLEEP\n", procesoKernel->PID);
				*/ 
				//ESTO ESTA COPYPASTEADO NO SI ESTA BN
				Peticion_Interfaz_STDOUT interfazsSTDOUT;
				Tipos_Interfaz tipoDeLaInterfaz;
				PCB* proceso;

				int pathLength;
				
				
				memcpy(&interfazsSTDOUT.direccion, stream, sizeof(uint32_t));
				stream += sizeof(uint32_t);
				memcpy(&interfazsSTDOUT.tamanio, stream, sizeof(uint8_t));
				stream += sizeof(uint8_t);
				memcpy(&interfazsSTDOUT.PID, stream, sizeof(int));
				stream += sizeof(int);
				memcpy(&pathLength, stream, sizeof(int));
				interfazsSTDOUT.nombre_interfaz = malloc(pathLength);
				memcpy(interfazsSTDOUT.nombre_interfaz, stream, pathLength);

				//MUTEX
				if(existeInterfaz(interfazsSTDOUT.nombre_interfaz,&tipoDeLaInterfaz)){
					t_paquete* paqueteIOSTDOUT=crear_paquete(IO_GEN_SLEEP);
					agregar_a_paquete(paqueteIOSTDOUT,&interfazsSTDOUT.direccion,sizeof(uint32_t));
					agregar_a_paquete(paqueteIOSTDOUT,&interfazsSTDOUT.tamanio,sizeof(uint8_t));
					agregar_a_paquete(paqueteIOSTDOUT,&interfazsSTDOUT.PID,sizeof(int));
					agregar_a_paquete(paqueteIOSTDOUT,interfazsSTDOUT.nombre_interfaz,pathLength);					
					enviar_paquete(paqueteIOSTDOUT,socketCliente);
					eliminar_paquete(paqueteIOSTDOUT);
					
					//bloquear procesos? //TODO
				}
				else{
					PCB* proceso = cambiarAExitDesdeRunning(lista_RUNNING);
					paquete_memoria_finalizar_proceso(proceso->PID);
					//eliminarProceso(proceso); //TODO
				}
				free(interfazsSTDOUT.nombre_interfaz);

				break;
			}
			case IO_FS_CREATE:
			{
				break;
			}
			case IO_FS_DELETE:
			{
				break;
			}
			case IO_FS_TRUNCATE:
			{
				break;
			}
			case IO_FS_WRITE:
			case IO_FS_READ:
			{
				break;
			}
				
			default:
			{
				//ningun mensaje valido recibido
				break;
			}
			eliminar_paquete(paquete);
		}
		//free(procesoCPU);
		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);
	}
}

int existeInterfaz(char *nombre,Tipos_Interfaz* tipo){
	//mutex lista
	Interfaces_conectadas_kernel *interfazBuffer;
	if(list_size(interfacesConectadas) != 0){
		for (int i = 0; i < list_size(interfacesConectadas); i++)
		{
			interfazBuffer=list_get(interfacesConectadas, i);
			if(!strcmp(interfazBuffer->nombre, nombre)){
				*tipo= interfazBuffer->tipo;
				return interfazBuffer->socketCliente;
			}
		}
	}
	
	return 0;
}
PCB* procesoBloqueado(uint32_t pid){
	pthread_mutex_lock(&mutexListaBlocked);
	PCB *proceso;
	if(list_size(lista_BLOCKED) != 0){
		for (int i = 0; i < list_size(lista_BLOCKED); i++)
		{
			proceso=list_get(lista_BLOCKED, i);
			if(proceso->PID == pid){
				proceso = list_remove(lista_BLOCKED, i);
				pthread_mutex_unlock(&mutexListaBlocked);
				return proceso;
			}
		}
	}
	pthread_mutex_unlock(&mutexListaBlocked);
	return NULL;
}

void* atenderPeticionesIO() {
    while (1) {
        int socketCliente = esperarClienteV2(loggerKernel, server_fd);
        pthread_t client_thread;
        int* pclient = malloc(sizeof(int));
        *pclient = socketCliente;
        pthread_create(&client_thread, NULL, manejarClienteIO, pclient);
        pthread_detach(client_thread);
    }
    return NULL;
}

void* manejarClienteIO(void *arg)
{
    int socketCliente = *((int*)arg);
    free(arg);
    while(1){
        t_paquete* paquete=recibirPaquete(socketCliente);
		void *stream = paquete->buffer->stream;
		switch(paquete->codigo_operacion){
        	case AGREGAR_INTERFACES:
            {
				Interfaces_conectadas_kernel *interfazBuffer = malloc(sizeof(Interfaces_conectadas_kernel));
				int charTam;
				memcpy(&charTam, stream, sizeof(int));
				stream += sizeof(int);
				interfazBuffer->nombre = malloc(charTam);
				memcpy(interfazBuffer->nombre,stream, charTam);
				stream += charTam;
				stream += sizeof(int);
				memcpy(&interfazBuffer->tipo, stream , sizeof(Tipos_Interfaz));
				interfazBuffer->socketCliente = socketCliente;
				list_add(interfacesConectadas, interfazBuffer);
				//TODO agregar q si se vuelve a conectar una interfaz
				log_info(loggerKernel, "Se conecto y guardo la interfaz con nombre:%s",interfazBuffer->nombre);
				
				break;
			}
			case DESBLOQUEAR_PROCESO_POR_IO:
			{
				//recibo nombre de la interfaz para sacarlo de la lista y pasarlo de bloqueado a ready
				sem_post(&semIOGEN);
				char *nombre;
				int charTam;
				uint32_t pid;
				PCB* proceso =NULL;
				memcpy(&charTam, stream, sizeof(int));
				stream += sizeof(int);
				nombre = malloc(charTam);
				memcpy(nombre,stream, charTam);
				stream += charTam;

				memcpy(&pid,stream,sizeof(uint32_t));
				proceso=procesoBloqueado(pid);
				sem_wait(&semPlaniBlocked);
				sem_post(&semPlaniBlocked);
				/*if(proceso == NULL){
					//logger
				}*/

				if(!strcmp(configuracionKernel.ALGORITMO_PLANIFICACION, "VRR"))
				{
					if(proceso->quantum < configuracionKernel.QUANTUM && proceso->quantum > 0){
						proceso->estado = READY;
						pthread_mutex_lock(&mutexListaReadyPri);
						list_add(lista_READYPRI, proceso);
						pthread_mutex_unlock(&mutexListaReadyPri);
						sem_post(&semListaReady);
					}
					
				}
				else{
					proceso->estado = READY;
					pthread_mutex_lock(&mutexListaReady);
					list_add(lista_READY, proceso);
					pthread_mutex_unlock(&mutexListaReady);
					sem_post(&semListaReady);
				}
				break;
			}
			case ERROR_EN_INTERFAZ:
			{
				break;
			}
			default:
			{
				//ningun mensaje valido recibido
				break;
			}
		}
	}
}

////////////////////////////////////////////////////////// EMPAQUETACION //////////////////////////////////////////////////////////

PCB *recibirProcesoContextoEjecucion(void *stream){
	PCB* proceso = malloc(sizeof(PCB));
	proceso->estado = RUNNING;
	memcpy(&proceso->PID, stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&proceso->cpuRegisters.PC, stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&proceso->cpuRegisters.AX, stream, sizeof(uint8_t));
	stream += sizeof(uint8_t);
	memcpy(&proceso->cpuRegisters.BX, stream, sizeof(uint8_t));
	stream += sizeof(uint8_t);
	memcpy(&proceso->cpuRegisters.CX, stream, sizeof(uint8_t));
	stream += sizeof(uint8_t);
	memcpy(&proceso->cpuRegisters.DX, stream, sizeof(uint8_t));
	stream += sizeof(uint8_t);
	memcpy(&proceso->cpuRegisters.EAX, stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&proceso->cpuRegisters.EBX, stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&proceso->cpuRegisters.ECX, stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&proceso->cpuRegisters.EDX, stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&proceso->cpuRegisters.SI, stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&proceso->cpuRegisters.DI, stream, sizeof(uint32_t));


	return proceso;
}

void paquete_CPU_ejecutar_proceso(PCB* proceso){
    t_paquete *paquete_CPU = crear_paquete(EJECUTAR_PROCESO);

    agregar_entero_a_paquete32(paquete_CPU, proceso->PID);
    agregar_entero_a_paquete32(paquete_CPU, proceso->cpuRegisters.PC);
    agregar_entero_a_paquete8(paquete_CPU, proceso->cpuRegisters.AX);
    agregar_entero_a_paquete8(paquete_CPU, proceso->cpuRegisters.BX);
    agregar_entero_a_paquete8(paquete_CPU, proceso->cpuRegisters.CX);
    agregar_entero_a_paquete8(paquete_CPU, proceso->cpuRegisters.DX);
    agregar_entero_a_paquete32(paquete_CPU, proceso->cpuRegisters.EAX);
    agregar_entero_a_paquete32(paquete_CPU, proceso->cpuRegisters.EBX);
    agregar_entero_a_paquete32(paquete_CPU, proceso->cpuRegisters.ECX);
    agregar_entero_a_paquete32(paquete_CPU, proceso->cpuRegisters.EDX);
    agregar_entero_a_paquete32(paquete_CPU, proceso->cpuRegisters.SI);
    agregar_entero_a_paquete32(paquete_CPU, proceso->cpuRegisters.DI);

    enviar_paquete(paquete_CPU, cpu_dispatch_fd);
    eliminar_paquete(paquete_CPU);
}

void paquete_CPU_interrumpir_proceso_fin_quantum(int pid){
    t_paquete *paquete_CPU = crear_paquete(PROCESO_INTERRUMPIDO_CLOCK);

    agregar_entero_a_paquete32(paquete_CPU, pid);
    enviar_paquete(paquete_CPU, cpu_interrupt_fd);
    eliminar_paquete(paquete_CPU);
}

void InterruptACPU(){
    t_paquete *paquete_CPU_interrupcion = crear_paquete(INTERRUMPIR_PROCESO);

    enviar_paquete(paquete_CPU_interrupcion, cpu_interrupt_fd);
    eliminar_paquete(paquete_CPU_interrupcion);
}


void enviar_resultado_recursos(op_code resultadoRecursos,int socket_cliente){

    t_paquete *paquete_cpu_recursos = crear_paquete(resultadoRecursos);
    
    enviar_paquete(paquete_cpu_recursos, socket_cliente);

    eliminar_paquete(paquete_cpu_recursos);
}

PCB* desalojarProceso(PCB* procesoKernel, PCB* procesoCPU){
	if(!strcmp(configuracionKernel.ALGORITMO_PLANIFICACION, "RR") || !strcmp(configuracionKernel.ALGORITMO_PLANIFICACION, "VRR")){
		terminarHiloQuantum();
	}
	pthread_mutex_lock(&mutexListaRunning);
	procesoKernel = list_remove(lista_RUNNING, 0);
	actualizarProceso(procesoCPU, procesoKernel);
	if(!strcmp(configuracionKernel.ALGORITMO_PLANIFICACION, "VRR")){
		procesoKernel->quantum -= tiempoEjecutando;
	}
	pthread_mutex_unlock(&mutexListaRunning);
	sem_post(&semListaRunning);

	return procesoKernel;
}

void bloquearProceso(PCB* proceso){
	pthread_mutex_lock(&mutexListaBlocked);
	proceso->estado = BLOCKED;
	list_add(lista_BLOCKED, proceso);
	pthread_mutex_unlock(&mutexListaBlocked);
}

void terminarProceso(PCB* proceso){
	paquete_memoria_finalizar_proceso(proceso->PID);
	proceso->estado = EXIT;
	pthread_mutex_lock(&mutexListaExit);
	list_add(lista_EXIT, proceso);
	pthread_mutex_unlock(&mutexListaExit);
}