#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int contadorMensajes = 0;
double contadorTiempoEspera = 0;
double contadorTiempoBloqueado = 0;
int pid;
clock_t tiempoEspera;
clock_t tiempoBloqueado;

int main(int argc, char **argv)
{
    pid = getpid();  
    //Leer parametros
    char nombreBuffer[] = "nombreBuffer";
    int media = 3;
    
    printf("Consumidor(%d) empieza.\n", pid);

    tiempoBloqueado = clock();
    //Pedir semaforo para contador de consumidores vivos
    tiempoBloqueado = clock() - tiempoBloqueado;
    contadorTiempoBloqueado += ((double)tiempoBloqueado)/CLOCKS_PER_SEC;
    //Aumentar el contador de consumidores vivos
    //Devolver el semaforo de contador de consumidores vivos

    while(1)
    {
        tiempoEspera = clock();
        //Calculo de media
        sleep(media);
        tiempoEspera = clock() - tiempoEspera;
        contadorTiempoEspera += ((double)tiempoEspera)/CLOCKS_PER_SEC + media;

        tiempoBloqueado = clock();
        //Pedir semaforo para indice de lectura
        tiempoBloqueado = clock() - tiempoBloqueado;
        contadorTiempoBloqueado += ((double)tiempoBloqueado)/CLOCKS_PER_SEC;

        //Leer el indice de lectura
        int indice = contadorMensajes;
        //Leer el contador de productores vivos
        int contadorProductoresVivos = 0;
        //Leer el contador de consumidores vivos
        int contadorConsumidoresVivos = 0;

        //Leer buffer en indice

        char *mensaje = "Llave: 5";

        printf("Consumidor(%d) lee mensaje, con el indice de entrada: %d. Productores vivos: %d, consumidores vivos: %d.\n", pid, indice, contadorProductoresVivos, contadorConsumidoresVivos);

        contadorMensajes++;

        int llave = atoi("5");
        llave = contadorMensajes;

        if (llave == 5 || pid % 5 == llave)
        {
            tiempoBloqueado = clock();
            //Pedir semaforo para contador de consumidores vivos
            tiempoBloqueado = clock() - tiempoBloqueado;
            contadorTiempoBloqueado += ((double)tiempoBloqueado)/CLOCKS_PER_SEC;
            //Decrementar el contador de consumidores vivos
            //Devolver el semaforo de contador de consumidores vivos

            printf("Consumidor(%d) termina con %d mensajes, %f segundos esperando y %f segundos bloqueado.\n", pid, contadorMensajes, contadorTiempoEspera, contadorTiempoBloqueado);
            return 0;
        }

        //Aumentar indice de lectura circular
        //Devolver semaforo de indice de lectura
    }
}