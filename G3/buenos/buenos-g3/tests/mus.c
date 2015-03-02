#include "tests/lib.h"

int main() {
    int mushandle = (int)syscall_sem_open("mus", 2);

    int i;
    for(i = 0; i < 10; i++){
        syscall_sem_p((usr_sem_t*) mushandle);
        printf("Mus\n\n");
        syscall_sem_v((usr_sem_t*) mushandle);
    }
    syscall_exit(0);
    return 0;
}