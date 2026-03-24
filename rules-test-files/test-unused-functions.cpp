#include <iostream>

// Test rule 2.4: Functions with limited visibility should be used at least once

// Non-compliant: unused static function
static void unused_static_func() { std::cout << "This is never called\n"; }

// Compliant: static function that is called
static void used_static_func() { std::cout << "This is called\n"; }

// Non-compliant: unused function in anonymous namespace
namespace {
void unused_internal_func() { std::cout << "Internal unused\n"; }
} // namespace

// Compliant: function in anonymous namespace that is used
namespace {
void used_internal_func() { std::cout << "Internal used\n"; }

void call_internal() { used_internal_func(); }
} // namespace

// Test class with member functions
class Calculator {
private:
  // Non-compliant: unused private member function
  void unused_helper() { std::cout << "Helper unused\n"; }

  // Compliant: used private member function
  int used_helper() { return 42; }

public:
  Calculator() {
    // Constructor calls used_helper but not unused_helper
    int result = used_helper();
    std::cout << "Result: " << result << "\n";
  }
};

// Compliant: function pointer/address taken
static void utility_func() { std::cout << "Utility\n"; }

using FuncPtr = void (*)();
FuncPtr get_func() { return &utility_func; }

// Compliant: virtual member functions are excluded
class Base {
private:
  virtual void virtual_private() = 0;
};

// Compliant: special member functions (destructors, etc.)
struct Resource {
private:
  Resource(const Resource &) = delete;
  Resource &operator=(const Resource &) = delete;

  ~Resource() { std::cout << "Cleanup\n"; }
};

int main() {
  used_static_func();

  Calculator calc;

  FuncPtr func = get_func();
  func();

  return 0;
}
