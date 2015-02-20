#ifndef BUENOS_PROC_PROCESS
#define BUENOS_PROC_PROCESS

typedef int process_id_t;

// Made private
//void *process_start(const char *executable);

#define USERLAND_STACK_TOP 0x7fffeffc

#define PROCESS_PTABLE_FULL -1
#define PROCESS_ILLEGAL_JOIN -2

#define PROCESS_MAX_PROCESSES 32


/* ########################################################################## */
#define PCB_FREE 0x00
#define PCB_NEW 0x01
#define PCB_READY 0x02
#define PCB_WAITING 0x03
#define PCB_ZOMBIE 0x04
#define PCB_RUNNING 0x05

// TERMINATED and DEAD is the same
#define PCB_TERMINATED 0x06
#define PCB_DEAD 0x06

#define PCB_PRIORITY_HIGH 0x01
#define PCB_PRIORITY_DEFAULT 0x02
#define PCB_PRIORITY_LOW 0x03


typedef struct {
    int state;
    int counter;
    int priority;
    int thread_id;
    int retval;
    //int process_pointer;
    //int process_burst_time;

    // registers?
    // memory?
    //int program_base_address;
    //int program_limit_address;
    // Accounting infomation
    // I/O status information


} process_control_block_t;

/* ########################################################################## */


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
