// RUN: %check_clang_tidy %s hsc-void-integer-to-pointer-cast %t -- -- -std=c++17

#include <cstdint>

struct Data {
  std::int32_t x;
  std::int32_t y;
};

enum class E : std::uint32_t { A = 0u };

void convert(void *raw, std::int32_t addr, E e) {
  // CHECK-MESSAGES: :[[@LINE+1]]:14: warning: casting from integral, enumerated, or void pointer type to a pointer type is prohibited
  auto d1 = static_cast<Data *>(raw);
  (void)d1;

  // CHECK-MESSAGES: :[[@LINE+1]]:14: warning: casting from integral, enumerated, or void pointer type to a pointer type is prohibited
  auto d2 = reinterpret_cast<Data *>(addr);
  (void)d2;

  // CHECK-MESSAGES: :[[@LINE+1]]:13: warning: casting from integral, enumerated, or void pointer type to a pointer type is prohibited
  void *v = reinterpret_cast<void *>(addr);
  (void)v;

  // CHECK-MESSAGES: :[[@LINE+1]]:13: warning: casting from integral, enumerated, or void pointer type to a pointer type is prohibited
  auto d3 = reinterpret_cast<Data *>(e);
  (void)d3;

  // CHECK-MESSAGES: :[[@LINE+1]]:14: warning: casting from integral, enumerated, or void pointer type to a pointer type is prohibited
  auto d4 = (Data *)raw;
  (void)d4;
}

void goodVoidQualifierChange(void *raw) {
  // CHECK-MESSAGES-NOT: warning: casting from integral, enumerated, or void pointer type to a pointer type is prohibited
  auto constRaw = const_cast<void const *>(raw);
  (void)constRaw;
}
