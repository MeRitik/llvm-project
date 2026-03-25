// RUN: %check_clang_tidy %s hsc-if-else-if-termination %t

void badControl(int x) {
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: if-else-if chain should be terminated with an else
  if (x == 1) {
  } else if (x == 2) {
  }
}

void goodControl(int x) {
  if (x == 1) {
  } else if (x == 2) {
  } else {
  }
}
