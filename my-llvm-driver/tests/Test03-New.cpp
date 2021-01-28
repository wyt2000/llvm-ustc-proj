// new nonpositive
int f(int n) {
  return n - 1;
}
void test() {
  int *p;
  p = new int[f(2)];
  p = new int[f(0)]; // warn
  delete [] p;
}