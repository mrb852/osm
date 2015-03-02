
#ifndef BUENOS_KERNEL_SEM_H
#define BUENOS_KERNEL_SEM_H

#include "kernel/semaphore.h"

typedef enum {
  USER_SEM_STATE_FREE,
  USER_SEM_STATE_RUNNING
} USER_SEM_STATE;

typedef void usr_sem_t;

// Bruger kernels semaphore implementering. Der laves en array med samme længe 
// som kernens semaphore implementering. Userland og kernen deler det totale 
// antal semaphore.
typedef struct {
  semaphore_t* sem;
  char name[32];
  USER_SEM_STATE state;
} user_semaphore_t;


usr_sem_t* syscall_sem_open(char const* name, int value);
int syscall_sem_p(usr_sem_t* handle);
int syscall_sem_v(usr_sem_t* handle);
int syscall_sem_destroy(usr_sem_t* handle);

#endif
