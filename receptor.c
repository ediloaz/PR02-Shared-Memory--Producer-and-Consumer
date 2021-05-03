//receptor
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

void sig_handler(int signum){

   if(signum == SIGUSR1){
       printf("Recibí la señal del sender\n");
   }

}

int main(int argc, char **argv){
    
    pid_t pid = getpid();
    printf("PID: %d\n", pid);
    
    signal(SIGUSR1, sig_handler);
    
    //Duerma indefinidamente hasta que se despierte con señal
    for(int i=1;;i++){
      printf("Esperando...\n");
      sleep(1);
    }
    
    return 0;
}
