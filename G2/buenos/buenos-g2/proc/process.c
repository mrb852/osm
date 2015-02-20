#include "proc/process.h"
#include "proc/elf.h"
#include "kernel/thread.h"
#include "kernel/assert.h"
#include "kernel/interrupt.h"
#include "kernel/config.h"
#include "fs/vfs.h"
#include "drivers/yams.h"
#include "vm/vm.h"
#include "vm/pagepool.h"


/** @name Process startup
 *
 * This module contains facilities for managing userland processes.
 */

process_control_block_t process_table[PROCESS_MAX_PROCESSES];

spinlock_t process_table_slock;

/**
 * Starts one userland process. The thread calling this function will
 * be used to run the process and will therefore never return from
 * this function. This function asserts that no errors occur in
 * process startup (the executable file exists and is a valid ecoff
 * file, enough memory is available, file operations succeed...).
 * Therefore this function is not suitable to allow startup of
 * arbitrary processes.
 *
 * @executable The name of the executable to be run in the userland
 * process
 */
void *process_start(const char *executable) {
    thread_table_t *my_entry;
    pagetable_t *pagetable;
    uint32_t phys_page;
    context_t user_context;
    uint32_t stack_bottom;
    elf_info_t elf;
    openfile_t file;

    int i;

    interrupt_status_t intr_status;

    my_entry = thread_get_current_thread_entry();

    /* If the pagetable of this thread is not NULL, we are trying to
       run a userland process for a second time in the same thread.
       This is not possible. */
    KERNEL_ASSERT(my_entry->pagetable == NULL);

    pagetable = vm_create_pagetable(thread_get_current_thread());
    KERNEL_ASSERT(pagetable != NULL);

    intr_status = _interrupt_disable();
    my_entry->pagetable = pagetable;
    _interrupt_set_state(intr_status);

    file = vfs_open((char *)executable);
    /* Make sure the file existed and was a valid ELF file */
    KERNEL_ASSERT(file >= 0);
    KERNEL_ASSERT(elf_parse_header(&elf, file));

    /* Trivial and naive sanity check for entry point: */
    KERNEL_ASSERT(elf.entry_point >= PAGE_SIZE);

    /* Calculate the number of pages needed by the whole process
       (including userland stack). Since we don't have proper tlb
       handling code, all these pages must fit into TLB. */
    KERNEL_ASSERT(elf.ro_pages + elf.rw_pages + CONFIG_USERLAND_STACK_SIZE
		  <= _tlb_get_maxindex() + 1);

    /* Allocate and map stack */
    for(i = 0; i < CONFIG_USERLAND_STACK_SIZE; i++) {
        phys_page = pagepool_get_phys_page();
        KERNEL_ASSERT(phys_page != 0);
        vm_map(my_entry->pagetable, phys_page, 
               (USERLAND_STACK_TOP & PAGE_SIZE_MASK) - i*PAGE_SIZE, 1);
    }

    /* Allocate and map pages for the segments. We assume that
       segments begin at page boundary. (The linker script in tests
       directory creates this kind of segments) */
    for(i = 0; i < (int)elf.ro_pages; i++) {
        phys_page = pagepool_get_phys_page();
        KERNEL_ASSERT(phys_page != 0);
        vm_map(my_entry->pagetable, phys_page, 
               elf.ro_vaddr + i*PAGE_SIZE, 1);
    }

    for(i = 0; i < (int)elf.rw_pages; i++) {
        phys_page = pagepool_get_phys_page();
        KERNEL_ASSERT(phys_page != 0);
        vm_map(my_entry->pagetable, phys_page, 
               elf.rw_vaddr + i*PAGE_SIZE, 1);
    }

    /* Put the mapped pages into TLB. Here we again assume that the
       pages fit into the TLB. After writing proper TLB exception
       handling this call should be skipped. */
    intr_status = _interrupt_disable();
    tlb_fill(my_entry->pagetable);
    _interrupt_set_state(intr_status);
    
    /* Now we may use the virtual addresses of the segments. */

    /* Zero the pages. */
    memoryset((void *)elf.ro_vaddr, 0, elf.ro_pages*PAGE_SIZE);
    memoryset((void *)elf.rw_vaddr, 0, elf.rw_pages*PAGE_SIZE);

    stack_bottom = (USERLAND_STACK_TOP & PAGE_SIZE_MASK) - 
        (CONFIG_USERLAND_STACK_SIZE-1)*PAGE_SIZE;
    memoryset((void *)stack_bottom, 0, CONFIG_USERLAND_STACK_SIZE*PAGE_SIZE);

    /* Copy segments */

    if (elf.ro_size > 0) {
	/* Make sure that the segment is in proper place. */
        KERNEL_ASSERT(elf.ro_vaddr >= PAGE_SIZE);
        KERNEL_ASSERT(vfs_seek(file, elf.ro_location) == VFS_OK);
        KERNEL_ASSERT(vfs_read(file, (void *)elf.ro_vaddr, elf.ro_size)
		      == (int)elf.ro_size);
    }

    if (elf.rw_size > 0) {
	/* Make sure that the segment is in proper place. */
        KERNEL_ASSERT(elf.rw_vaddr >= PAGE_SIZE);
        KERNEL_ASSERT(vfs_seek(file, elf.rw_location) == VFS_OK);
        KERNEL_ASSERT(vfs_read(file, (void *)elf.rw_vaddr, elf.rw_size)
		      == (int)elf.rw_size);
    }


    /* Set the dirty bit to zero (read-only) on read-only pages. */
    for(i = 0; i < (int)elf.ro_pages; i++) {
        vm_set_dirty(my_entry->pagetable, elf.ro_vaddr + i*PAGE_SIZE, 0);
    }

    /* Insert page mappings again to TLB to take read-only bits into use */
    intr_status = _interrupt_disable();
    tlb_fill(my_entry->pagetable);
    _interrupt_set_state(intr_status);

    /* Initialize the user context. (Status register is handled by
       thread_goto_userland) */
    memoryset(&user_context, 0, sizeof(user_context));
    user_context.cpu_regs[MIPS_REGISTER_SP] = USERLAND_STACK_TOP;
    user_context.pc = elf.entry_point;

    thread_goto_userland(&user_context);

    KERNEL_PANIC("thread_goto_userland failed.");

    return NULL;
}


void process_init() {
    spinlock_reset(&process_table_slock);

    // Initializing all process as DEAD in order to invalidate all processes
    for (int p = 0; p < PROCESS_MAX_PROCESSES; p++){
        process_table[p].state = PCB_FREE;
    }

    // Ready
    for (int p = 0; p < PROCESS_MAX_PROCESSES; p++){
        process_table[p].state = PCB_READY;
    }

    //thread_table_init();
}

process_id_t process_spawn(const char *executable) {


    interrupt_status_t intr_status;
    intr_status = _interrupt_disable();
    spinlock_acquire(&process_table_slock);


    process_id_t p_id = -1;
    for (int p = 0; p < PROCESS_MAX_PROCESSES; p++){
        if (process_table[p].state == PCB_READY){
            p_id = (process_id_t) p;
            break;
        }
    }

    spinlock_release(&process_table_slock);
    _interrupt_set_state(intr_status);

    if (p_id == -1) KERNEL_PANIC("No empty process slot! (process_table)");

    process_table[p_id].state = PCB_NEW;
    process_table[p_id].priority = PCB_PRIORITY_DEFAULT;
    //process_table[p_id].program_counter = 0;
    process_table[p_id].thread_id = thread_create((void *)process_start, (uint32_t) executable);
    thread_run(process_table[p_id].thread_id);
    process_table[p_id].state = PCB_RUNNING;

    return p_id;
}

/* Stop the process and the thread it runs in.  Sets the return value as
   well. */
void process_finish(int retval) {
    _interrupt_disable();

    spinlock_acquire(&process_table_slock);

    process_table[p_id].retval = retval;
    process_table[p_id].state = PCB_DYING;
    //thread_table[my_tid].state = THREAD_DYING;
    spinlock_release(&process_table_slock);

    _interrupt_enable();
    _interrupt_generate_sw0();

    KERNEL_PANIC("Not implemented: process_finish");
}

int process_join(process_id_t pid) {


    TID_t t = process_table[p_id].thread_id;
    while(thread_table[t].state != THREAD_DYING);

    int retval = -1337;

    return retval;
}

process_id_t process_get_current_process(void) {
    return thread_get_current_thread_entry()->process_id;
}

process_control_block_t *process_get_current_process_entry(void) {
    return &process_table[process_get_current_process()];
}

process_control_block_t *process_get_process_entry(process_id_t pid) {
    return &process_table[pid];
}

/** @} */
