#include "local.h"

int getRandom(int,int);
int main(int argc, char *argv[]){
  key_t       key; // key for generating msg queue
  pid_t       parent_pid = getppid(); // parent pid for defining the shared memory key
  int         mid, n,shmid; // msg queue id, n for reading ( will store char count) , shmid (shared memory id)
  MESSAGE     msg; // instance of message struct 

  static struct  MEMORY memory; // memory struct
  char          *shmptr; // pointer to shared memory 
  union semun    arg; // arg for later use
  struct msqid_ds buf; // to get info on msg queue

  int behaviour = 100;
  int timeToScan = getRandom(MINIMUM_SCANNING_TIME, MAXIMUM_SCANNING_TIME); 
  int sales = 0;

  prctl(PR_SET_PDEATHSIG, SIGHUP); // GET A SIGNAL WHEN PARENT IS KILLED
  srand((unsigned) getpid());

  if(argc != 2){
    perror("Not enough args");
    exit(-2);
  }

  int index = atoi(argv[1]);
  printf("\nParent Pid: %d\n", parent_pid);
  
  if ((key = ftok(".", SEED + index)) == -1) {    
    perror("Client: key generation");
    return 1;
  }

  if ((mid = msgget(key, 0 )) == -1 ) {        
    mid = msgget(key,IPC_CREAT | 0660);
  }

    if ( (shmid = shmget((int)parent_pid + index, sizeof(memory),
		       IPC_CREAT | 0666)) != -1 ) {
    
    if ( (shmptr = (char *) shmat(shmid, 0, 0)) == (char *) -1 ) {
      perror("shmptr -- parent -- attach");
      exit(1);
    }
    memcpy(shmptr, (char *) &memory, sizeof(memory));
  }
  else {
    perror("shmid -- parent -- creation");
    exit(2);
  }

  while(1){
    msgctl(mid, IPC_STAT, &buf);
    printf("Current # of bytes on queue\t %d\n", buf.msg_cbytes);
    printf("Current # of messages on queue\t %d\n", buf.msg_qnum); /* Read Queue Status to update Shared Memory*/

    memory.queueSize = buf.msg_qnum;
    memory.numberOfItems = buf.msg_cbytes; // MUST DO EQUATION TO CALCULATE NUMBER OF ITEMS BASED ON SIZE -> AFTER DEFININE THE SHOPPING CART STRUCT
    memory.timeToScan = timeToScan;
    memory.behaviour = behaviour; /* Update Shared Memory */

    if ((n = msgrcv(mid, &msg, sizeof(msg), SERVER, 0)) == -1 ) { /* Start waiting for a message to appear in MQ */
      perror("Server: msgrcv");
      return 2;
    } 
                
    /* Handle the message (shopping cart) & calculate the total cost */
    int totalCost = 0 ;

    /* start by checking if the process is still alive */

    pid_t c_pid; // REAL PID FROM MSG
    if (kill(c_pid,SIGUSR1) == 0 ){
      /* Message still up -> handle it */
    }



    kill(c_pid,SIGUSR2); /* LET CUSTOMER KNOW YOU'RE DONE PROCESSING THEM!.*/

    /* Behaviour will decrease with time, total sales will be increased aswell. Check if conditions met & send signals if so.*/
    behaviour--;
    if (behaviour == 0){
      kill(getppid(), 2); // might have to cast ppid to int 
    }

    sales = sales + totalCost;
    if (sales >= INCOME_THRESHOLD){
      kill(getppid(), 12);
    }
  }
  return 0;
}

int getRandom(int min, int max){
  return (int) (min + (rand() % (max - min)));
}