// RUN: %check_clang_tidy %s hsc-default-argument-override %t

#include <cstdint>

class Parent {
public:
  virtual void correct(int32_t Val = 0);
  virtual void wrong1(int32_t Val = 0);
  virtual void wrong2(int32_t Val);
};

class Child : public Parent {
public:
  void correct(int32_t Val = 0) override;
  void wrong1(int32_t Val = 1) override;
  // CHECK-MESSAGES: :[[@LINE-1]]:21: warning: overriding virtual function parameter shall not specify a different default argument [hsc-default-argument-override]
  void wrong2(int32_t Val = 2) override;
  // CHECK-MESSAGES: :[[@LINE-1]]:21: warning: overriding virtual function parameter shall not specify a different default argument [hsc-default-argument-override]
};

class ChildOk : public Parent {
public:
  void correct(int32_t Val = 0) override;
  void wrong1(int32_t Val) override;
};

class Payload {};

class Link {
public:
  virtual bool transmit(const Payload &P, int TimeoutMs = 1000);
};

class SecureLink : public Link {
public:
  bool transmit(const Payload &P, int TimeoutMs = 5000) override;
  // CHECK-MESSAGES: :[[@LINE-1]]:40: warning: overriding virtual function parameter shall not specify a different default argument [hsc-default-argument-override]
};
