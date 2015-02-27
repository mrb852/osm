#include "tests/lib.h"

int main() {
    int mushandle = (int)syscall_sem_open("mus", 2);

    syscall_sem_p((usr_sem_t*) mushandle);

    //tty_write(0, "Mussen fanger katten.", 64);

    syscall_sem_v((usr_sem_t*) mushandle);

    syscall_exit(0);
    return 0;
}