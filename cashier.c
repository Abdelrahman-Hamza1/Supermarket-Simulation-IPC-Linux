#include "local.h"
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
  int timeToScan = 5; //WILL BE RANDOM LATER ON 
  int sales =0;
  int maxSales = 10000; // FROM FILE


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

    if ((n = msgrcv(mid, &msg, sizeof(message), SERVER, 0)) == -1 ) { /* Start waiting for a message to appear in MQ */
      perror("Server: msgrcv");
      return 2;
    } 
                
    /* Handle the message (shopping cart) & calculate the total cost */
    int totalCost =0;

    // Send cost back to customer ? ? ? ?  IF NOT -> KILL CUSTOMER (LEAVE SUPER MARKET)

    /* Behaviour will decrease with time, total sales will be increased aswell. Check if conditions met & send signals if so.*/
    behaviour--;
    if (behaviour == 0){
      kill(getppid(), 2);
    }

    sales = sales + totalCost;
    if (sales >= maxSales){
      kill(getppid(), 12);
    }

    
  }
  return 0;
}

