#ifndef BUENOS_PROC_SEMAPHORE
#define BUENOS_PROC_SEMAPHORE

#include "lib/types.h"
#include "kernel/semaphore.h"


typedef void usr_sem_t;

#define MAX_USRSEM 32
#define MAX_USRSEM_NAME 32

typedef enum {
    USRSEM_FREE,
    USRSEM_USED
} usr_sem_state_t;

typedef struct {
    usr_sem_state_t state;
    char name[MAX_USRSEM_NAME];
    semaphore_t* kernel_sem;
} usr_sem_block_t;

void usr_sem_init();

usr_sem_t* usr_sem_open(const char* name, int value);

int usr_sem_p(usr_sem_t* sem);

int usr_sem_v(usr_sem_t* sem);

int usr_sem_destroy(usr_sem_t* sem);

#endif
