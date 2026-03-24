// RUN: %check_clang_tidy %s hsc-unused-variable %t

namespace {
class Item {};
Item Obj;
// CHECK-MESSAGES: [[@LINE-1]]:6: warning: variable 'Obj' is never used
}

void process() {
  int Counter = 42;
  // CHECK-MESSAGES: [[@LINE-1]]:7: warning: variable 'Counter' is never used
}

struct Guard {
  Guard();
  ~Guard();
};

void protectedSection() {
  Guard G;
}

void check(int Val) {
  [[maybe_unused]] bool Flag = Val > 0;
  (void)Val;
}

void aggregate() {
  double Total = 0.0;
  // CHECK-MESSAGES: [[@LINE-1]]:10: warning: variable 'Total' is never used
}