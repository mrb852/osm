#include "tests/lib.h"
#include "proc/syscall.h"

int main(void) {
  int l = 64;
  char buffer[l];
  syscall_read(FILEHANDLE_STDIN, buffer, l);
  syscall_write(FILEHANDLE_STDOUT, buffer, l);
  return 0;
}
