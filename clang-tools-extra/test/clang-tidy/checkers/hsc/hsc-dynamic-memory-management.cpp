// RUN: %check_clang_tidy %s hsc-dynamic-memory-management %t

void badMemory() {
  // CHECK-MESSAGES: :[[@LINE+1]]:10: warning: use std::make_unique
  int* ptr = new int(42);
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: use smart pointers
  delete ptr;
}

void goodMemory() {
  auto ptr = std::make_unique<int>(42);
}
