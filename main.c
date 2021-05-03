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

//Flag del productores
int flag_productor = 0;
//***Hay que usar memoria compartida para la bandera (?)***

int total_productores = 0;
double tiempo = 0;

// A normal C function that is executed as a thread
// when its name is specified in pthread_create()
double ran_expo(double lambda, double current_time)
{
    double u;
    u = rand() / (RAND_MAX + 1.0);
    double exp = -lambda * log(u);
    return exp + current_time;
}

//Recibe: -nombre del buffer circular
//        -media exponencial de cada cuando enviará un msj al buffer
void productor(char nombreBuffer[],double lambda){

  printf("Productor(%d) empieza.\n", getpid());
  int total_mensajes = 0;
  int pid;

  while(flag_productor){
    //int total_tiempo = 0;
    //Aumenta total de productores
    total_productores++;
    //Obtener información del mensaje
    pid = getpid();
    //Calcula tiempo del siguiente mensaje
    double next_msg = ran_expo(lambda, tiempo);
    tiempo += next_msg;
    //Calcular llave
    //rand() % (max_number + 1 - minimum_number) + minimum_number
    int llave = rand() % (5);
    sleep(next_msg);
    //Acceder al Buffer con info: pid.tiempo,llave
    printf("El productor añade al buffer el siguiente mensaje:\n Identificador %d \n Tiempo: %f \n Llave %d.\n", pid, tiempo, llave);
    //Función imprimir mensaje desde buffer (?)
  }

  if (!flag_productor) {
    total_productores--;
    printf("El productor con el id %d finalizó. A continuación sus estadísticas\n",pid );
    //***Faltan estadísticas***
    printf("Total de mensajes enviados: %d\n Total de tiempo acumulado:%f ",total_mensajes,tiempo );
  }
}

//Controlar que si no hay espacio en buffer productor en pausa con sems.

int main(int argc, char **argv)
{
    double tiempo = 0;
    //**Buffer en memoria compartida o sems (?)***

}