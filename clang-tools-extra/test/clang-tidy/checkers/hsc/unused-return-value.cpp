// RUN: %check_clang_tidy %s hsc-unused-return-value %t

#include <functional>

int compute();

class DeviceController {
public:
  bool startup();
  int fetchData();
};

struct Number {
  int Value;
  Number &operator=(const Number &);
};

Number operator+(const Number &, const Number &);

void rule_examples(DeviceController &Ctrl, std::function<int()> &Fn) {
  compute();
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used

  Ctrl.startup();
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used

  Ctrl.fetchData();
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used

  Fn();
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used

  auto Operation = []() { return 42; };
  Operation();
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used

  (void)compute();
  auto Result = compute();
  (void)Result;

  if (!Ctrl.startup()) {
  }
  int Data = Ctrl.fetchData();
  (void)Data;

  Number First{1};
  Number Second{2};
  First + Second;
  First = Second;
};
