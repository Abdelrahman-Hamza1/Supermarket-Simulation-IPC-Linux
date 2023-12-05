#include "local.h"

void signal_catcher(int );
void cleanUp();
int main(int argc, char *argv[]){

    int numberOfServers = 5;
    for (int i = 0 ; i < numberOfServers ; i++){
        switch (fork()) {
            case -1:
            perror("Cashier: fork");
            return 2;

            case 0:          
            execlp("./cashier", "cashier", i, "&", 0);
            perror("Cashier: exec");
            return 3;
        }
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
            execlp("./customer", "customer", numberOfServers, "&", 0);
            perror("customer: exec");
            return 3;
        }
    }

//     positive behavior dropped to 0 
//     customers impatient 
//  cashiers made income t
    int behaviorMax = 4, customersImpatientMax = 10, currentBehaviour = 0, currentCustomerImpatient = 0;


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


}

void signal_catcher(int the_sig){
  printf("\nSignal %d received.\n", the_sig);

  switch(the_sig){
    case 2:
        currentBehaviour++;
        if(currentBehaviour < behaviorMax){
            break;
        }
        cleanUp();
        exit(2);
    case 10:
        currentCustomerImpatient++;
        if(currentCustomerImpatient < customersImpatientMax){
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
    int  pid = (int) getpid();
     for (int i = 0 ; i < 5 ; i++){
        key_t key = ftok(".", SEED + i);
        int mid = msgget(key, IPC_CREAT | 0666);
        msgctl(mid, IPC_RMID, (struct msgid_ds *) 0); /* remove first message queue*/
        
        
        int shmid = shmget(pid + i, sizeof(MEMORY), IPC_CREAT | 0666); // POSSIBLE: CHANGE LAST ARG TO 0 
        shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0);
     }
}

/*
Cashier 1: Queue + Shmem
Cashier 2: Queue + Shmem
Cashier 3: Queue + Shmem
Cashier 4: Queue + Shmem
Cashier 5: Queue + Shmem
*/