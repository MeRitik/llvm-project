// RUN: %check_clang_tidy %s hsc-dynamic-type-in-constructor %t

class Base {
public:
  virtual ~Base() = default;
};

class Derived : public Base {
public:
  Derived() {
    // CHECK-MESSAGES: :[[@LINE+1]]:5: warning: dynamic type operations (typeid)
    typeid(*this);
  }
};

class GoodDerived : public Base {
public:
  GoodDerived() {
    // CHECK-MESSAGES-NOT: warning
  }
};
