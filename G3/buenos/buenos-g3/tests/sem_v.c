#include "tests/lib.h"

int main(void) {
    syscall_write();
    syscall_sem_p();

    //syscall_exit(0);
    return 0;
}
