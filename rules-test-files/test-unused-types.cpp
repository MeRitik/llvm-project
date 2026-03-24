#include <iostream>

// Test rule 2.3: Types with limited visibility should be used at least once

// Non-compliant: unused type alias in function scope
void example1() {
  using Alias = int; // Unused - should warn
  std::cout << "Example 1\n";
}

// Non-compliant: unused struct in function scope
void example2() {
  struct LocalData {
    int x;
    int y;
  }; // Unused - should warn

  std::cout << "Example 2\n";
}

// Compliant: type alias marked with [[maybe_unused]]
void example3() {
  using Alias [[maybe_unused]] = double;
  std::cout << "Example 3\n";
}

// Compliant: struct is used
void example4() {
  struct LocalData {
    int x;
    int y;
  };

  LocalData data{1, 2}; // Used
  std::cout << "Data: " << data.x << ", " << data.y << "\n";
}

// Non-compliant: unused type in anonymous namespace
namespace {
struct UnusedHelper {
  int compute() { return 42; }
}; // Unused - should warn
} // namespace

// Compliant: type in anonymous namespace is used
namespace {
struct UsedHelper {
  int compute() { return 42; }
};

void use_helper() {
  UsedHelper h;
  std::cout << "Helper: " << h.compute() << "\n";
}
} // namespace

int main() {
  example1();
  example2();
  example3();
  example4();

  return 0;
}
