// malloc nonpositive
#include <stdlib.h>
void test() {
  int *p = (int *) malloc(0);
  free(p);
}