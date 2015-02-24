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
#include "kernel/cswitch.h"
#include "proc/syscall.h"
#include "proc/process.h"
#include "kernel/halt.h"
#include "kernel/panic.h"
#include "lib/libc.h"
#include "kernel/assert.h"
#include "drivers/device.h"
#include "drivers/gcd.h"

#define A0 user_context->cpu_regs[MIPS_REGISTER_A0]
#define A1 user_context->cpu_regs[MIPS_REGISTER_A1]
#define A2 user_context->cpu_regs[MIPS_REGISTER_A2]
#define A3 user_context->cpu_regs[MIPS_REGISTER_A3]
#define V0 user_context->cpu_regs[MIPS_REGISTER_V0]

gcd_t* get_tty() {
    device_t *dev;
    gcd_t *gcd;

    /* Find system console (first tty) */
    dev = device_get(YAMS_TYPECODE_TTY, 0);
    KERNEL_ASSERT(dev != NULL);
    gcd = (gcd_t *)dev->generic_device;
    KERNEL_ASSERT(gcd != NULL);
    return gcd;
}

uint32_t tty_read(int filehandle, void* buffer, int length) {
    /* Only support standard in for now. */
    KERNEL_ASSERT(filehandle == FILEHANDLE_STDIN);
    gcd_t *gcd = get_tty();
    return gcd->read(gcd, buffer, length);
}

uint32_t tty_write(int filehandle, void* buffer, int length) {
    /* Only support standard out for now. */
    KERNEL_ASSERT(filehandle == FILEHANDLE_STDOUT);
    gcd_t *gcd = get_tty();
    return gcd->write(gcd, buffer, length);
}

process_id_t syscall_exec(const char *filename) {
    return process_spawn(filename);
}

void syscall_exit(int retval) {
    process_finish(retval);
}

int syscall_join(process_id_t pid) {
    return process_join(pid);
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
    case SYSCALL_READ:
        V0 = tty_read((int) A1, (void*) A2, (int) A3);
        break;
    case SYSCALL_WRITE:
        V0 = tty_write((int) A1, (void*) A2, (int) A3);
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
    default:
        KERNEL_PANIC("Unhandled system call\n");
    }

    /* Move to next instruction after system call */
    user_context->pc += 4;
}
