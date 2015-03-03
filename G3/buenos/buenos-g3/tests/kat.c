#include "tests/lib.h"

int main() {
    int kathandle = (int)syscall_sem_open("kat",2);

    int i;
    for(i = 0; i < 10; i++){
        syscall_sem_p((usr_sem_t*) kathandle);
        printf("kat\n\n");
        syscall_sem_v((usr_sem_t*) kathandle);
    }

    syscall_exit(0);
    return 0;
}
