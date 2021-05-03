//Envio señal
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

int main(int argc, char **argv){

   pid_t pid = atoi(argv[1]);
   printf("Receptor PID: %d\n", pid);
   sleep(3);
   printf("Voy a enviar señal a receptor\n");
   
   int result = kill(pid, SIGUSR1);
   
   if(result == 0){
   	printf("Enviado correctamente\n");
   	return 0;
   }else{
   	printf("Error al enviar\n");
   }   
}
