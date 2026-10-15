#include "process.h"
#include "memory.h"
#include "serial.h"
#include "string.h"

static pcb_t process_table[MAX_PROCESSES];
static pcb_t *current_process = NULL;
static uint32_t next_pid_counter = 0;

static uint32_t allocate_pid(void) {
  uint32_t candidate;
  int is_unique;
  int attempts = 0;

  while (attempts < MAX_PROCESSES) {
    candidate = next_pid_counter++;
    if (candidate == 0) {
      candidate = next_pid_counter++;
    }

    is_unique = 1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
      if (process_table[i].state != STATE_TERMINATED &&
          process_table[i].pid == candidate) {
        is_unique = 0;
        break;
      }
    }

    if (is_unique) {
      return candidate;
    }
    attempts++;
  }
  return 0;
}

void process_init(void) {
  for (int i = 0; i < MAX_PROCESSES; i++) {
    process_table[i].pid = 0;
    process_table[i].state = STATE_TERMINATED;
    process_table[i].name[0] = '\0';
    process_table[i].priority = 0;
    process_table[i].stack_base = NULL;
    process_table[i].esp = NULL;
    process_table[i].entry_point = NULL;
    process_table[i].exit_code = 0;
  }

  pcb_t *null_proc = &process_table[0];
  null_proc->pid = 0;
  null_proc->state = STATE_CURRENT;
  null_proc->priority = 0;
  strcpy(null_proc->name, "null_process");
  null_proc->stack_base = NULL;
  null_proc->esp = NULL;
  null_proc->entry_point = NULL;

  current_process = null_proc;
  next_pid_counter = 1;
}

pcb_t *process_get_current(void) {
  return current_process;
}

void process_set_current(pcb_t *proc) {
  current_process = proc;
}

int32_t process_create(const char *name, void (*entry_point)(void), uint32_t priority) {
  if (entry_point == NULL) {
    return -1;
  }

  int free_slot = -1;
  for (int i = 0; i < MAX_PROCESSES; i++) {
    if (process_table[i].state == STATE_TERMINATED) {
      free_slot = i;
      break;
    }
  }

  if (free_slot == -1) {
    return -2;
  }

  pcb_t *pcb = &process_table[free_slot];

  void *stack_mem = kstalloc(PROCESS_STACK_SIZE);
  if (stack_mem == NULL) {
    return -3;
  }

  uint32_t new_pid = allocate_pid();
  if (new_pid == 0) {
    ksfree(stack_mem);
    return -4;
  }

  pcb->pid = new_pid;
  pcb->priority = priority;
  pcb->entry_point = entry_point;
  pcb->stack_base = stack_mem;
  pcb->exit_code = 0;

  if (name) {
    int j = 0;
    while (name[j] != '\0' && j < MAX_PROCESS_NAME - 1) {
      pcb->name[j] = name[j];
      j++;
    }
    pcb->name[j] = '\0';
  } else {
    strcpy(pcb->name, "task");
  }

  uintptr_t stack_top_addr = (uintptr_t)stack_mem + PROCESS_STACK_SIZE;
  stack_top_addr = stack_top_addr & ~0x7;
  uint32_t *stk = (uint32_t *)stack_top_addr;

  *(--stk) = (uint32_t)(uintptr_t)entry_point;
  *(--stk) = 0x00000202;
  *(--stk) = 0;
  *(--stk) = 0;
  *(--stk) = 0;
  *(--stk) = 0;
  *(--stk) = 0;
  *(--stk) = 0;
  *(--stk) = 0;

  pcb->esp = stk;
  pcb->state = STATE_READY;

  return (int32_t)new_pid;
}

int32_t process_terminate(uint32_t pid, int32_t exit_code) {
  if (pid == 0) {
    return -1;
  }

  pcb_t *target = NULL;
  for (int i = 0; i < MAX_PROCESSES; i++) {
    if (process_table[i].state != STATE_TERMINATED && process_table[i].pid == pid) {
      target = &process_table[i];
      break;
    }
  }

  if (target == NULL) {
    return -2;
  }

  if (target->stack_base != NULL) {
    ksfree(target->stack_base);
    target->stack_base = NULL;
  }

  target->exit_code = exit_code;
  target->state = STATE_TERMINATED;
  target->pid = 0;
  target->esp = NULL;
  target->entry_point = NULL;
  target->name[0] = '\0';

  return 0;
}

int32_t process_set_state(uint32_t pid, process_state_t new_state) {
  pcb_t *target = NULL;
  for (int i = 0; i < MAX_PROCESSES; i++) {
    if (process_table[i].state != STATE_TERMINATED && process_table[i].pid == pid) {
      target = &process_table[i];
      break;
    }
  }

  if (target == NULL) {
    return -1;
  }

  process_state_t old_state = target->state;

  if (old_state == new_state) {
    return 0;
  }

  switch (old_state) {
  case STATE_READY:
    if (new_state != STATE_CURRENT && new_state != STATE_TERMINATED) {
      return -2;
    }
    break;

  case STATE_CURRENT:
    if (new_state != STATE_READY && new_state != STATE_TERMINATED) {
      return -2;
    }
    break;

  case STATE_TERMINATED:
    return -3;
  }

  target->state = new_state;

  if (new_state == STATE_CURRENT) {
    current_process = target;
  }

  return 0;
}