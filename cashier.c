#include "local.h"

int getRandom(int,int);
int main(int argc, char *argv[]){
  key_t       key; // key for generating msg queue
  pid_t       parent_pid = getppid(); // parent pid for defining the shared memory key
  int         mid, n,shmid; // msg queue id, n for reading ( will store char count) , shmid (shared memory id)
  MESSAGE     msg; // instance of message struct 

  struct  MEMORY* memory; // memory struct
  char          *shmptr; // pointer to shared memory 
  union semun    arg; // arg for later use
  struct msqid_ds buf; // to get info on msg queue

  int behaviour = 100;
  int timeToScan = getRandom(MINIMUM_SCANNING_TIME, MAXIMUM_SCANNING_TIME); 
  int sales = 0;

  prctl(PR_SET_PDEATHSIG, SIGHUP); // GET A SIGNAL WHEN PARENT IS KILLED
  srand((unsigned) getpid());

  if(argc < 2){
    perror("CASHIER: Not enough args");
    exit(-2);
  }

  int index = atoi(argv[1]);
  printf("\nCASHIER: Parent Pid: %d, Index: %d\n", parent_pid, index);
  
  if ((key = ftok(".", SEED + index)) == -1) {    
    perror("CASHIER:  Client: key generation");
    return 1;
  }

  if ((mid = msgget(key, 0 )) == -1 ) {
    mid = msgget(key,IPC_CREAT | 0660);
  }
  printf("\nCASHIER: SUCCESSFULY CREATED MQ! id =  %d \n", mid);

  
    if ( (shmid = shmget((int)parent_pid + index, sizeof(memory),
		       IPC_CREAT | 0666)) != -1 ) {
    
    if ( (shmptr = (struct MEMORY *) shmat(shmid, 0, 0)) == (char *) -1 ) {
      perror("shmptr -- parent -- attach");
      exit(1);
    }
    //memcpy(shmptr, (struct MEMORY *) &memory, sizeof(memory));
    memory = (struct MEMORY *) shmptr;
    printf("CASHIER: SUCCESSFULY CREATED SHMEM! id =  %d\n", shmid);
  }
  else {
    perror("shmid -- parent -- creation");
    exit(2);
  }

  while(1){
    msgctl(mid, IPC_STAT, &buf);
    printf("CASHIER: Current # of bytes on queue\t %d\n", buf.msg_cbytes);
    printf("CASHIER: Current # of messages on queue\t %d\n", buf.msg_qnum); /* Read Queue Status to update Shared Memory*/
    printf("CASHIER: Time to scan = %d\n", timeToScan);
    printf("CASHIER: Behaviour = %d\n", behaviour);

    memory->queueSize = buf.msg_qnum;
    memory->numberOfItems = buf.msg_cbytes; // MUST DO EQUATION TO CALCULATE NUMBER OF ITEMS BASED ON SIZE -> AFTER DEFININE THE SHOPPING CART STRUCT
    memory->timeToScan = timeToScan;
    memory->behaviour = behaviour; /* Update Shared Memory */

    if ((n = msgrcv(mid, &msg, sizeof(msg), SERVER, 0)) == -1 ) { /* Start waiting for a message to appear in MQ */
      perror("CASHIER:  msgrcv error");
      return 2;
    }
    printf("CASHIER: Just recieved message by : %d\nCASHIER: Now testing to see if he's still available!\n", (int)msg.clientId);
                
    /* Handle the message (shopping cart) & calculate the total cost */
    int totalCost = 0 ;

    /* start by checking if the process is still alive */

    pid_t c_pid = msg.clientId; // REAL PID FROM MSG
    if (kill(c_pid,SIGUSR1) == 0 ){
      /* Message still up -> handle it */
      printf("CASHIER: Items in the cart:\n");
      for(int i = 0; i<msg.cart.itemCount;i++) // something wrong about msg.cart.itemCount
      {
        printf("CASHIER: {%d} %s {Quantity: %d, price: %.2f}\n", index, msg.cart.items[i].name, msg.cart.items[i].price, msg.cart.items[i].price);
        totalCost += msg.cart.items[i].price; //increase the total coast
        usleep(19000000); // delay between priniting each item (scaning time)
      }
      // print the total coast
      printf("CASHIER: id = {%d} Finished\n Sum of prices : %.2f\n", index,totalCost); // regarding index is the id (from the loop) of the cashier
    }


    printf("CASHIER:  informing client id %s\n", (int)c_pid);
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