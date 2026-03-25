// RUN: %check_clang_tidy %s hsc-advanced-memory-management %t

void badMemory() {
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: placement new should be used with caution
  int buffer[100];
  new(buffer) int(42);
}

void goodMemory() {
  auto ptr = std::make_unique<int>(42);
}
