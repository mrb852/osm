#include "tests/lib.h"
#include "proc/syscall.h"


#define NUM_THREADS 2

int main(void) {

    int i, pid[NUM_THREADS];
    char string[] = "[disk]nojob";

    // Start two jobs
    for (i = 0; i < NUM_THREADS; i++)
        pid[i] = syscall_exec(string);

    // Join them again
    for (i = 0; i < NUM_THREADS; i++)
        syscall_join(pid[i]);

    return 0;
}