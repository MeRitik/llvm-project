// RUN: %check_clang_tidy %s hsc-virtual-member-pointer-cmp %t

#include <cstdint>

class Entity {
public:
  void action1();
  void action2();
  virtual void action3();
};

void testEntity() {
  if (&Entity::action1 != &Entity::action2) {}
  if (&Entity::action1 != nullptr) {}
  if (&Entity::action3 == &Entity::action2) {}
  // CHECK-MESSAGES: :[[@LINE-1]]:7: warning: comparison of a potentially virtual pointer to member function shall only be with nullptr [hsc-virtual-member-pointer-cmp]
  if (&Entity::action3 == nullptr) {}
}

void checkEntity(void (Entity::*Ptr)()) {
  if (Ptr == &Entity::action2) {}
  // CHECK-MESSAGES: :[[@LINE-1]]:7: warning: comparison of a potentially virtual pointer to member function shall only be with nullptr [hsc-virtual-member-pointer-cmp]
}

class Item {
public:
  void op1();
  void op2();
  virtual void op3(int32_t I);
};

void checkItem(void (Item::*Ptr)()) {
  if (Ptr == &Item::op2) {}
}

struct Incomplete;

void testIncomplete(void (Incomplete::*P1)(), void (Incomplete::*P2)()) {
  if (P1 == P2) {}
  // CHECK-MESSAGES: :[[@LINE-1]]:7: warning: comparison of a potentially virtual pointer to member function shall only be with nullptr [hsc-virtual-member-pointer-cmp]
}
