// RUN: %check_clang_tidy %s hsc-overlapping-copy %t

void badCopy(void* src, void* dst, size_t size) {
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: memcpy with potentially overlapping regions
  memcpy(dst, src, size);
}

void goodCopy(void* src, void* dst, size_t size) {
  memmove(dst, src, size);
}
