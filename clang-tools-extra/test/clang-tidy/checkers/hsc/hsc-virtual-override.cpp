// RUN: %check_clang_tidy %s hsc-mixed-virtual-inheritance %t

class Base {};

class Derived1 : virtual public Base {};

class Derived2 : public Base {};

// CHECK-MESSAGES: :[[@LINE+1]]:7: warning: mixing virtual and non-virtual inheritance
class Problematic : public Derived1, public Derived2 {};
