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
//gcc productor.c -o productor -lm -lpthread -lrt

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



//Variables globales
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


//Flag del productores
//***Hay que usar memoria compartida para la bandera (?)***


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

  char msgBitacora[LOGMAX];

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
  sem_wait(&auxptr->SEM_BITACORA);
  sprintf(msgBitacora, "El productor (%d) fue creado", pid);
  strcpy(&auxptr->mensaje_log[0], msgBitacora);
  kill(pidCreator, SIGUSR2);

  printf("Productor(%d) empieza.\n", getpid());

  while(auxptr->flag_productor){

    //Dormir
    double sleepTime = ran_expo(media);
    tiempoEspera = clock();
    sem_wait(&auxptr->SEM_BITACORA);
    sprintf(msgBitacora, "El productor (%d) va a dormir", pid);
    strcpy(&auxptr->mensaje_log[0], msgBitacora);
    kill(pidCreator, SIGVTALRM );
    sleep(sleepTime);
    tiempoEspera = clock() - tiempoEspera;
    contadorTiempoEspera += ((double)tiempoEspera)/CLOCKS_PER_SEC + sleepTime;
    sem_wait(&auxptr->SEM_BITACORA);
    sprintf(msgBitacora, "El productor (%d) se despertó", pid);
    strcpy(&auxptr->mensaje_log[0], msgBitacora);
    kill(pidCreator, SIGVTALRM );

    //Semaforo Vacio
    sem_wait(&auxptr->SEM_BITACORA);
    sprintf(msgBitacora, "El productor (%d) va a pedir el semáforo de vacío", pid);
    strcpy(&auxptr->mensaje_log[0], msgBitacora);
    kill(pidCreator, SIGVTALRM );
    tiempoBloqueado = clock();
    sem_wait(&auxptr->SEM_VACIO);
    tiempoBloqueado = clock() - tiempoBloqueado;
    contadorTiempoBloqueado += ((double)tiempoBloqueado)/CLOCKS_PER_SEC;
    sem_wait(&auxptr->SEM_BITACORA);
    sprintf(msgBitacora, "El productor (%d) recibió el semáforo de lleno", pid);
    strcpy(&auxptr->mensaje_log[0], msgBitacora);
    kill(pidCreator, SIGVTALRM );

    //Semaforo Buffer
    sem_wait(&auxptr->SEM_BITACORA);
    sprintf(msgBitacora, "El productor (%d) va a pedir el semáforo del buffer", pid);
    strcpy(&auxptr->mensaje_log[0], msgBitacora);
    kill(pidCreator, SIGVTALRM );
    tiempoBloqueado = clock();
    sem_wait(&auxptr->SEM_BUFFER);
    tiempoBloqueado = clock() - tiempoBloqueado;
    contadorTiempoBloqueado += ((double)tiempoBloqueado)/CLOCKS_PER_SEC;
    sem_wait(&auxptr->SEM_BITACORA);
    sprintf(msgBitacora, "El productor (%d) recibió el semáforo del buffer", pid);
    strcpy(&auxptr->mensaje_log[0], msgBitacora);
    kill(pidCreator, SIGVTALRM );


    //Generación de mensajes
    //Calcular llave
    //rand() % (max_number + 1 - minimum_number) + minimum_number
    int llave = rand() % (5);
    //Calculamos el tiempo
    time_t t;
    time(&t);
    //Creamos el mensaje
    char mensaje[64];
    int pos = 0;
    pos += sprintf(&mensaje[pos],"%s", "Llave: ");
    pos += sprintf(&mensaje[pos], "%d", llave);
    pos += sprintf(&mensaje[pos],"%s", " Identificador: ");
    pos += sprintf(&mensaje[pos], "%d", pid);
    pos += sprintf(&mensaje[pos], "%s", " Tiempo: ");
    pos += sprintf(&mensaje[pos], "%s", ctime(&t));

    //Añadimos mensaje al buffer
    strcpy(bufptr[auxptr->index_escritura], mensaje);

    sem_wait(&auxptr->SEM_BITACORA);
    sprintf(msgBitacora, "El productor (%d) escribió en el índice de escritura %d el mensaje: %s", pid, auxptr->index_escritura, mensaje);
    strcpy(&auxptr->mensaje_log[0], msgBitacora);

    //Aumentamos indice index_escritura
    auxptr->index_escritura = (auxptr->index_escritura + 1) % bufferSize;

    //Devolver semáforos

    kill(pidCreator, SIGCHLD);
    sem_post(&auxptr->SEM_BUFFER);
    sem_wait(&auxptr->SEM_BITACORA);
    sprintf(msgBitacora, "El productor (%d) liberó el semáforo del buffer", pid);
    strcpy(&auxptr->mensaje_log[0], msgBitacora);
    kill(pidCreator, SIGVTALRM );

    sem_post(&auxptr->SEM_LLENO);
    sem_wait(&auxptr->SEM_BITACORA);
    sprintf(msgBitacora, "El productor (%d) liberó el semáforo de lleno", pid);
    strcpy(&auxptr->mensaje_log[0], msgBitacora);
    kill(pidCreator, SIGVTALRM );

    printf("Productor(%d) envió mensaje, con el indice de entrada: %d. Productores vivos: %d, consumidores vivos: %d.\n", pid, auxptr->index_escritura, auxptr->PRODUCTORES, auxptr->CONSUMIDORES);

    contadorMensajes++;

  }

    tiempoBloqueado = clock();
    sem_wait(&auxptr->SEM_PRODUCTORES);
    tiempoBloqueado = clock() - tiempoBloqueado;
    contadorTiempoBloqueado += ((double)tiempoBloqueado)/CLOCKS_PER_SEC;
    //Decrementar el contador de productores vivos
    auxptr->PRODUCTORES--;
    //Devolver el semaforo de contador de productores vivos
    sem_post(&auxptr->SEM_PRODUCTORES);
    sem_wait(&auxptr->SEM_BITACORA);
    printf("Productor(%d) termina con %d mensajes, %f segundos esperando y %f segundos bloqueado.\n", pid, contadorMensajes, contadorTiempoEspera, contadorTiempoBloqueado);
    sprintf(msgBitacora, "El productor (%d) murió", pid);
    strcpy(&auxptr->mensaje_log[0], msgBitacora);
    kill(pidCreator, SIGUSR2);

    return 0;

//productor(nombreBuffer,media);

}
