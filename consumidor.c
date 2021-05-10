#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <semaphore.h>

#define NAMEMAX 100 		//tamaño máximo del numbre del buffer
#define LOGMAX 164
#define ENTRYMAX 64
#define AUX "\auxiliar"

//gcc consumidor.c -o consumidor -lm -lpthread -lrt

struct auxiliar_t{
    int index_lectura;		//Índice de lectura
    int index_escritura;	//Índice de escritura
    int max_buffer;		//Tamaño máximo de capacidad del buffer
    int flag_productor; //Flag del productor

    sem_t SEM_CONSUMIDORES; 	//semáforo de total de consumidores vivos
    sem_t SEM_PRODUCTORES; 	//semáforo de total de productores vivos
    sem_t SEM_FINALIZADOR;  //semáforo del finalizador

    sem_t SEM_LLENO;	     	//semáforo de buffer lleno
    sem_t SEM_VACIO;		//semáforo de buffer vacío
    sem_t SEM_BUFFER;		//semáforo de acceso a buffer
    sem_t SEM_BITACORA; //semáforo de acceso a la bitácora
    
    int PRODUCTORES;		//total de productores vivos
    int CONSUMIDORES;		//total de consumidores vivos


    char mensaje_log[LOGMAX];
} ;

struct auxiliar_t* auxptr;

char (*bufptr)[ENTRYMAX];

int contadorMensajes = 0;
double contadorTiempoEspera = 0;
double contadorTiempoBloqueado = 0;
int pid;
clock_t tiempoEspera;
clock_t tiempoBloqueado;

double ran_expo(double lambda)
{
    double u;
    u = rand() / (RAND_MAX + 1.0);
    double exp = -lambda * log(u);
    return exp;
}

int main(int argc, char **argv)
{
    pid = getpid();
    char nombreBuffer[NAMEMAX] = "nombreBuffer";
    int media = 3;

    int paramIndex = 1;
    int bufferSize = 10;

    pid_t pidCreator;

    while(paramIndex < argc)
    {
    	char* param = argv[paramIndex];
    	paramIndex++;

    	if(param[0] == '-'){
    	    switch(param[1]){
    	    	case 'n':
    	    	strcpy(nombreBuffer, argv[paramIndex]);
    	    	printf("Nombre: %s\n", nombreBuffer);
    	    	paramIndex++;
    	    	break;
    	    case 'm':
    	    	media = atoi(argv[paramIndex]);
    	    	printf("Media: %d\n", media);
    	    	paramIndex++;
    	    	break;
            case 'p':
    	    	pidCreator = atoi(argv[paramIndex]);
    	    	printf("pid Creator: %d\n", pidCreator);
    	    	paramIndex++;
    	    	break;
    	    default:
    	    	printf("Error: Parámetro no reconocido\n");
    	    	return 1;
    	    }
    	}
    }
    printf("Consumidor(%d) empieza.\n", pid);

    //=======Consigue direccion de memoria compartida=======
    int len = sizeof(struct auxiliar_t);
    int fd = shm_open(AUX, O_RDWR, 0600);
    if(fd == -1){
        printf("Fallo de shm_open\n");
        return 1;
    }
    if(ftruncate(fd, len) == -1) {
    	printf("Error de ftruncate\n");
        return 1;
    }
    struct auxiliar_t* auxptr = mmap(0, len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if(auxptr == MAP_FAILED){
    	printf("Error de mmap\n");
    	return 1;
    }
    bufferSize  = auxptr->max_buffer;
    printf("Tamaño de buffer: %d\n", auxptr->max_buffer);

    int fd2 = shm_open(nombreBuffer, O_RDWR, 0600);
    if(fd2 == -1){
    	printf("Fallo de shm_open\n");
    	return 1;
    }
    if(ftruncate(fd2, ENTRYMAX * bufferSize) == -1) {
    	printf("Error de ftruncate\n");
        return 1;
    }
    bufptr = mmap(0, ENTRYMAX * bufferSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd2, 0);

    //=======Tiempo bloqueado=======
    tiempoBloqueado = clock();

    //Pide semáforo de CONSUMIDORES
    sem_wait(&auxptr->SEM_CONSUMIDORES);
    tiempoBloqueado = clock() - tiempoBloqueado;
    contadorTiempoBloqueado += ((double)tiempoBloqueado)/CLOCKS_PER_SEC;
    auxptr->CONSUMIDORES++;
    sem_post(&auxptr->SEM_CONSUMIDORES);

    char msgBitacora[LOGMAX];
    sem_wait(&auxptr->SEM_BITACORA);
    sprintf(msgBitacora, "El consumidor (%d) fue creado", pid);
    strcpy(&auxptr->mensaje_log[0], msgBitacora);
    kill(pidCreator, SIGUSR1);

    while(1)
    {
        //Dormir
        double sleepTime = ran_expo(media);
        tiempoEspera = clock();

        sem_wait(&auxptr->SEM_BITACORA);
        sprintf(msgBitacora, "El consumidor (%d) va a dormir", pid);
        strcpy(&auxptr->mensaje_log[0], msgBitacora);
        kill(pidCreator, SIGVTALRM);

        sleep(sleepTime);
        tiempoEspera = clock() - tiempoEspera;
        contadorTiempoEspera += ((double)tiempoEspera)/CLOCKS_PER_SEC + sleepTime;

        sem_wait(&auxptr->SEM_BITACORA);
        sprintf(msgBitacora, "El consumidor (%d) se despertó", pid);
        strcpy(&auxptr->mensaje_log[0], msgBitacora);
        kill(pidCreator, SIGVTALRM);

        //Semaforo Vacio
        sem_wait(&auxptr->SEM_BITACORA);
        sprintf(msgBitacora, "El consumidor (%d) va a pedir el semáforo de lleno", pid);
        strcpy(&auxptr->mensaje_log[0], msgBitacora);
        kill(pidCreator, SIGVTALRM);

        tiempoBloqueado = clock();
        sem_wait(&auxptr->SEM_LLENO);
        tiempoBloqueado = clock() - tiempoBloqueado;
        contadorTiempoBloqueado += ((double)tiempoBloqueado)/CLOCKS_PER_SEC;

        sem_wait(&auxptr->SEM_BITACORA);
        sprintf(msgBitacora, "El consumidor (%d) recibió el semáforo de lleno", pid);
        strcpy(&auxptr->mensaje_log[0], msgBitacora);
        kill(pidCreator, SIGVTALRM);

        //Semaforo Buffer
        sem_wait(&auxptr->SEM_BITACORA);
        sprintf(msgBitacora, "El consumidor (%d) va a pedir el semáforo del buffer", pid);
        strcpy(&auxptr->mensaje_log[0], msgBitacora);
        kill(pidCreator, SIGVTALRM);

        tiempoBloqueado = clock();
        sem_wait(&auxptr->SEM_BUFFER);
        tiempoBloqueado = clock() - tiempoBloqueado;
        contadorTiempoBloqueado += ((double)tiempoBloqueado)/CLOCKS_PER_SEC;

        sem_wait(&auxptr->SEM_BITACORA);
        sprintf(msgBitacora, "El consumidor (%d) recibió el semáforo del buffer", pid);
        strcpy(&auxptr->mensaje_log[0], msgBitacora);
        kill(pidCreator, SIGVTALRM);

        //Leer mensaje
        char * mensaje = bufptr[auxptr->index_lectura];
        sem_wait(&auxptr->SEM_BITACORA);
        sprintf(msgBitacora, "El consumidor (%d) leyó en el í ndice %d el mensaje: %s", pid, auxptr->index_lectura, mensaje);

        //Aumentar indice de lectura circular
        auxptr->index_lectura = (auxptr->index_lectura + 1) % bufferSize;

        //Borrar el mensaje
        strcpy(bufptr[auxptr->index_lectura], "");

        strcpy(&auxptr->mensaje_log[0], msgBitacora);
        kill(pidCreator, SIGCHLD);

        //Devolver semaforo de buffer
        sem_post(&auxptr->SEM_BUFFER);
        sem_wait(&auxptr->SEM_BITACORA);
        sprintf(msgBitacora, "El consumidor (%d) liberó el semáforo del buffer", pid);
        strcpy(&auxptr->mensaje_log[0], msgBitacora);
        kill(pidCreator, SIGVTALRM);

        sem_post(&auxptr->SEM_VACIO);
        sem_wait(&auxptr->SEM_BITACORA);
        sprintf(msgBitacora, "El consumidor (%d) liberó el semáforo de vacio", pid);
        strcpy(&auxptr->mensaje_log[0], msgBitacora);
        kill(pidCreator, SIGVTALRM);

	    printf("MSG: %s", mensaje);

        printf("Consumidor(%d) lee mensaje, con el indice de entrada: %d. Productores vivos: %d, consumidores vivos: %d.\n", pid, auxptr->index_lectura, auxptr->PRODUCTORES, auxptr->CONSUMIDORES);

        contadorMensajes++;

        int llave = mensaje[7] - '0';

        if (llave == 5 || pid % 5 == llave)
        {
            tiempoBloqueado = clock();
            sem_wait(&auxptr->SEM_CONSUMIDORES);
            tiempoBloqueado = clock() - tiempoBloqueado;
            contadorTiempoBloqueado += ((double)tiempoBloqueado)/CLOCKS_PER_SEC;
            //Decrementar el contador de consumidores vivos
            auxptr->CONSUMIDORES--;

            //Devolver el semaforo de contador de consumidores vivos
            sem_post(&auxptr->SEM_CONSUMIDORES);

            printf("Consumidor(%d) termina con %d mensajes, %f segundos esperando y %f segundos bloqueado.\n", pid, contadorMensajes, contadorTiempoEspera, contadorTiempoBloqueado);

            sem_wait(&auxptr->SEM_BITACORA);
            sprintf(msgBitacora, "El consumidor (%d) murió", pid);
            strcpy(&auxptr->mensaje_log[0], msgBitacora);
            kill(pidCreator, SIGUSR1);

            //Avisa al finalizador
            sem_post(&auxptr->SEM_FINALIZADOR);


            return 0;
        }
    }
}
