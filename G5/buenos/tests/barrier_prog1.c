#include "tests/lib.h"

#define BUFFER_SIZE 60

int main(void) {
  usr_sem_t *wait0, *wait1, *read_write_lock;
  char line[BUFFER_SIZE];

  puts("[barrier_prog1] Open the semaphores.\n");
  wait0 = syscall_sem_open("wait0", -1);
  wait1 = syscall_sem_open("wait1", -1);
  read_write_lock = syscall_sem_open("rwlock", -1);

  puts("[barrier_prog1] Do work before barrier.\n");
  syscall_sem_p(read_write_lock);
  readline(line, BUFFER_SIZE);
  printf("[barrier_prog1] You wrote: %s\n", line);
  syscall_sem_v(read_write_lock);

  puts("[barrier_prog1] Wait for the other process.\n");
  syscall_sem_v(wait1); /* DIFFERENT THAN IN barrier_prog0 */
  syscall_sem_p(wait0); /* DIFFERENT THAN IN barrier_prog0 */

  puts("[barrier_prog1] Do work after barrier.\n");

  syscall_exit(11);
  return 11;
}
