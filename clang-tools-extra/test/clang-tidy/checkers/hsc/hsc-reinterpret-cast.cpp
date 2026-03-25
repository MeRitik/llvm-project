// RUN: %check_clang_tidy %s hsc-reinterpret-cast %t

void badCast() {
  int x = 5;
  // CHECK-MESSAGES: :[[@LINE+1]]:13: warning: reinterpret_cast should be used with extreme caution
  double* ptr = reinterpret_cast<double*>(&x);
}

void goodCode() {
  int x = 5;
  void* ptr = &x;
}
