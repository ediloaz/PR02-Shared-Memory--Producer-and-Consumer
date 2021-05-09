#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/types.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define NAMEMAX 100 		//tamaño máximo del numbre del buffer
#define LOGMAX 164
#define ENTRYMAX 164
#define AUX "\auxiliar"

//COMPILAR: gcc CreadorDummy.c -o creador -lpthread -lrt
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
    int PRODUCTORES;		//total de productores vivos
    int CONSUMIDORES;		//total de consumidores vivos

    char mensaje_log[LOGMAX];
} ;


struct auxiliar_t* auxptr;

void sig_handler(int signum){

   if(signum == SIGUSR1){
       printf("Recibí la señal de creacion de consumidor. Ahora hay: %d vivos\n",auxptr->CONSUMIDORES );
       printf("LOG: %s\n", auxptr->mensaje_log);
   }
}

void sig_handler_P(int signum){

   if(signum == SIGUSR2){
       printf("Recibí la señal de creacion de productor. Ahora hay: %d vivos\n",auxptr->PRODUCTORES);
       printf("LOG: %s\n", auxptr->mensaje_log);
   }
}

void sig_handlerLog(int signum){
   printf("LOG: %s\n", auxptr->mensaje_log);
}


void sig_handlerBuff(int signum){

   if(signum == SIGALRM){
       printf("Buffer leido: INDEX_LECTURA: %d, INDEX_ESCRITURA: %d",auxptr->index_lectura, auxptr->index_escritura );
       printf("LOG: %s\n", auxptr->mensaje_log);
   }
}


int main(int argc, char** argv){

    pid_t pid = getpid();
    printf("Mi PID es: %d\n", pid);

    //LECTURA DE PARÁMETROS
    char nombreBuffer[NAMEMAX] = "/nombreBuffer";
    int buff_size = 10;
    int paramIndex = 1;

    while(paramIndex < argc)
    {
    	char* param = argv[paramIndex];
    	paramIndex++;

    	if(param[0] == '-'){
    	    switch(param[1]){
    	    	case 'n':
    	    	strcpy(nombreBuffer, argv[paramIndex]);
    	    	printf("Nombre del buffer: %s\n", nombreBuffer);
    	    	paramIndex++;
    	    	break;
    	    case 'm':
    	    	buff_size = atoi(argv[paramIndex]);
    	    	printf("Tamño del buffer: %d\n", buff_size);
    	    	paramIndex++;
    	    	break;
    	    default:
    	    	printf("Error: Parámetro no reconocido\n");
    	    	return 1;
    	    }
    	}
    }



    signal(SIGUSR1, sig_handler);
    signal(SIGUSR2, sig_handler_P);
    signal(SIGVTALRM, sig_handlerLog);
    signal(SIGALRM, sig_handlerBuff);


    struct buffer_t{
         char b[buff_size][ENTRYMAX];
    };

    struct buffer_t buffer;

    char (*bufptr)[ENTRYMAX];


    //DECLARA MEMORIA COMPARTIDA
    int len = sizeof(struct auxiliar_t);
    int fd = shm_open(AUX, O_RDWR | O_CREAT, 0666);
    int fd2 = shm_open(nombreBuffer, O_RDWR | O_CREAT, 0666);
    if(fd < 0 || fd2 < 0){
        printf("Error de shm_open\n");
        return 1;
    }
    if(ftruncate(fd, len) == -1 || ftruncate(fd2, ENTRYMAX*buff_size) == -1) {
    	printf("Error de ftruncate\n");
        return 1;
    }
    printf("Largo: %d\n", len);
    auxptr = mmap(0,len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    bufptr = mmap(0,ENTRYMAX*buff_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd2,0);

    if(auxptr == MAP_FAILED){
    	printf("Error al crear la memoria compartida (mmap)\n");
        return 1;
    }


    //INICIALIZA VALROES DE STRUCT

    sem_init(&auxptr->SEM_CONSUMIDORES, 1, 1);
    sem_init(&auxptr->SEM_PRODUCTORES, 1, 1);
    sem_init(&auxptr->SEM_FINALIZADOR, 1, 0);


    sem_init(&auxptr->SEM_LLENO, 1, 0);
    sem_init(&auxptr->SEM_VACIO, 1, buff_size-1);

    sem_init(&auxptr->SEM_BUFFER, 1, 1);

    auxptr->index_lectura = auxptr->index_escritura = 0;
    auxptr->PRODUCTORES = auxptr->CONSUMIDORES = 0;
    auxptr->max_buffer = buff_size;
    auxptr->flag_productor = 1;


    //printf("Largo: %ld\n", sizeof(bufptr));
    printf("Memoria compartida creada correctamente\n");
    //Ciclo infinito para mantenerlo vivo
    for(int i=1;;i++){
      sleep(1);
    }

    return 0;
}
