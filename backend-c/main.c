#include "pushc.c"

int main() {
  m_machine_t _cpu, *cpu = &_cpu;
  m_machine_init(cpu);

  int ticks = 0;
  while (m_apply(cpu) && ticks < 100000) {
    ticks++;
  }

  printf("end at %d ticks\n", ticks);
  printf("ints:   ");       m_integer_print(cpu);
  printf("booleans:   ");   m_boolean_print(cpu);
  printf("exec:   ");       m_exec_print(cpu);
  printf("\n");

  m_machine_free(cpu);
  return 0;
}
