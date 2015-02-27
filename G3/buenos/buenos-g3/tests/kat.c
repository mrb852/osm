#include "tests/lib.h"
#include "proc/sem.h"

int kathandle = syscall_sem_open("kat",2);

int main() {

    syscall_sem_p(kathandle);

    //tty_write(0, "Katten fanger mussen.", 64);

    syscall_sem_v(kathandle);

    syscall_exit(0);
    return 0;
}
