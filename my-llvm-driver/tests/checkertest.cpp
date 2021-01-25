void test() {
  int *p = new int[0]; // warn
  delete[] p;
}