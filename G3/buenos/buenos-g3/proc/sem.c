#include "sem.h"
#include "kernel/semaphore.h"
#include "kernel/interrupt.h"
#include "kernel/config.h"

#define SEM_MAX_NAME_LEN 32

static spinlock_t semaphore_table_slock;
static user_semaphore_t semaphore_table[CONFIG_MAX_SEMAPHORES];

usr_sem_t* syscall_sem_open(char const* name, int value) {

  semaphore_t sem = semaphore_create(value);
  if (sem == NULL) return NULL;

  for (int i = 0; i < CONFIG_MAX_SEMAPHORES; ++i) {
    if (semaphore_table[i].sem.creator == -1) {
      semaphore_table[sem_id].sem.creator = thread_get_current_thread();
      break;
    }
  }

  user_semaphore_t user_sem = {
    .sem = sem,
    .name = name
  };

  return user_sem;
}

int syscall_sem_p(usr_sem_t* handle) {

}

int syscall_sem_v(usr_sem_t* handle) {

}

int syscall_sem_destroy(usr_sem_t* handle) {

}
