// RUN: %check_clang_tidy %s hsc-c-style-functional-cast %t

#include <cstdint>

struct Item {
  Item(char c);
};

int compute();

void example1(Item obj) {
  auto const &ref = obj;
  // CHECK-MESSAGES: :[[@LINE+1]]:15: warning: C-style casts shall not be used
  Item *ptr = (Item *)&ref;
  (void)ptr;
}

void example2(int32_t num) {
  // CHECK-MESSAGES: :[[@LINE+1]]:17: warning: functional-notation casts shall not be used
  auto narrow = int8_t(num);
  (void)narrow;
}

void exampleVoidDiscard() {
  (void)compute();
}

void exampleCompliant(int32_t num) {
  auto narrow = int8_t{42};
  auto item = Item('c');
  (void)num;
  (void)narrow;
  (void)item;
}
