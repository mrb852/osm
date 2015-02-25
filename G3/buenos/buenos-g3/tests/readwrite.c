/*
 * Userland read and write test
 */

#include "tests/lib.h"

#define READ_LENGTH 58

int main() {
    char text_read[READ_LENGTH];
    int read_length;
    char hello[] = "HELLO THERE!  WRITE SOMETHING!\n";
    char you_wrote[] = "YOU WROTE: ";

    syscall_write(stdout, hello, sizeof(hello));
    read_length = syscall_read(stdin, text_read, READ_LENGTH);
    syscall_write(stdout, you_wrote, sizeof(you_wrote));
    syscall_write(stdout, text_read, read_length);
    syscall_write(stdout, "\n", 1);

    syscall_halt();
    return 0;
}
