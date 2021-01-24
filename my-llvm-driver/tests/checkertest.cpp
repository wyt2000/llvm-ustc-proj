#include <stdlib.h>
class A {
public:
  ~A() {
    std::exit(1); // warn
  }
  
  class B {
  public:
    ~B() {
      std::exit(1); // warn
    }
  };
  
};

int main() {
  exit(1);
}