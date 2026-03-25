// RUN: %check_clang_tidy %s hsc-self-assignment %t

class Foo {
private:
  int value;
public:
  Foo& operator=(const Foo& other) {
    // CHECK-MESSAGES: :[[@LINE+2]]:3: warning: assignment operator should check for self-assignment
    value = other.value;
    return *this;
  }
};

class GoodFoo {
private:
  int value;
public:
  GoodFoo& operator=(const GoodFoo& other) {
    if (this != &other) {
      value = other.value;
    }
    return *this;
  }
};
