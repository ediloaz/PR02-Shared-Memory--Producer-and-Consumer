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
#define LOGMAX 100
#define ENTRYMAX 64
#define AUX "\auxiliar"
//gcc consumidorDummy.c -o consumidor -lm -lpthread -lrt

struct auxiliar_t{    
    int index_lectura;		//Índice de lectura
    int index_escritura;	//Índice de escritura
    int max_buffer;		//Tamaño máximo de capacidad del buffer
    
    sem_t SEM_CONSUMIDORES; 	//semáforo de total de consumidores vivos
    sem_t SEM_PRODUCTORES; 	//semáforo de total de productores vivos
    
    sem_t SEM_LLENO;	     	//semáforo de buffer lleno
    sem_t SEM_VACIO;		//semáforo de buffer vacío
    sem_t SEM_CBUFFER;		//semáforo de acceso a buffer de consumidores
    sem_t SEM_PBUFFER;		//Semáforo de acceso a buffer de productores
    
    int PRODUCTORES;		//total de productores vivos
    int CONSUMIDORES;		//total de consumidores vivos
    
    char mensaje_log[LOGMAX];
    char **BUFFER;
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
    char nombreBuffer[NAMEMAX] = "/nombreBuffer";
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
    bufptr = mmap(0,ENTRYMAX*bufferSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd,len+5);
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
    tiempoBloqueado = clock() - tiempoBloqueado;
    contadorTiempoBloqueado += ((double)tiempoBloqueado)/CLOCKS_PER_SEC;
    printf("Tiempo listo\n");

    /*=======ACTUALIZAR TOTAL CONSUMIDORES=======*/    
    sem_wait(&auxptr->SEM_CONSUMIDORES);//Pide semáforo de CONSUMIDORES

    auxptr->CONSUMIDORES++;
    
    sem_post(&auxptr->SEM_CONSUMIDORES);
    
    kill(pidCreator, SIGUSR1);//Envía señal a creador de actualización del valor
    
    /*=======ESCRIBIR EN EL BUFFER=======*/ 
    printf("Mensaje: %s\n", bufptr[0]);
    strcpy(bufptr[0], "Hola mundo");
    printf("Mensaje: %s\n", bufptr[0]);


    return 0;
}
/*
    while(1)
    {
        double sleepTime = ran_expo(media);
        tiempoEspera = clock();
        sleep(sleepTime);
        tiempoEspera = clock() - tiempoEspera;
        contadorTiempoEspera += ((double)tiempoEspera)/CLOCKS_PER_SEC + sleepTime;

        tiempoBloqueado = clock();
        sem_t *semvacio = sem_open("SEMVACIO", O_CREAT, 0600, 0);
        tiempoBloqueado = clock() - tiempoBloqueado;
        contadorTiempoBloqueado += ((double)tiempoBloqueado)/CLOCKS_PER_SEC;

        tiempoBloqueado = clock();
        sem_t *sembuffer = sem_open("SEMBUFFER", O_CREAT, 0600, 0);
        tiempoBloqueado = clock() - tiempoBloqueado;
        contadorTiempoBloqueado += ((double)tiempoBloqueado)/CLOCKS_PER_SEC;

        //Leer el índice de lectura
        printf("%d", (char*)ptrIndiceLectura);
        int indice = atoi((char*)ptrIndiceLectura);

        printf("%d", (char*)ptrIndiceLectura);

        //Leer el contador de productores vivos
        int contadorProductoresVivos = atoi((char*)ptrProductores);

        //Leer el contador de consumidores vivos
        int contadorConsumidoresVivos = atoi((char*)ptrConsumidores);

        //Leer buffer en indice CUIDADO
        char *mensaje = &ptrBuffer[indice];

        //Aumentar indice de lectura circular
        indice = (indice + 1) % bufferSize;
        memcpy(ptrIndiceLectura, indice, sizeof(indice));

        //TODO: Borrar el mensaje
        memcpy(&ptrBuffer[indice], "", sizeof(""));

        //Devolver semaforo de indice de lectura
        sem_post(sembuffer);
        sem_post(semvacio);

        kill(pidCreator, SIGALRM);

        printf("Consumidor(%d) lee mensaje, con el indice de entrada: %d. Productores vivos: %d, consumidores vivos: %d.\n", pid, indice, contadorProductoresVivos, contadorConsumidoresVivos);

        contadorMensajes++;

        int llave = atoi("5");
        llave = contadorMensajes;

        if (llave == 5 || pid % 5 == llave)
        {
            tiempoBloqueado = clock();
            sem_wait(semconsumidores);
            tiempoBloqueado = clock() - tiempoBloqueado;
            contadorTiempoBloqueado += ((double)tiempoBloqueado)/CLOCKS_PER_SEC;
            //Decrementar el contador de consumidores vivos
            int consumidores = atoi((char*)ptrConsumidores) - 1;
            memcpy(ptrConsumidores, consumidores, sizeof(consumidores));
            //Devolver el semaforo de contador de consumidores vivos
            sem_post(semconsumidores);

            printf("Consumidor(%d) termina con %d mensajes, %f segundos esperando y %f segundos bloqueado.\n", pid, contadorMensajes, contadorTiempoEspera, contadorTiempoBloqueado);
            return 0;
        }
    }
}*/
