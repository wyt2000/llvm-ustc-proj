void f() {
  throw 1;
}

class A {
  A() {}
  ~A() { 
    f(); 
  } // warn
};