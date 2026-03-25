// RUN: %check_clang_tidy %s hsc-remove-const %t

void badCast() {
  const int x = 5;
  // CHECK-MESSAGES: :[[@LINE+1]]:15: warning: const_cast should not be used
  int& ref = const_cast<int&>(x);
}

void goodCode() {
  int x = 5;
  const int& ref = x;
}
