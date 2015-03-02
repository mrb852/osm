#include "sem.h"
#include "kernel/semaphore.h"
#include "kernel/interrupt.h"
#include "kernel/config.h"
#include "kernel/panic.h"

#define SEM_MAX_NAME_LEN 32

static spinlock_t semaphore_table_slock;
static user_semaphore_t semaphore_table[CONFIG_MAX_SEMAPHORES];


usr_sem_t* syscall_sem_open(char const* name, int value) {
  // Opretter semaphore
  semaphore_t* sem = semaphore_create(value);
  // Ser om det var muligt at lave en semaphore
  if (sem == NULL) return NULL;

  // Låser så der ikke overskrives
  int intr_status = _interrupt_disable();
  spinlock_acquire(&semaphore_table_slock);

  // Finder første ledige og reservere pladsen
  int i;
  for (i = 0; i < CONFIG_MAX_SEMAPHORES; ++i) {
    if (semaphore_table[i].state == USER_SEM_STATE_FREE) {
      semaphore_table[i].sem = sem;
      user_semaphore_t *sem_pointer = &semaphore_table[i]; 
      stringcopy(sem_pointer->name, name, SEM_MAX_NAME_LEN);
      
      semaphore_table[i].state = USER_SEM_STATE_RUNNING;
      break;
    }
  }

  // Åbner låsen igen
  spinlock_release(&semaphore_table_slock);
  _interrupt_set_state(intr_status);

  return &semaphore_table[i];
}

// Bruger kernel implementeringen af semaphore
int syscall_sem_p(usr_sem_t* handle) {
  semaphore_P(((user_semaphore_t *)handle)->sem);
  return 0;
}

// Bruger kernel implementeringen af semaphore
int syscall_sem_v(usr_sem_t* handle) {
  semaphore_V(((user_semaphore_t *)handle)->sem);
  return 0;
}

// Fjerner semaphoren ved at sætte dens state til free 
int syscall_sem_destroy(usr_sem_t* handle) {
  int intr_status = _interrupt_disable();
  spinlock_acquire(&semaphore_table_slock);

  ((user_semaphore_t *)handle)->state = USER_SEM_STATE_FREE;

  spinlock_release(&semaphore_table_slock);
  _interrupt_set_state(intr_status);

  return 0;
}
