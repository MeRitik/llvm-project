// RUN: %check_clang_tidy %s hsc-default-argument-override %t

class Base {
public:
  virtual void method(int x = 5);
};

class Derived : public Base {
public:
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: default argument in override differs
  void method(int x = 10) override;
};

class GoodDerived : public Base {
public:
  void method(int x = 5) override;
};
