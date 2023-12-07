#include "local.h"

 int currentBehaviour = 0, currentCustomerImpatient =0,  numberOfServers = 1;

void signal_catcher(int );
void cleanUp();
int main(int argc, char *argv[]){

    
    char buff[20];
    sprintf(buff, "%d", numberOfServers);

    for (int i = 0 ; i < numberOfServers ; i++){
        switch (fork()) {
            case -1:
            perror("Cashier: fork");
            return 2;

            case 0:
            char buffer[20];
            sprintf(buffer, "%d", i);          
            execlp("./cashier", "cashier", buffer, "&", 0);
            perror("Cashier: exec");
            return 3;
        }
    }
        if ( sigset(2, signal_catcher) == SIG_ERR ) { // behaviour
        perror("Sigset can not set SIGINT");
        exit(SIGINT);
    }
    if ( sigset(10, signal_catcher) == SIG_ERR ) { // customers
        perror("Sigset can not set SIGINT");
        exit(SIGINT);
    }
    if ( sigset(12, signal_catcher) == SIG_ERR ) { // income
        perror("Sigset can not set SIGINT");
        exit(SIGINT);
    }

    int sleepTime = 10;
    
    while(1){
        sleep(10);

        // Create Client
      
            switch (fork()) {
                case -1:
                perror("Client: fork");
                return 2;

            case 0:        
            execlp("./customer", "customer", buff, "&", 0);
            perror("customer: exec");
            return 3;
     
            }
      
    }
}

void signal_catcher(int the_sig){
  printf("\nSignal %d received.\n", the_sig);

  switch(the_sig){
    case 2:
        currentBehaviour++;
        printf("Current Behaviour: %d", currentBehaviour);
        if(currentBehaviour < BEHAVIOUR_THRESHOLD){
            break;
        }
        cleanUp();
        exit(2);
    case 10:
        currentCustomerImpatient++;
        if(currentCustomerImpatient < ANGER_THRESHOLD){
            break;
        }
        cleanUp();
        exit(10);
    case 12:
        cleanUp();
        exit(12);
  }
}

void cleanUp(){
    // MSG QUEUE -> ID => CHARACTER BASED ||||| SHMEM -> ID -> PPID + index
    printf("PARENT CLEANING UP!\n");
    struct MEMORY mem;
    int  pid = (int) getpid();
     for (int i = 0 ; i < numberOfServers ; i++){
        key_t key = ftok(".", SEED + i);
        int mid = msgget(key, 0);
        msgctl(mid, IPC_RMID, (struct msgid_ds *) 0); /* remove first message queue*/
        
        
        int shmid = shmget(pid + i, sizeof(mem), 0); // POSSIBLE: CHANGE LAST ARG TO 0 
        shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0);
     }
}
