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


#define ROWS   5
#define COLS   3

#define SLOT_LEN  50
#define N_SLOTS   6

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
  char buffer[N_SLOTS][SLOT_LEN];
  int  head, tail;
}; 


#define SEED   'g'		/* seed for ftok */
// #define SERVER 1L		/* message for the server */

typedef struct {
  long      msg_type;
  char      buffer[BUFSIZ];
} MESSAGE;

#endif
