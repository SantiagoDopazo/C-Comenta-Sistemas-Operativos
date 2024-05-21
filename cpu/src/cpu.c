#include "../include/cpu.h"

void cargar_configuracion(char* archivo_configuracion)
{
	t_config* config = config_create(archivo_configuracion); //Leo el archivo de configuracion

	if (config == NULL) {
		perror("Archivo de configuracion no encontrado");
		exit(-1);
	}

	config_valores.ip_memoria =	config_get_string_value(config,    "IP_MEMORIA");
	config_valores.puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
	config_valores.puerto_escucha_dispatch = config_get_string_value(config,    "PUERTO_ESCUCHA_DISPATCH");
	config_valores.puerto_escucha_interrupt = config_get_string_value(config,    "PUERTO_ESCUCHA_INTERRUPT");
	config_valores.cant_enradas_tlb = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
	config_valores.algoritmo_tlb = config_get_string_value(config, "ALGORITMO_TLB");
	
	
}

int main(int argc, char* argv[]) {
    
    logger = log_create("./log/cpu.log", "CPU", true, LOG_LEVEL_INFO);
	log_info(logger, "Se creo el log!");

	cargar_configuracion("/home/utnso/tp-2024-1c-File-System-Fanatics/cpu/cpu.config");

	//me conecto a memoria
	memoria_fd = crear_conexion(logger,"CPU",config_valores.ip_memoria, config_valores.puerto_memoria);
	log_info(logger, "Me conecte a memoria!");

	//envio mensaje
    enviar_mensaje("Hola, soy CPU!", memoria_fd);

	// levanto el servidor dispatch e interrupt
	fd_cpu_dispatch = iniciar_servidor(logger,server_name_dispatch ,IP, config_valores.puerto_escucha_dispatch);
    fd_cpu_interrupt = iniciar_servidor(logger, server_name_interrupt ,IP, config_valores.puerto_escucha_interrupt);
	log_info(logger, "Servidor listo para recibir al cliente");

	// espero mensjaes de kernel
    while(server_escuchar(fd_cpu_interrupt, fd_cpu_dispatch));



    return EXIT_SUCCESS;
}


static void procesar_conexion_interrupt(void* void_args) {
    int *args = (int*) void_args;
	int cliente_socket_interrupt = *args;

    op_code cop;
	while (cliente_socket_interrupt != -1) {
		if (recv(cliente_socket_interrupt, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
			log_info(logger, ANSI_COLOR_BLUE"El cliente se desconecto de INTERRUPT");
			return;
		}
        switch (cop) {
            case MENSAJE:
                char* mensaje = recibir_mensaje(cliente_socket_interrupt);
				log_info(logger, "Recibi el mensaje: %s , soy interrupt", mensaje);
                break;
            default: {
                log_error(logger, "Código de operación no reconocido en Interrupt: %d", cop);
                break;
            }
        }
    }
}

static void procesar_conexion_dispatch(void* void_args) {
    int *args = (int*) void_args;
	int cliente_socket_dispatch = *args;

    op_code cop;
	while (cliente_socket_dispatch != -1 && !recibio_interrupcion) {
        if (recv(cliente_socket_dispatch, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
			log_info(logger, ANSI_COLOR_BLUE"El cliente se desconecto de DISPATCH");
			return;
		}
        switch (cop) {
            case MENSAJE:
				char* mensaje = recibir_mensaje(cliente_socket_dispatch);
				log_info(logger, "Recibi el mensaje: %s , soy dispatch", mensaje);
                break;
		    
            default: {
                log_error(logger, "Código de operación no reconocido en Dispatch: %d", cop);
                break;
            }
        }
    }
    return;
}

int server_escuchar(int fd_cpu_interrupt, int fd_cpu_dispatch) {
	 int socket_cliente_dispatch = esperar_cliente(logger, server_name_dispatch, fd_cpu_dispatch);
	int socket_cliente_interrupt = esperar_cliente(logger, server_name_interrupt, fd_cpu_interrupt);
	
    if (socket_cliente_interrupt != -1 && socket_cliente_dispatch != -1) {
        //hilo para servidor dispatch
        pthread_t hilo_dispatch;
		pthread_create(&hilo_dispatch, NULL, (void*) procesar_conexion_dispatch, (void*) &socket_cliente_dispatch);
		pthread_detach(hilo_dispatch);
        //hilo para servidor interrupt
        pthread_t hilo_interrupt;
		pthread_create(&hilo_interrupt, NULL, (void*) procesar_conexion_interrupt, (void*) &socket_cliente_interrupt);
		pthread_detach(hilo_interrupt);
        return 1;
 	}
    return 0;
}




	

	
	