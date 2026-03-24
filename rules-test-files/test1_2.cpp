#include <functional>
#include <string>

unsigned short compute();

class DeviceController {
public:
  bool startup();
  int fetchData();
};

struct Number {
  int v;
  Number &operator=(const Number &);
};

Number operator+(const Number &, const Number &);

void hscaa_1_2_examples(DeviceController &ctrl, std::function<int()> &fn) {
  // Non-compliant: discarded non-void function return
  compute();

  // Compliant: explicitly discarded
  (void)compute();

  // Compliant: value used
  auto r = compute();
  (void)r;

  // Non-compliant: discarded member function returns
  ctrl.startup();
  ctrl.fetchData();

  // Compliant: value used
  if (!ctrl.startup()) {
    return;
  }
  int data = ctrl.fetchData();
  (void)data;

  // Compliant exception per rule: overloaded operators in operator syntax
  Number a{1}, b{2};
  a + b;
  a = b;

  // Non-compliant per rule text: std::function and lambda calls
  fn();
  auto op = []() { return 42; };
  op();
}