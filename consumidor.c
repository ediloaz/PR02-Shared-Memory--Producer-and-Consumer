#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
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

    int result = kill(pidCreator, SIGUSR1);
    if(result == 0){
   	    printf("Enviado correctamente\n");
    }else{
   	    printf("Error al enviar\n");
    }   
    
    printf("Consumidor(%d) empieza.\n", pid);

    tiempoBloqueado = clock();
    //Pedir semaforo para contador de consumidores vivos
    tiempoBloqueado = clock() - tiempoBloqueado;
    contadorTiempoBloqueado += ((double)tiempoBloqueado)/CLOCKS_PER_SEC;
    //Aumentar el contador de consumidores vivos
    //Devolver el semaforo de contador de consumidores vivos

    while(1)
    {
        double sleepTime = ran_expo(media);
        tiempoEspera = clock();
        sleep(sleepTime);
        tiempoEspera = clock() - tiempoEspera;
        contadorTiempoEspera += ((double)tiempoEspera)/CLOCKS_PER_SEC + sleepTime;

        tiempoBloqueado = clock();
        //Pedir semaforo para leer
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
        //Aumentar indice de lectura circular
        //Borrar el mensaje
        //Devolver semaforo de indice de lectura

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
    }
}
