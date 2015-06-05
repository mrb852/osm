Buenos G5 handout, version 1.0
==============================

Changes from Buenos G3 handout:
  + We have solved G3.
  + VFS_NAME_LENGTH = 16 is now also the maximum volume name length.
  + The solution to G1 can now be found in proc/io.[hc].
  + Added VFS system calls for G5 to proc/syscall.h.
  + Made the VFS implementation less prone to panic.

Changes from Buenos 1.1.2:

  + We have solved G1 and G2 (changes in `proc/syscall.c`, `proc/process.h`,
    `proc/process.c`, and `init/main.c`), including some tests.
  + We have defined the `usr_sem_t` type in `tests/lib.h`.
  + We have included a userland library in `tests/lib.h` and `tests/lib.c`.
  + We have defined the SYSCALL_SEM_ syscall ids in `proc/syscall.h`.
