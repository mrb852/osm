#include "tests/lib.h"
#include "proc/sem.h"

int mushandle = syscall_sem_open("mus", 2);

int main() {

    syscall_sem_p(mushandle);

    //tty_write(0, "Mussen fanger katten.", 64);

    syscall_sem_v(mushandle);

    syscall_exit(0);
    return 0;
}