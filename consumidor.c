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

#define NAMEMAX 100

//gcc consumidor.c -o consumidor -lm

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

    void* ptrConsumidores = mmap(0, 4096, PROT_READ, MAP_SHARED, shm_open("CONSUMIDORES", O_RDWR, 0666), 0);
    void* ptrIndiceLectura = mmap(0, 4096, PROT_READ, MAP_SHARED, shm_open("INDICELECTURA", O_RDWR, 0666), 0);
    void* ptrProductores = mmap(0, 4096, PROT_READ, MAP_SHARED, shm_open("PRODUCTORES", O_RDWR, 0666), 0);
    void* ptrBuffer = mmap(0, 4096, PROT_READ, MAP_SHARED, shm_open("BUFFER", O_RDWR, 0666), 0);

    tiempoBloqueado = clock();
    sem_t *semconsumidores = sem_open("SEMCONSUMIDORES", O_CREAT, 0600, 0);
    tiempoBloqueado = clock() - tiempoBloqueado;
    contadorTiempoBloqueado += ((double)tiempoBloqueado)/CLOCKS_PER_SEC; 

    if (semconsumidores == SEM_FAILED) printf("SEMCONSUMIDORES Failed\n");     

    printf("%d", (char*)ptrConsumidores);
    int consumidores = atoi((char*)ptrConsumidores) + 1;
    memcpy(ptrConsumidores, consumidores, sizeof(consumidores));
    printf("%d", (char*)ptrConsumidores);
    
    sem_post(semconsumidores);
    kill(pidCreator, SIGUSR1);

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
}
