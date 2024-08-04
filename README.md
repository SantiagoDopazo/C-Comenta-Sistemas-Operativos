# C-COMENTA:

Enunciado : [Enunciado del Trabajo Práctico](https://docs.google.com/document/d/1-AqFTroovEMcA1BfC2rriB5jsLE6SUa4mbcAox1rPec/edit)

## OBJETIVOS

El objetivo del trabajo práctico consiste en desarrollar una solución que permita la simulación de un sistema distribuido, donde los grupos tendrán que planificar procesos, resolver peticiones al sistema y administrar de manera adecuada una memoria y un sistema de archivos bajo los esquemas explicados en sus correspondientes módulos.

### Mediante la realización de este trabajo se espera que el alumno:
* Adquiera conceptos prácticos del uso de las distintas herramientas de programación e interfaces (APIs) que brindan los sistemas operativos.
* Entienda aspectos del diseño de un sistema operativo.
* Afirme diversos conceptos teóricos de la materia mediante la implementación práctica de algunos de ellos.
* Se familiarice con técnicas de programación de sistemas, como el empleo de makefiles, archivos de configuración y archivos de log.
* Conozca con grado de detalle la operatoria de Linux mediante la utilización de un lenguaje de programación de relativamente bajo nivel como C.


## Compilación

Cada módulo del proyecto se compila de forma independiente a través de un
archivo `makefile`. Para compilar un módulo, es necesario ejecutar el comando
`make` desde la carpeta correspondiente.

El ejecutable resultante se guardará en la carpeta `bin` del módulo.

## Importar desde Visual Studio Code

Para importar el workspace, debemos abrir el archivo `tp.code-workspace` desde
la interfaz o ejecutando el siguiente comando desde la carpeta raíz del
repositorio:

```bash
code tp.code-workspace
```
## Deploy
Para desplegar el proyecto en una máquina Ubuntu Server, podemos utilizar el
script [so-deploy] de la cátedra:

```bash
git clone https://github.com/sisoputnfrba/so-deploy.git
cd so-deploy
./deploy.sh -r=release -p=utils -p=kernel -p=cpu -p=memoria -p=entradasalida "tp-{año}-{cuatri}-{grupo}"
```

El mismo se encargará de instalar las Commons, clonar el repositorio del grupo
y compilar el proyecto en la máquina remota.
### Especificaciones
Cada módulo puede ser levantado en distintas computadoras en la misma red local, hay que cambiar las IP de los archivos de configuración según corresponda.

## Pruebas
Documento con las pruebas para probar distintas funcionalidades de la simulación del sistema distribuido: [Documento de Pruebas](https://docs.google.com/document/d/1XsBsJynoN5A9PTsTEaZsj0q3zsEtcnLgdAHOQ4f_4-g/edit)

## Bibliotecas usadas
### Funciones proporcionadas por la Cátedra de la Materia:
* [so-commons-library](https://github.com/sisoputnfrba/so-commons-library)
### Bibliotecas estándar de C:
* [stdio](https://man7.org/linux/man-pages/man3/stdio.3.html)
* [stdlib](https://man7.org/linux/man-pages/man0/stdlib.h.0p.html)
* [stdbool](https://man7.org/linux/man-pages/man0/stdbool.h.0p.html)
* [string](https://man7.org/linux/man-pages/man3/string.3.html)
* [assert](https://man7.org/linux/man-pages/man3/assert.3.html)
### POSIX y otras:
* [pthread](https://man7.org/linux/man-pages/man7/pthreads.7.html)
* [semaphore](https://man7.org/linux/man-pages/man0/semaphore.h.0p.html)
* [unistd](https://man7.org/linux/man-pages/man0/unistd.h.0p.html)
* [readline](https://man7.org/linux/man-pages/man3/readline.3.html)
* [sys/types](https://man7.org/linux/man-pages/man0/sys_types.h.0p.html)
* [sys/socket](https://man7.org/linux/man-pages/man2/socket.2.html)
* [netdb](https://man7.org/linux/man-pages/man0/netdb.h.0p.html)
* [fcntl](https://man7.org/linux/man-pages/man2/fcntl.2.html)
* [sys/mman](https://man7.org/linux/man-pages/man2/mmap.2.html)
* [sys/stat](https://man7.org/linux/man-pages/man2/stat.2.html)
* [dirent](https://man7.org/linux/man-pages/man0/dirent.h.0p.html)

