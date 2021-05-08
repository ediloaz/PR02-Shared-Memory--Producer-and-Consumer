#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>


#define NAMEMAX 100

//Para compilar
//gcc consumidor.c -o consumidor -lm -lpthread -lrt

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


//Recibe: -nombre del buffer circular
//        -media exponencial de cada cuando enviará un msj al buffer
void productor(char nombreBuffer[],double lambda){


  while(flag_productor){


    printf("Productor(%d) empieza.\n", getpid());

    void* ptrConsumidores = mmap(0, 4096, PROT_READ, MAP_SHARED, shm_open("CONSUMIDORES", O_RDWR, 0666), 0);
    void* ptrIndiceEscritura = mmap(0, 4096, PROT_READ, MAP_SHARED, shm_open("INDICELECTURA", O_RDWR, 0666), 0);
    void* ptrProductores = mmap(0, 4096, PROT_READ, MAP_SHARED, shm_open("PRODUCTORES", O_RDWR, 0666), 0);
    void* ptrBuffer = mmap(0, 4096, PROT_READ, MAP_SHARED, shm_open("BUFFER", O_RDWR, 0666), 0);

    tiempoBloqueado = clock();
    sem_t *semproductores = sem_open("SEMPRODUCTORES", O_CREAT, 0600, 0);
    tiempoBloqueado = clock() - tiempoBloqueado;
    contadorTiempoBloqueado += ((double)tiempoBloqueado)/CLOCKS_PER_SEC;

    if (semproductores == SEM_FAILED) printf("SEMPRODUCTORES Failed\n");

    //Aumentamos cantidad de productores
    printf("%d", (char*)ptrProductores);
    int productores = atoi((char*)ptrProductores) + 1;
    memcpy(ptrProductores, productores, sizeof(productores));
    printf("%d", (char*)ptrProductores);

    sem_post(semproductores);
    kill(pidCreator, SIGUSR1);

    double sleepTime = ran_expo(lambda);
    tiempoEspera = clock();
    sleep(sleepTime);
    tiempoEspera = clock() - tiempoEspera;
    contadorTiempoEspera += ((double)tiempoEspera)/CLOCKS_PER_SEC + sleepTime;

    //Tiempo de bloqueo si está lleno
    tiempoBloqueado = clock();
    sem_t *sembuffer = sem_open("SEMBUFFER", O_CREAT, 0600, 0);
    tiempoBloqueado = clock() - tiempoBloqueado;
    contadorTiempoBloqueado += ((double)tiempoBloqueado)/CLOCKS_PER_SEC;

    //Leer el contador de productores vivos
    int contadorProductoresVivos = atoi((char*)ptrProductores);

    //Leer el contador de consumidores vivos
    int contadorConsumidoresVivos = atoi((char*)ptrConsumidores);

    //Indice de index_escritura
    //printf("%d", (char*)ptrIndiceEscritura);
    int indice = atoi((char*)ptrIndiceEscritura);

    //printf("%d", (char*)ptrIndiceEscritura);

    //Generación de mensajes
    //Calcular llave
    //rand() % (max_number + 1 - minimum_number) + minimum_number
    int llave = rand() % (5);
    //Calculamos el tiempo
    time_t t;
    time(&t);
    //Creamos el mensaje
    char mensaje[64];
    strcpy(mensaje, "Llave: ");
    //sprintf(llave_s, "%d", llave);
    strcpy(mensaje, llave);
    strcpy(mensaje, "Identificador: ");
    //sprintf(pid_s, "%d", pid);
    strcpy(mensaje, pid);
    strcpy(mensaje, "Tiempo: ");
    //sprintf(tiempo_s, "%s", ctime(&t));
    strcpy(mensaje, t);

    //Añadimos mensaje al buffer
    memcpy(&ptrBuffer[indice], mensaje, sizeof(mensaje));

    //Aumentamos indice index_escritura
    indice = (indice + 1) % bufferSize;
    memcpy(ptrIndiceEscritura, indice, sizeof(indice));

    //Devolver semaforo de indice de lectura
    sem_post(sembuffer);

    kill(pidCreator, SIGALRM);

    printf("Productor(%d) envió mensaje, con el indice de entrada: %d. Productores vivos: %d, consumidores vivos: %d.\n", pid, indice, contadorProductoresVivos, contadorConsumidoresVivos);

    contadorMensajes++;

  }

  if (!flag_productor) {
    printf("Productor(%d) termina con %d mensajes, %f segundos esperando y %f segundos bloqueado.\n", pid, contadorMensajes, contadorTiempoEspera, contadorTiempoBloqueado);
    return 0;
  }
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

productor(nombreBuffer,media);

}
