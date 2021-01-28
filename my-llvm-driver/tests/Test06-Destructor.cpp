#include <stdlib.h>

void test() {
    std::exit(0); // no warn
}

class A {
public:
    ~A() {
        test();// warn
    }
    class B {
        ~B() {
          std::exit(1); // warn
        }
    };
};