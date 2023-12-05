#include "local.h"
int main(int argc, char *argv[]){
  key_t       key;
  pid_t       parent_pid = getppid();
  int         mid, n;
  MESSAGE     msg;
  static char m_key[10];

  
  static struct  MEMORY memory;
  int            semid, shmid, croaker;
  char          *shmptr;
  pid_t          p_id, c_id, pid = getpid();
  union semun    arg;

  if(argc != 2){
    perror("Not enough args");
    exit(2);
  }

  int index = atoi(argv[1]);
  printf("%d", parent_pid);
  
  if ((key = ftok(".", SEED + index)) == -1) {    
    perror("Client: key generation");
    return 1;
  }

  if ((mid = msgget(key, 0 )) == -1 ) {        
    mid = msgget(key,IPC_CREAT | 0660);
  }

    if ( (shmid = shmget((int) parent_pid, sizeof(memory),
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
    update status on shmem (using external system methods to inspect Message Queue)

    read queue (blocking)

    ....... execute queue .... 

    POSSIBLE : return0 to client or print to client. 
  }

  
//   while (1) {
//     msg.msg_to = SERVER;
//     msg.msg_fm = cli_pid;                   
//     //write(fileno(stdout), "cmd> ", 6);
//     write(fileno(stdout), "cmd> ", strlen("cmd> "));
//     memset(msg.buffer, 0x0, BUFSIZ);
    
//     if ( (n = read(fileno(stdin), msg.buffer, BUFSIZ)) == 0 )
//       break;
    
//     n += sizeof(msg.msg_fm);
    
//     if (msgsnd(mid, &msg, n, 0) == -1 ) {
//       perror("Client: msgsend");
//       return 4;
//     }
    
//     if( (n = msgrcv(mid, &msg, BUFSIZ, cli_pid, 0)) != -1 )
//       write(fileno(stdout), msg.buffer, n);  
//   }
//   msgsnd(mid, &msg, 0, 0);
  return 0;
}
