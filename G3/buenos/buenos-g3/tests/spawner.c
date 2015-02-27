#include "tests/lib.h"

int main(void) {
    semaphore_t *one = syscall_sem_open("Slumbob", 2);
    semaphore_t *two = syscall_sem_open("Sup bob", 3);
    return 0;
}
