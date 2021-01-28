void f() {
  throw 1;
}

class A {
  A() {}
  ~A() { 
    throw 1;
    f(); 
  } // warn
};