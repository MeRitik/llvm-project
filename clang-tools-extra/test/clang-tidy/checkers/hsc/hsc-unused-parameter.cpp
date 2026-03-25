// RUN: %check_clang_tidy %s hsc-unused-parameter %t

void unusedNamedParam(int x, int y) {
  // CHECK-MESSAGES: :[[@LINE-1]]:31: warning: unused named parameter 'y' detected [hsc-unused-parameter]
  return y;
}

void usedParam(int x) {
  // CHECK-MESSAGES-NOT: warning
  return x;
}
