// RUN: %check_clang_tidy %s hsc-virtual-member-pointer-cmp %t

class Base {
public:
  virtual void method();
};

void test(Base& obj) {
  typedef void (Base::*MethodPtr)();
  MethodPtr ptr = &Base::method;
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: comparing virtual member function pointers
  if (ptr != nullptr) {}
}
