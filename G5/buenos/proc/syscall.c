/*
 * System calls.
 *
 * Copyright (C) 2003 Juha Aatrokoski, Timo Lilja,
 *   Leena Salmela, Teemu Takanen, Aleksi Virtanen.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: syscall.c,v 1.3 2004/01/13 11:10:05 ttakanen Exp $
 *
 */
#include "fs/vfs.h"
#include "kernel/cswitch.h"
#include "kernel/halt.h"
#include "kernel/panic.h"
#include "lib/libc.h"
#include "proc/io.h"
#include "proc/syscall.h"
#include "proc/process.h"
#include "kernel/assert.h"
#include "drivers/device.h"
#include "drivers/gcd.h"
#include "proc/usr_sem.h"

#define A0 user_context->cpu_regs[MIPS_REGISTER_A0]
#define A1 user_context->cpu_regs[MIPS_REGISTER_A1]
#define A2 user_context->cpu_regs[MIPS_REGISTER_A2]
#define A3 user_context->cpu_regs[MIPS_REGISTER_A3]
#define V0 user_context->cpu_regs[MIPS_REGISTER_V0]

process_id_t syscall_exec(const char *filename) {
  return process_spawn(filename);
}

void syscall_exit(int retval) {
  process_finish(retval);
}

int syscall_join(process_id_t pid) {
  return process_join(pid);
}

openfile_t syscall_open(char *pathname) {
  openfile_t result = vfs_open(pathname);
  if(result < 0) {
    switch(result) {
      case VFS_UNUSABLE:
        kprintf("Error opening file. File system unusable.\n");
        break;
      case VFS_INVALID_PARAMS:
        kprintf("Error opening file: Invalid params\n");
        break;
      case VFS_LIMIT:
        kprintf("Error opening file: Too many programs are running\n");
        break;
      case VFS_NO_SUCH_FS:
        kprintf("Error opening file: No such file exists\n");
        break;
      default:
        kprintf("Error opening file: unknown error\n");
        break;
    }

  }
  return result + 2;
}

int syscall_close(openfile_t file) {
  int result = vfs_close(file - 2);
  if (result < 0) {
    switch(result) {
      case VFS_UNUSABLE:
        kprintf("Error closing file. File system unusable.\n");
      break;
      case VFS_NOT_OPEN:
        kprintf("Error closing file: File not open\n");
      break;
      default:
        kprintf("Error closing file: unknown error\n");
      break;
    }
    KERNEL_PANIC("Error closing file");
  }
  return result;
}

int syscall_seek(openfile_t file, int position) {
  int result = vfs_seek(file - 2, position);
  if (result < 0) {
    switch(result) {
      case VFS_UNUSABLE:
        kprintf("Error seeking in file. File system unusable.\n");
      break;
      case VFS_INVALID_PARAMS:
        kprintf("Error seeking in file: Invalid params\n");
      break;
      case VFS_NOT_OPEN:
        kprintf("Error seeking in file: File not open\n");
      break;
      default:
        kprintf("Error seeking in file: unknown error\n");
      break;
    }

  }
  return result;
}

int syscall_read(openfile_t file, void *buffer, int bufsize) {
  int result = vfs_read(file - 2, buffer, bufsize);
  if (result < 0) {
    switch(result) {
      case VFS_UNUSABLE:
        kprintf("Error reading file. File system unusable.\n");
      break;
      case VFS_INVALID_PARAMS:
        kprintf("Error reading file: Invalid params\n");
      break;
      case VFS_NOT_OPEN:
        kprintf("Error reading file: File not open\n");
      break;
      default:
        kprintf("Error reading file: unknown error\n");
      break;
    }

  }
  return result;
}

int syscall_write(openfile_t file, void *buffer, int datasize) {
  int result = vfs_write(file - 2, buffer, datasize);
  if (result < 0) {
    switch(result) {
      case VFS_UNUSABLE:
        kprintf("Error writing file. File system unusable.\n");
      break;
      case VFS_INVALID_PARAMS:
        kprintf("Error writing file: Invalid params\n");
      break;
      case VFS_NOT_OPEN:
        kprintf("Error writing file: File not open\n");
      break;
      default:
        kprintf("Error writing file: unknown error\n");
      break;
    }

  }

  return result;
}

int syscall_create(const char *pathname, int size) {
  int result = vfs_create(pathname, size);
  if (result < 0) {
    switch(result) {
      case VFS_UNUSABLE:
        kprintf("Error creating file. File system unusable.\n");
      break;
      case VFS_INVALID_PARAMS:
        kprintf("Error creating file: Invalid params\n");
      break;
      case VFS_NO_SUCH_FS:
        kprintf("Error creating file: No such filesystem\n");
      break;
      default:
        kprintf("Error writing file: unknown error\n");
      break;
    }

  }

  return result;
}

int syscall_remove(const char *pathname) {
  int result = vfs_remove(pathname);
  if (result < 0) {
    switch(result) {
      case VFS_UNUSABLE:
        kprintf("Error removing file. File system unusable.\n");
      break;
      case VFS_INVALID_PARAMS:
        kprintf("Error removing file: Invalid params\n");
      break;
      case VFS_NO_SUCH_FS:
        kprintf("Error removing file: No such filesystem\n");
      break;
      default:
        kprintf("Error writing file: unknown error\n");
      break;
    }

  }

  return result;
}

int syscall_tell(openfile_t file) {
  int result = vfs_tell(file - 2);
  if (result < 0) {
    switch(result) {
      case VFS_UNUSABLE:
        kprintf("Error telling file's position. File system unusable.\n");
      break;
      case VFS_NOT_OPEN:
        kprintf("Error telling file's position: File not open\n");
      break;
      default:
        kprintf("Error writing file: unknown error\n");
      break;
    }

  }

  return result;
}

/**
 * Handle system calls. Interrupts are enabled when this function is
 * called.
 *
 * @param user_context The userland context (CPU registers as they
 * where when system call instruction was called in userland)
 */
void syscall_handle(context_t *user_context)
{
  /* When a syscall is executed in userland, register a0 contains
   * the number of the syscall. Registers a1, a2 and a3 contain the
   * arguments of the syscall. The userland code expects that after
   * returning from the syscall instruction the return value of the
   * syscall is found in register v0. Before entering this function
   * the userland context has been saved to user_context and after
   * returning from this function the userland context will be
   * restored from user_context.
   */
  switch (A0) {
  case SYSCALL_HALT:
    halt_kernel();
    break;
  case SYSCALL_READ: {
      int filehandle = (int)A1;
      if (filehandle == FILEHANDLE_STDIN || filehandle == FILEHANDLE_STDOUT
        || filehandle == FILEHANDLE_STDERR) {
        V0 = io_read((int) A1, (void*) A2, (int) A3);
      } else {
        V0 = syscall_read((openfile_t)A1, (void *)A2, (int)A3);
      }
    }
    break;
  case SYSCALL_WRITE: {
      int filehandle = (int)A1;
      if (filehandle == FILEHANDLE_STDIN || filehandle == FILEHANDLE_STDOUT
        || filehandle == FILEHANDLE_STDERR) {
        V0 = io_write((int) A1, (void*) A2, (int) A3);
      } else {
        V0 = syscall_write((openfile_t)A1, (void *)A2, (int)A3);
      }
    }
    break;
  case SYSCALL_EXEC:
    V0 = syscall_exec((char*) A1);
    break;
  case SYSCALL_EXIT:
    syscall_exit((int) A1);
    break;
  case SYSCALL_JOIN:
    V0 = syscall_join((process_id_t) A1);
    break;
  case SYSCALL_OPEN:
    V0 = syscall_open((char *) A1);
  break;
  case SYSCALL_CLOSE:
    V0 = syscall_close((openfile_t) A1);
  break;
  case SYSCALL_SEEK:
    V0 = syscall_seek((openfile_t)A1, (int)A2);
  break;
  case SYSCALL_CREATE:
    V0 = syscall_create((const char *)A1, (int)A2);
  break;
  case SYSCALL_REMOVE:
    V0 = syscall_remove((const char *)A1);
  break;
  case SYSCALL_TELL:
    V0 = syscall_tell((openfile_t)A1);
  break;
  case SYSCALL_SEM_OPEN:
    V0 = (uint32_t) usr_sem_open((char*) A1, A2);
    break;
  case SYSCALL_SEM_PROCURE:
    V0 = usr_sem_p((usr_sem_t*) A1);
    break;
  case SYSCALL_SEM_VACATE:
    V0 = usr_sem_v((usr_sem_t*) A1);
    break;
  case SYSCALL_SEM_DESTROY:
    V0 = usr_sem_destroy((usr_sem_t*) A1);
    break;
  default:
    KERNEL_PANIC("Unhandled system call\n");
  }

  /* Move to next instruction after system call */
  user_context->pc += 4;
}
