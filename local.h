#ifndef __LOCAL_H_
#define __LOCAL_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <wait.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

/* This declaration is *MISSING* is many solaris environments.
   It should be in the <sys/sem.h> file but often is not! If 
   you receive a duplicate definition error message for semun
   then comment out the union declaration.
   */

union semun {
  int              val;
  struct semid_ds *buf;
  ushort          *array; 
};

struct MEMORY {
  int queueSize;
  int numberOfItems;
  int timeToScan;
  int behaviour;
}; 


#define SEED   'g'		/* seed for ftok */
#define SERVER 1L
#define CLIENT 0L

typedef struct {
  long      msg_type;
  /* SHOPPING CART -> for now i'll assume it's just a number that represents how many items*/
  int items;
  
} MESSAGE;


// Defining variables

#define MINIMUM_ARRIVAL_RATE 10
#define MAXIMUM_ARRIVAL_RATE 20

#define MINIMUM_SHOPPING_TIME 10
#define MANIMUM_SHOPPING_TIME 20

#define MINIMUM_SCANNING_TIME 5
#define MAXIMUM_SCANNING_TIME 10

#define MAXIMUM_WAITING_TIME 50

#define BEHAVIOUR_THRESHOLD 5

#define ANGER_THRESHOLD 10

#define INCOME_THRESHOLD 10000

#endif
