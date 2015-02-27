#include "tests/lib.h"

int main() {
    int kathandle = (int)syscall_sem_open("kat",2);

    syscall_sem_p((usr_sem_t*) kathandle);

    //tty_write(0, "Katten fanger mussen.", 64);

    syscall_sem_v((usr_sem_t*) kathandle);

    syscall_exit(0);
    return 0;
}
