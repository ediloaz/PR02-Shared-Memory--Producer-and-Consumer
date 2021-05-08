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
#define LOGMAX 100 
#define ENTRYMAX 64


//COMPILAR: gcc CreadorDummy.c -o creador -lpthread -lrt
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

struct auxiliar_t* bufptr;

void sig_handler(int signum){

   if(signum == SIGUSR1){
       printf("Recibí la señal de creacion de consumidor. Ahora hay: %d vivos\n",bufptr->CONSUMIDORES );
       
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
    
    typedef char buffer_t[buff_size][ENTRYMAX]; 
    buffer_t* buffer;
    
    
    //DECLARA MEMORIA COMPARTIDA    
    int len = 4096;
    int fd = shm_open(nombreBuffer, O_RDWR | O_CREAT, 0666);
    if(fd < 0){
        printf("Error de shm_open\n");        
        return 1;
    }
    if(ftruncate(fd, len) == -1) {
    	printf("Error de ftruncate\n");        
        return 1;
    }
    printf("Largo: %ld\n", sizeof(struct auxiliar_t));
    bufptr = mmap(0,len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    buffer = (buffer_t*)mmap(0,buff_size*ENTRYMAX*sizeof(char), PROT_READ|PROT_WRITE, MAP_SHARED, fd,330);
    
    
    
    if(bufptr == MAP_FAILED){
    	printf("Error al crear la memoria compartida (mmap)\n");
        return 1;
    }

    
    //INICIALIZA VALROES DE STRUCT
    
    
    sem_init(&bufptr->SEM_CONSUMIDORES, 1, 1);
    sem_init(&bufptr->SEM_PRODUCTORES, 1, 1);
    
    sem_init(&bufptr->SEM_LLENO, 1, 0);
    sem_init(&bufptr->SEM_VACIO, 1, buff_size);
    
    sem_init(&bufptr->SEM_CBUFFER, 1, 1);
    sem_init(&bufptr->SEM_PBUFFER, 1, 1);
    
    bufptr->index_lectura = bufptr->index_escritura = 0;
    bufptr->PRODUCTORES = bufptr->CONSUMIDORES = 0;
    bufptr->max_buffer = buff_size;
    
    strcpy(buffer[0], "Hola");
    buffer[0] = "Hola\0";
    
    printf("Memoria compartida creada correctamente\n");
    //Ciclo infinito para mantenerlo vivo
    for(int i=1;;i++){
      sleep(1);
    }
    
    return 0;
}


