#ifndef BUENOS_PROC_PROCESS
#define BUENOS_PROC_PROCESS

typedef int process_id_t;

// Made private
//void *process_start(const char *executable);

#define USERLAND_STACK_TOP 0x7fffeffc

#define PROCESS_PTABLE_FULL -1
#define PROCESS_ILLEGAL_JOIN -2

#define PROCESS_MAX_PROCESSES 32


// State definitions
#define PCB_FREE 0x00
#define PCB_NEW 0x01
#define PCB_READY 0x02
#define PCB_WAITING 0x03
#define PCB_DYING 0x04
#define PCB_RUNNING 0x05
#define PCB_DEAD 0x06

// Definition of structure for PCB
typedef struct {
    int state;
    int thread_id;
    int retval;
    char executable[256]; // Defined in vfs.h
} process_control_block_t;


/* Initialize the process table.  This must be called during kernel startup
   before any other process-related calls. */
void process_init();

/* Run process in a new thread.  Returns the PID of the new process. */
process_id_t process_spawn(const char *executable);

/* Stop the process and the thread it runs in.  Sets the return value as
   well. */
void process_finish(int retval);

/* Wait for the given process to terminate, returning its return value.  This
 * will also mark its process table entry as free.  Only works on child
 * processes */
int process_join(process_id_t pid);

/* Return PID of current process. */
process_id_t process_get_current_process(void);

/* Return PCB of current process. */
process_control_block_t *process_get_current_process_entry(void);

#endif
