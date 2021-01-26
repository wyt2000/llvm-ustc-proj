void f();

class A {
public:
  ~A() {
    f(); // warn
  }
};

class B {};

A a;

void f() {
  static B b;
}