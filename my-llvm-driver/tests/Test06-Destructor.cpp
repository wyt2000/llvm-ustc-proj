#include <stdlib.h>

void test() {
    std::exit(1); // warn
}

void g() {
    std::exit(0); // no warn
}

class A {
public:
    void f() {
        std::exit(0); // no warn
    }
    ~A() {
        test();
    }
    class B {
        ~B() {
          std::exit(1); // warn
        }
    };
};