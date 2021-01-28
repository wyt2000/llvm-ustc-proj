// malloc 2^64 - 1
#include <stdlib.h>
#include <limits.h>
void test() {
  unsigned long long u = ULLONG_MAX;
  int *p = (int *) malloc(u);
  free(p);
}