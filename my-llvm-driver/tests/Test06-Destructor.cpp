#include <stdlib.h>

class A {
public:
    ~A() {
        std::exit(1); // warn
    }
    A() {
        std::exit(0); // no warn
    }
    class B {
        ~B() {
          std::exit(1); // warn
      }
    };
};

void test() {
    std::exit(0); // no warn
}