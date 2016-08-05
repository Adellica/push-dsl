
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <assert.h>

// ==================== static types

typedef unsigned char m_boolean_t;
typedef signed   int  m_integer_t;
typedef unsigned char m_op_t;

// ==================== dynamic types

typedef struct m_obj_t {
  unsigned char __type_; // careful when accessing this! make sure lower 2 bits are 0
  union {
    struct {
      struct m_obj_t *car;
      struct m_obj_t *cdr;
    } pair;
  } data;
} m_obj_t;

/* no GC so truely "unlimited extent" */
m_obj_t *alloc_object(void) {
  m_obj_t *obj;

  obj = malloc(sizeof(m_obj_t));
  printf("                                        ALLOC: %p\n", obj);
  if (obj == NULL) {
    fprintf(stderr, "out of memory\n");
    exit(1);
  }
  return obj;
}


// immediate masks. if lower bits are 00, it's pointer to an object
//#define M_IMASK_PAIR      0b00
#define M_IMASK_INTEGER   0b01
#define M_IMASK_BOOLEAN   0b10
#define M_IMASK_OP        0b11

#define M_TYPE_INTEGER   M_IMASK_INTEGER
#define M_TYPE_BOOLEAN   M_IMASK_BOOLEAN
#define M_TYPE_OP        M_IMASK_OP
#define M_TYPE_PAIR      0b100
#define M_TYPE_NIL       0b101

m_obj_t the_empty_list = { M_TYPE_NIL };

char is_the_empty_list(m_obj_t *obj) {
  return obj == &the_empty_list;
}

m_obj_t *m_obj_from_integer(int value) {
  return (void*)((long)(value << 2) | M_IMASK_INTEGER);
}
int m_obj_to_integer(m_obj_t *obj) {
  assert(((long)obj & M_IMASK_INTEGER) == M_IMASK_INTEGER);
  return (long)obj >> 2;
}

m_obj_t *m_obj_from_boolean(m_boolean_t value) {
  return (void*)(((long)value << 2) | M_IMASK_BOOLEAN);
}
m_boolean_t m_obj_to_boolean(m_obj_t *obj) {
  assert(((long)obj & M_IMASK_BOOLEAN) == M_IMASK_BOOLEAN);
  return (long)obj >> 2;
}

m_obj_t *m_obj_from_op(m_op_t value) {
  return (void*)(((long)value << 2) | M_IMASK_OP);
}
m_op_t m_obj_to_op(m_obj_t *obj) {
  assert(((long)obj & M_IMASK_OP) == M_IMASK_OP);
  return (long)obj >> 2;
}

m_obj_t *m_obj_cons(m_obj_t *car, m_obj_t *cdr) {
  m_obj_t *obj;
    
  obj = alloc_object();
  obj->__type_ = M_TYPE_PAIR;
  obj->data.pair.car = car;
  obj->data.pair.cdr = cdr;
  return obj;
}

/* char is_pair(m_obj_t *obj) { */
/*   return obj->type == M_TYPE_PAIR; */
/* } */

m_obj_t *m_obj_car(m_obj_t *pair) {
  return pair->data.pair.car;
}

void m_obj_set_car(m_obj_t *obj, m_obj_t* value) {
  obj->data.pair.car = value;
}

m_obj_t *m_obj_cdr(m_obj_t *pair) {
  return pair->data.pair.cdr;
}

void m_obj_set_cdr(m_obj_t *obj, m_obj_t* value) {
  obj->data.pair.cdr = value;
}


int m_typeof_obj(m_obj_t *obj) {

  switch ((long)obj & 0b11) {
  case M_IMASK_BOOLEAN: return M_IMASK_BOOLEAN;
  case M_IMASK_INTEGER: return M_IMASK_INTEGER;
  case M_IMASK_OP: return M_IMASK_OP;
  case 0:
    switch (obj->__type_) {
    case M_TYPE_NIL: return M_TYPE_NIL;
    case M_TYPE_PAIR: return M_TYPE_PAIR;
    }
    // it must be a pointer to an m_obj_t
  }
  printf("error: could not find type for object at %p\n", obj);
  exit(-1);
}

void write(m_obj_t *obj);

void write_pair(m_obj_t *pair) {
  m_obj_t *car_obj;
  m_obj_t *cdr_obj;
    
  car_obj = m_obj_car(pair);
  cdr_obj = m_obj_cdr(pair);
  write(car_obj);
  if (m_typeof_obj(cdr_obj) == M_TYPE_PAIR) {
    printf(" ");
    write_pair(cdr_obj);
  }
  else if (m_typeof_obj(cdr_obj) == M_TYPE_NIL) {

  }
  else {
    printf(" . ");
    write(cdr_obj);
  }
}

char *lookup (m_op_t);

void write(m_obj_t *obj) {
  //printf("== write object: %p\n", obj);
  switch ((long)obj & 0b11) {
  case M_IMASK_BOOLEAN:
    printf("#%c", m_obj_to_boolean(obj) ? 't' : 'f');
    break;
  case M_IMASK_INTEGER:
    printf("%ld", m_obj_to_integer(obj));
    break;
  case M_IMASK_OP:
    printf("%s", lookup(m_obj_to_op(obj)));
    break;
  case 0:
    if(obj == &the_empty_list) {
      printf("()");
      break;
    } else {
      // it must be a pointer to an m_obj_t
      printf("(");
      write_pair(obj);
      printf(")");
      break;
    }
  }
}





typedef m_obj_t   *m_exec_t;

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

void m_stack_push(m_stack_t *stack, void* data, tf_size len) {
  m_stack_t_ensure_size(stack, len);
  void* block = m_stack_t_pt(stack);
  memcpy(block, data, len);
  stack->position += len;
}

// push the value of the pointer - don't push the data the p points to!
void m_stack_push_pointer(m_stack_t *s, void *p) {
  //printf("pushing pointer %p\n", p);
  m_stack_push(s, &p, sizeof(void*));
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


// push the value of the pointer - don't push the data the p points to!
void *m_stack_pop_pointer(m_stack_t *s) {
  void* p = *((void**)m_stack_pop(s, sizeof(void*)));
  //printf("poping pointer %x\n", p);
  return p;
}



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

int m_stack_exec_length(m_machine_t *m) {
  return m->exec.position / sizeof(m_exec_t);
}
void m_stack_exec_push(m_machine_t *m, m_exec_t value) {
  m_stack_push(&m->exec, &value, sizeof(m_exec_t));
}
m_exec_t m_stack_exec_pop(m_machine_t *m) {
  return *((m_exec_t*)m_stack_pop(&m->exec, sizeof(m_exec_t)));
}


// ==================== type conversion


m_boolean_t m_boolean_from_integer(m_integer_t value) {
  return value && 1;
}
m_integer_t m_integer_from_boolean(m_boolean_t value) {
  return value;
}




// ==================== instructions

#include "push-core.scm.gen.c"

// print raw bytes of stack, first item to pop on the right
void m_stack_print_hex(m_stack_t *stack) {
  int i;
  for(i = stack->position - 1 ; i >= 0 ; i--) {
    printf("%02x ", (((char*)stack->root)[i] & 0xFF));
    if(i % 8 == 0) printf("  ");
  }
  printf("\n");
}

// kinda dangerous if applied to a non-m_obj_t stack
void _m_stack_print_obj(m_stack_t *stack) {
  int i;
  int len = (stack->position / sizeof(m_obj_t*));
  for(i = len - 1 ; i >= 0 ; i--) {
    write((m_obj_t*)(((long*)stack->root)[i]));
    printf(" ");
  }
  printf("\n");
}

void m_exec_print(m_machine_t *m) {
  _m_stack_print_obj(&m->exec);
}

void m_integer_print(m_machine_t *m) {
  int i;
  for(i = (m->integer.position / sizeof(m_integer_t)) - 1 ; i >= 0 ; i--) {
    printf("%8d ", (((m_integer_t*)m->integer.root)[i]));
  }
  printf("\n");
}


int m_apply (m_machine_t *m) {
    if (m_stack_exec_length(m) < 1) {
        return 0;
    }
    m_exec_t instruction = m_stack_exec_pop(m);
    int ret;
    m_obj_t *next;
    m_stack_t tmps;

  retry:
    switch (m_typeof_obj(instruction)) {
        case M_TYPE_OP:
            ret = m_apply_op(m, m_obj_to_op(instruction));
            break;
        case M_TYPE_PAIR:
          next = m_obj_cdr(instruction);
          m_stack_init(&tmps);
          // push each element onto stack in reverse order
          while (!is_the_empty_list(next)) {
            // TODO: next.refcount--
            if(m_typeof_obj(next) != M_TYPE_PAIR) {
              m_stack_push_pointer(&tmps, next);
              break;
            }
            m_stack_push_pointer(&tmps, m_obj_car(next));
            next = m_obj_cdr(next);
            // handle improper lists for efficiency (no ensure-list needed)
          }
          while(tmps.position > 0) {
            m_stack_exec_push(m, m_stack_pop_pointer(&tmps));
          }
          instruction = m_obj_car(instruction);
          m_stack_free(&tmps);
          goto retry;
          break;
        case M_TYPE_NIL:
            
            break;
        default:
            ret = m_apply_literal(m, instruction);
            break;
    }
    return ret;
}


// example program skeleton
int main_test() {
  m_machine_t _cpu, *cpu = &_cpu;
  m_machine_init(cpu);

  int ticks = 0;
  while (m_apply(cpu)) { ticks++; }

  printf("end at %d ticks\n", ticks);
  printf("ints:   ");       m_integer_print(cpu);
  printf("booleans:   ");   m_stack_print_hex(&cpu->boolean);
  printf("exec:   ");       m_exec_print(cpu);
  printf("\n");

  m_machine_free(cpu);
  return 0;
}
