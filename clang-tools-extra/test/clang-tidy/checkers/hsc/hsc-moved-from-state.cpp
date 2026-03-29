// RUN: %check_clang_tidy %s hsc-moved-from-state %t

#include <memory>
#include <string>
#include <utility>

size_t measure(std::string text) {
  std::string copy = std::move(text);
  // CHECK-MESSAGES: :[[@LINE+1]]:10: warning: object used after being moved-from
  return text.size();
}

void duplicate(std::string text) {
  std::string first = std::move(text);
  // CHECK-MESSAGES: :[[@LINE+1]]:24: warning: object used after being moved-from
  std::string second = text;
}

void reset(std::string text) {
  std::string first = std::move(text);
  text = std::string{};
  (void)text.size();
}

void allowUniquePtr(std::unique_ptr<int> ptr) {
  auto copy = std::move(ptr);
  if (ptr)
    (void)*ptr;
}

template <typename T> void consume(T &&) {}

template <typename T> void forwardUse(T &&item) {
  consume(std::forward<T>(item));
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: object used after being moved-from
  ++item;
}
