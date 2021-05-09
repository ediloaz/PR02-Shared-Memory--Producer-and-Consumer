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

//Para compilar
//gcc consumidor.c -o consumidor -lm -lpthread -lrt

struct auxiliar_t{
    int index_lectura;		//Índice de lectura
    int index_escritura;	//Índice de escritura
    int max_buffer;		//Tamaño máximo de capacidad del buffer

    sem_t SEM_CONSUMIDORES; 	//semáforo de total de consumidores vivos
    sem_t SEM_PRODUCTORES; 	//semáforo de total de productores vivos

    sem_t SEM_LLENO;	     	//semáforo de buffer lleno
    sem_t SEM_VACIO;		//semáforo de buffer vacío
    sem_t SEM_BUFFER;		//semáforo de acceso a buffer

    int PRODUCTORES;		//total de productores vivos
    int CONSUMIDORES;		//total de consumidores vivos

    char mensaje_log[LOGMAX];
} ;

struct auxiliar_t* auxptr;

char (*bufptr)[ENTRYMAX];



//Variables globales
int contadorMensajes = 0;
double contadorTiempoEspera = 0;
double contadorTiempoBloqueado = 0;
int pid;
clock_t tiempoEspera;
clock_t tiempoBloqueado;
pid_t pidCreator;

int paramIndex = 1;
int bufferSize = 10;



double ran_expo(double lambda)
{
    double u;
    u = rand() / (RAND_MAX + 1.0);
    double exp = -lambda * log(u);
    return exp;
}


//Flag del productores
int flag_productor = 0;
//***Hay que usar memoria compartida para la bandera (?)***






int main(int argc, char **argv)
{
  int flag_productor = 0;
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

  char msgBitacora[LOGMAX];
  while(flag_productor){


    printf("Productor(%d) empieza.\n", getpid());

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

    //Pide semáforo de PRODUCTORES
    sem_wait(&auxptr->SEM_PRODUCTORES);
    tiempoBloqueado = clock() - tiempoBloqueado;
    contadorTiempoBloqueado += ((double)tiempoBloqueado)/CLOCKS_PER_SEC;

    auxptr->PRODUCTORES++;
    sem_post(&auxptr->SEM_PRODUCTORES);

    sprintf(msgBitacora, "El productor (%d) fue creado", pid);
    strcpy(&auxptr->mensaje_log[0], msgBitacora);
    kill(pidCreator, SIGUSR2);


    //Dormir
    double sleepTime = ran_expo(media);
    tiempoEspera = clock();
    sprintf(msgBitacora, "El productor (%d) va a dormir", pid);
    strcpy(&auxptr->mensaje_log[0], msgBitacora);
    kill(pidCreator, SIGINT);
    sleep(sleepTime);
    tiempoEspera = clock() - tiempoEspera;
    contadorTiempoEspera += ((double)tiempoEspera)/CLOCKS_PER_SEC + sleepTime;
    sprintf(msgBitacora, "El productor (%d) se despertó", pid);
    strcpy(&auxptr->mensaje_log[0], msgBitacora);
    kill(pidCreator, SIGINT);

    //Semaforo Vacio
    sprintf(msgBitacora, "El productor (%d) va a pedir el semáforo de lleno", pid);
    strcpy(&auxptr->mensaje_log[0], msgBitacora);
    kill(pidCreator, SIGINT);
    tiempoBloqueado = clock();
    sem_wait(&auxptr->SEM_LLENO);
    tiempoBloqueado = clock() - tiempoBloqueado;
    contadorTiempoBloqueado += ((double)tiempoBloqueado)/CLOCKS_PER_SEC;
    sprintf(msgBitacora, "El productor (%d) recibió el semáforo de lleno", pid);
    strcpy(&auxptr->mensaje_log[0], msgBitacora);
    kill(pidCreator, SIGINT);

    //Semaforo Buffer
    sprintf(msgBitacora, "El productor (%d) va a pedir el semáforo del buffer", pid);
    strcpy(&auxptr->mensaje_log[0], msgBitacora);
    kill(pidCreator, SIGINT);
    tiempoBloqueado = clock();
    sem_wait(&auxptr->SEM_BUFFER);
    tiempoBloqueado = clock() - tiempoBloqueado;
    contadorTiempoBloqueado += ((double)tiempoBloqueado)/CLOCKS_PER_SEC;
    sprintf(msgBitacora, "El productor (%d) recibió el semáforo del buffer", pid);
    strcpy(&auxptr->mensaje_log[0], msgBitacora);
    kill(pidCreator, SIGINT);


    //Generación de mensajes
    //Calcular llave
    //rand() % (max_number + 1 - minimum_number) + minimum_number
    int llave = rand() % (5);
    //Calculamos el tiempo
    time_t t;
    time(&t);
    //Creamos el mensaje
    char mensaje[64];
    sprintf(mensaje,"%s", "Llave: ");
    sprintf(mensaje, "%d", llave);
    sprintf(mensaje,"%s", "Identificador: ");
    sprintf(mensaje, "%d", pid);
    sprintf(mensaje, "%s", "Tiempo: ");
    sprintf(mensaje, "%s", ctime(&t));

    //Añadimos mensaje al buffer
    strcpy(bufptr[auxptr->index_escritura], mensaje);

    sprintf(msgBitacora, "El productor (%d) escribió en el índice %d el mensaje: %s", pid, auxptr->index_lectura, mensaje);

    //Aumentamos indice index_escritura
    auxptr->index_escritura = (auxptr->index_escritura + 1) % bufferSize;

    strcpy(&auxptr->mensaje_log[0], msgBitacora);
    kill(pidCreator, SIGALRM);

    //Devolver semáforos

    kill(pidCreator, SIGALRM);
    sem_post(&auxptr->SEM_BUFFER);
    sprintf(msgBitacora, "El productor (%d) liberó el semáforo del buffer", pid);
    strcpy(&auxptr->mensaje_log[0], msgBitacora);
    kill(pidCreator, SIGINT);

    sem_post(&auxptr->SEM_LLENO);
    sprintf(msgBitacora, "El productor (%d) liberó el semáforo de vacio", pid);
    strcpy(&auxptr->mensaje_log[0], msgBitacora);
    kill(pidCreator, SIGINT);

    printf("Productor(%d) envió mensaje, con el indice de entrada: %d. Productores vivos: %d, consumidores vivos: %d.\n", pid, auxptr->index_escritura, auxptr->PRODUCTORES, auxptr->CONSUMIDORES);

    contadorMensajes++;

  }

  if (!flag_productor) {
    tiempoBloqueado = clock();
    sem_wait(&auxptr->SEM_PRODUCTORES);
    tiempoBloqueado = clock() - tiempoBloqueado;
    contadorTiempoBloqueado += ((double)tiempoBloqueado)/CLOCKS_PER_SEC;
    //Decrementar el contador de productores vivos
    auxptr->PRODUCTORES++;
    //Devolver el semaforo de contador de productores vivos
    sem_post(&auxptr->SEM_PRODUCTORES);

    printf("Productor(%d) termina con %d mensajes, %f segundos esperando y %f segundos bloqueado.\n", pid, contadorMensajes, contadorTiempoEspera, contadorTiempoBloqueado);
    sprintf(msgBitacora, "El consumidor (%d) murió", pid);
    strcpy(&auxptr->mensaje_log[0], msgBitacora);
    kill(pidCreator, SIGUSR1);

    return 0;
  }
//productor(nombreBuffer,media);

}
