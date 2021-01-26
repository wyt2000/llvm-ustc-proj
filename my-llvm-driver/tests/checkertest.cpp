#include <stdlib.h>
void f(int, int);
int g(void *);
int h() __attribute__((noreturn));

void test() {
  // It is possible that 'malloc(1)' is called first,
  // then 'h()', that is (or calls) noreturn and eventually
  // 'g()' is never called.
  f(g(malloc(1)), h()); // warn: 'g()' may never be called.
}