#include "tests/lib.h"
#include "proc/syscall.h"

int main() {
    // semaphore_t *one = syscall_sem_open("Slumbob", 2);
    // semaphore_t *two = syscall_sem_open("Sup bob", 3);
    
    // syscall_exec("[disk]halt");
    int mus = syscall_exec("[disk]mus");
    int kat = syscall_exec("[disk]kat");

    syscall_join(mus);
    syscall_join(kat);


    return 0;
}
