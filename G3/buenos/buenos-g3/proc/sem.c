#include "sem.h"
#include "kernel/semaphore.h"
#include "kernel/interrupt.h"
#include "kernel/config.h"

#define SEM_MAX_NAME_LEN 32

static spinlock_t semaphore_table_slock;
static user_semaphore_t semaphore_table[CONFIG_MAX_SEMAPHORES];

/*
typedef struct {
  semaphore_t* sem;
  const char* name;
  USER_SEM_STATE state;
} user_semaphore_t;
*/

usr_sem_t* syscall_sem_open(char const* name, int value) {

  semaphore_t* sem = semaphore_create(value);
  if (sem == NULL) return NULL;

  int intr_status = _interrupt_disable();
  spinlock_acquire(&semaphore_table_slock);

  int i;
  for (i = 0; i < CONFIG_MAX_SEMAPHORES; ++i) {
    if (semaphore_table[i].state == USER_SEM_STATE_FREE) {
      semaphore_table[i].sem = sem;
      stringcopy(semaphore_table[i].name, name, SEM_MAX_NAME_LEN);
      semaphore_table[i].state = USER_SEM_STATE_RUNNING;
      break;
    }
  }

  spinlock_release(&semaphore_table_slock);
  _interrupt_set_state(intr_status);

  return &semaphore_table[i];
}

int syscall_sem_p(usr_sem_t* handle) {
  semaphore_P(((user_semaphore_t *)handle)->sem);
  return 0;
}

int syscall_sem_v(usr_sem_t* handle) {
  semaphore_V(((user_semaphore_t *)handle)->sem);
  return 0;
}

int syscall_sem_destroy(usr_sem_t* handle) {
  int intr_status = _interrupt_disable();
  spinlock_acquire(&semaphore_table_slock);

  ((user_semaphore_t *)handle)->state = USER_SEM_STATE_FREE;

  spinlock_release(&semaphore_table_slock);
  _interrupt_set_state(intr_status);

  return 0;
}
