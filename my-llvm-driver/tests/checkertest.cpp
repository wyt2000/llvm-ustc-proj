#include <cstdlib>

class A {
public:
  ~A() {
    std::exit(1); // warn
  }
};