#include "tests/lib.h"

// A small program that writes some text into a file
int main() {

  // The file name
  const char* filename = "[disk]hello.txt";

  // create the file
  int file = syscall_create(filename, 40);

  // Check for errors
  if (file < 0) {
    return file;
  }

  // open the file
  file = syscall_open(filename);

  // Check for errors
  if (file < 0) {
    return file;
  }

  // write to the file
  syscall_write(file, "test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test test\n", 1000);

  // Close the file
  syscall_close(file);

  return 0;
}
