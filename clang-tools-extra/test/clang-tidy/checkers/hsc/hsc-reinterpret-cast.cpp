// RUN: %check_clang_tidy %s hsc-reinterpret-cast %t -- -- -std=c++17

#include <cstddef>
#include <cstdint>

void badPtrToUnrelatedPtr(std::uint8_t *narrow) {
  // CHECK-MESSAGES: :[[@LINE+1]]:{{[0-9]+}}: warning: reinterpret_cast shall not be used
  auto wide = reinterpret_cast<std::uint32_t *>(narrow);
  (void)wide;
}

void badPtrToDifferentObjectType() {
  std::uint32_t val = 0;
  // CHECK-MESSAGES: :[[@LINE+1]]:{{[0-9]+}}: warning: reinterpret_cast shall not be used
  auto ptr = reinterpret_cast<std::uint16_t *>(&val);
  (void)ptr;
}

void goodBytePointerException(float num) {
  // CHECK-MESSAGES-NOT: warning: reinterpret_cast shall not be used
  auto bytes = reinterpret_cast<std::byte const *>(&num);
  (void)bytes;
}

void goodCharPointerException(int value) {
  // CHECK-MESSAGES-NOT: warning: reinterpret_cast shall not be used
  auto p = reinterpret_cast<char const *>(&value);
  (void)p;
}

void goodUintptrException(float num) {
  // CHECK-MESSAGES-NOT: warning: reinterpret_cast shall not be used
  auto addr = reinterpret_cast<std::uintptr_t>(&num);
  (void)addr;
}
