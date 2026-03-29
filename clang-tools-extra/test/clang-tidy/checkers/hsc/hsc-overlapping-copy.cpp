// RUN: %check_clang_tidy %s hsc-overlapping-copy %t -- -- -std=c++17

#include <array>
#include <cstdint>
#include <cstring>

void badOverlappingMemcpy(std::array<std::int16_t, 20> &arr) {
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: memcpy on overlapping or same object; use memmove instead
  std::memcpy(&arr[0], &arr[1], 10u * sizeof(arr[0]));
}

void goodOverlappingMemmove(std::array<std::int16_t, 20> &arr) {
  std::memmove(&arr[0], &arr[1], 10u * sizeof(arr[0]));
}

void goodNonOverlappingMemcpy(std::array<std::int16_t, 20> &arr) {
  // CHECK-MESSAGES-NOT: warning: memcpy on overlapping or same object; use memmove instead
  std::memcpy(&arr[0], &arr[5], 5u * sizeof(arr[0]));
}

void badUnionMemberCopy() {
  union {
    std::int16_t small;
    std::int32_t large;
  } data = {0};

  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: copy between members of the same union object is not allowed
  data.large = data.small;
}
