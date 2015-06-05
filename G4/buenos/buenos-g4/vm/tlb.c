/*
 * TLB handling
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
 * $Id: tlb.c,v 1.6 2004/04/16 10:54:29 ttakanen Exp $
 *
 */

#include "kernel/panic.h"
#include "kernel/assert.h"
#include "vm/tlb.h"
#include "vm/pagetable.h"
#include "kernel/thread.h"

void tlb_close_process(char* error_message) {
  // Get the current thread entry
  thread_table_t* thread = thread_get_current_thread_entry();

  if (thread->user_context->status & USERLAND_ENABLE_BIT) {
    kwrite(error_message);
    thread_finish();
  }
  else {
    KERNEL_PANIC("unhandled tlb exception");
  }
}

int tlb_handle_exception()
{
  // Get the exception state
  tlb_exception_state_t state;
  _tlb_get_exception_state(&state);

  // Get the current thread
  thread_table_t* thread = thread_get_current_thread_entry();

  // Check if the states bad virtual address is odd
  uint32_t is_odd = state.badvaddr & 4096;
  kprintf("\n\ncount: %d\n\n", thread->pagetable->valid_count);
  // Try to find a valid mapping for the process
  int i = 0;
  for (; i < PAGETABLE_ENTRIES; ++i) {
    // get the i'th entry and check if it's valid to store / load
    tlb_entry_t* entry = &thread->pagetable->entries[i];

    uint32_t is_valid = is_odd ? entry->V1 : entry->V0;

    // write the entry if the states bad virtual page number is the same as the entries
    if ( is_valid && state.badvpn2 == entry->VPN2 ) {
      // update the entries ASID (thread id) to the exception states
      entry->ASID = state.asid;
      _tlb_write_random(entry);
      return 1;
    }
  }

  return 0;
}

void tlb_load_exception(void)
{
  if (tlb_handle_exception() == 0) {
    // Could not load entry so close
    tlb_close_process("TLB access violation: could not load entry\n");
  }
}

void tlb_store_exception(void)
{
  if (tlb_handle_exception() == 0) {
    // Could not store entry so close
    tlb_close_process("TLB access violation: could not store entry\n");
  }
}

void tlb_modified_exception(void)
{
  // Could not modify entry so close
  tlb_close_process("TLB access violation: could not modify entry\n");
}
