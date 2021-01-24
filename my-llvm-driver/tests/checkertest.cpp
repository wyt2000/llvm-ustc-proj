#include <stdlib.h>
void f() {
}
class A {
public:
  ~A() {
    f();
    exit(1);
  }
  
  class B {
  public:
    ~B() {
      exit(1);
    }
  };
  
};

int main() {
  exit(1);
}