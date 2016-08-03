
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef unsigned char m_op_t;
typedef unsigned int  m_exec_t;
typedef unsigned char m_boolean_t;
typedef signed   int  m_integer_t;

typedef unsigned int tf_size;
typedef unsigned char tf_type;
typedef int i32;

typedef struct m_stack_t {
  int size;
  int position;
  void* root;
} m_stack_t;


void m_stack_init(m_stack_t *stack) {
  stack->root = 0;
  stack->size = 0;
  stack->position = 0;
}
void m_stack_free(m_stack_t *stack) {
  if(stack->root) free(stack->root);
  stack->root = 0;
}



// ==================== stack

// return pointer at current stack position
void* m_stack_t_pt(m_stack_t *stack) {
  return stack->root + stack->position;
}

// grow stack if needed to fit extra size bytes.
// todo: make it shrink too?
void m_stack_t_ensure_size(m_stack_t *stack, int size) {
  // todo: too many reallocs here for big blobs in start of blob
  while(stack->position + size > stack->size) {
    int newsize = stack->size == 0 ? 16 : stack->size * 2;
    // printf("\x1b[31mstack resize from %d to %d\x1b[0m\n", stack->size, newsize);
    stack->root = realloc(stack->root, newsize);
    stack->size = newsize;
  }
}

int m_stack_t_isempty(m_stack_t *stack) {
  return stack->position <= 0;
}

void m_stack_push(m_stack_t *stack, void* data, tf_size len) {
  m_stack_t_ensure_size(stack, len);
  void* block = m_stack_t_pt(stack);
  memcpy(block, data, len);
  stack->position += len;
}

// place a simple blob of fixed size onto stack. this size much be known at pop-time!
void* m_stack_pop(m_stack_t *stack, tf_size len) {
  if(stack->position < len) {
    printf("error 6f13cf5a stack underflow: pop %d bytes, %d available\n", len, stack->position);
    exit(0);
  }
  stack->position -= len;
  return m_stack_t_pt(stack);
}




/* typedef struct tf_item { */
/*   int size; */
/*   int type; */
/*   char* data; */
/* } tf_item; */


typedef struct m_machine_t {
  m_stack_t integer;
  m_stack_t boolean;
  m_stack_t exec;
} m_machine_t;

void m_machine_init(m_machine_t *cpu) {
  m_stack_init(&cpu->integer);
  m_stack_init(&cpu->boolean);
  m_stack_init(&cpu->exec);
}

void m_machine_free(m_machine_t *cpu) {
  m_stack_free(&cpu->integer);
  m_stack_free(&cpu->boolean);
  m_stack_free(&cpu->exec);
}




m_integer_t m_stack_integer_length(m_machine_t *m) {
  return m->integer.position / sizeof(m_integer_t);
}
void m_stack_integer_push(m_machine_t *m, m_integer_t value) {
  m_stack_push(&m->integer, (void*)(m_integer_t*)(&value), sizeof(m_integer_t));
}
m_integer_t m_stack_integer_pop(m_machine_t *m) {
  return *((m_integer_t*)m_stack_pop(&m->integer, sizeof(m_integer_t)));
}


int m_stack_boolean_length(m_machine_t *m) {
  return m->boolean.position / sizeof(m_boolean_t);
}
void m_stack_boolean_push(m_machine_t *m, m_boolean_t value) {
  m_stack_push(&m->boolean, (void*)(m_boolean_t*)(&value), sizeof(m_boolean_t));
}
int m_stack_boolean_pop(m_machine_t *m) {
  return *((int*)m_stack_pop(&m->boolean, sizeof(m_boolean_t)));
}

m_exec_t m_stack_exec_length(m_machine_t *m) {
  // TODO
  return m->exec.position / sizeof(m_exec_t);
}
void m_stack_exec_push(m_machine_t *m, m_exec_t value) {
  m_stack_push(&m->exec, &value, sizeof(m_exec_t));
  //m_stack_push(&m->exec, (void*)&value->type, sizeof(value->type));
}
m_exec_t m_stack_exec_pop(m_machine_t *m) {
  return *((m_exec_t*)m_stack_pop(&m->exec, sizeof(m_exec_t)));
}


// ==================== type conversion

#define M_OP_MASK             0xf0000000
#define M_EXEC_TYPE_OP        0x10000000
#define M_EXEC_TYPE_INTEGER   0x20000000
#define M_EXEC_TYPE_BOOLEAN   0x30000000

m_exec_t m_convert_exec_from_op(m_op_t value) {
  return M_EXEC_TYPE_OP | value;
}

m_exec_t m_convert_exec_from_integer(m_integer_t value) {
  return M_EXEC_TYPE_INTEGER | value;
}
m_boolean_t m_convert_boolean_from_integer(m_integer_t value) {
  return value;
}
m_integer_t m_convert_integer_from_boolean(m_boolean_t value) {
  return value;
}

m_integer_t m_convert_integer_from_exec(m_exec_t value) {
  return (~M_EXEC_TYPE_INTEGER & value);
}
m_boolean_t m_convert_boolean_from_exec(m_exec_t value) {
  return ~M_EXEC_TYPE_BOOLEAN & value;
}
m_op_t m_convert_op_from_exec(m_exec_t value) {
  return ~M_EXEC_TYPE_OP & value;
}

int m_typeof_exec(m_exec_t value) {
  int ret = M_OP_MASK & value;
  if(ret == 0) printf("error: missing type mask for %x\n", value);
  return ret;
}


// ==================== instructions

#include "push-core.scm.gen.c"

// print raw bytes of stack, first item to pop on the right
void m_stack_print_hex(m_stack_t *stack) {
  int i;
  for(i = stack->position - 1 ; i >= 0 ; i--) {
    printf("%02x ", (((char*)stack->root)[i] & 0xFF));
  }
  printf("\n");
}

int main() {
  m_machine_t _cpu, *cpu = &_cpu;
  m_machine_init(cpu);


  m_stack_exec_push(cpu, m_convert_exec_from_integer(0x7));
  m_stack_exec_push(cpu, m_convert_exec_from_integer(0x15));

  m_stack_exec_push(cpu, m_convert_exec_from_op(OP_EXEC_IF));
  m_stack_exec_push(cpu, m_convert_exec_from_op(OP_INTEGER__GT_));
  m_stack_exec_push(cpu, m_convert_exec_from_integer(3));
  m_stack_exec_push(cpu, m_convert_exec_from_integer(2));

  printf("state: \n");
  printf("ints:   "); m_stack_print_hex(&cpu->integer);
  printf("booleans:   "); m_stack_print_hex(&cpu->boolean);
  printf("exec:   "); m_stack_print_hex(&cpu->exec);
  printf("\n");

  while (m_apply(cpu)) {
    printf("state: \n");
    printf("ints:   "); m_stack_print_hex(&cpu->integer);
    printf("booleans:   "); m_stack_print_hex(&cpu->boolean);
    printf("exec:   "); m_stack_print_hex(&cpu->exec);
    printf("\n");
  }

  m_machine_free(cpu);
  return 0;
}

// ((me.surrounding-smells integer.< code.do*times) (-1 0 1) ((vect.fromintegers) code.quote ((0 (vect.fromintegers ((exec.if (() integer.dup) 1 exec.if -1)))) ())))
