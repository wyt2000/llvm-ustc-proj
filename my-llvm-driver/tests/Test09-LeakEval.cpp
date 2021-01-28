#include <stdlib.h>
void f(int, int);
int g(int *);
int h() {
    exit(0);
};

void test() {
  // It is possible that 'new int' is called first,
  // then 'h()', that throws an exception and eventually
  // 'g()' is never called.
  f(g(new int), h()); // warn: 'g()' may never be called.
}
