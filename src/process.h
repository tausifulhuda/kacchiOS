#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

#define MAX_PROCESSES      16
#define MAX_PROCESS_NAME   32
#define PROCESS_STACK_SIZE 2048

typedef enum {
  STATE_READY = 0,
  STATE_CURRENT,
  STATE_TERMINATED
} process_state_t;

typedef struct process {
  uint32_t pid;
  char name[MAX_PROCESS_NAME];
  process_state_t state;
  uint32_t priority;
  void *stack_base;
  uint32_t *esp;
  void (*entry_point)(void);
  int32_t exit_code;
} pcb_t;

void process_init(void);
pcb_t* process_get_current(void);
void process_set_current(pcb_t* proc);

int32_t process_create(const char* name, void (*entry_point)(void), uint32_t priority);
int32_t process_terminate(uint32_t pid, int32_t exit_code);
int32_t process_set_state(uint32_t pid, process_state_t new_state);

#endif