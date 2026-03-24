#include <iostream>
#include <memory>
#include <vector>

// RAII guard class - should NOT trigger warning
class ScopedGuard {
public:
  ScopedGuard() { std::cout << "Guard acquired\n"; }
  ~ScopedGuard() { std::cout << "Guard released\n"; }
};

// Simple resource holder
class Resource {
private:
  int *data;

public:
  Resource() : data(nullptr) {}
  ~Resource() { /* cleanup */ }
};

// Test namespace scope
namespace TestNamespace {
// Unused namespace-scoped variable - SHOULD WARN
static int unusedGlobal = 42;

// Used namespace-scoped variable - should NOT warn
static int usedGlobal = 100;
int getGlobal() { return usedGlobal; }
} // namespace TestNamespace

int main() {
  // Unused local variable - SHOULD WARN
  int unusedInt = 10;

  // Unused local variable - SHOULD WARN
  double unusedDouble = 3.14;

  // Used local variable - should NOT warn
  int usedInt = 20;
  std::cout << "Used: " << usedInt << "\n";

  // RAII guard - should NOT warn (has user-provided destructor)
  ScopedGuard guard;

  // Resource with destructor - should NOT warn
  Resource res;

  // Variable marked with [[maybe_unused]] - should NOT warn
  [[maybe_unused]] int explicitlyUnused = 999;

  // Unused pointer - SHOULD WARN
  int *ptr = nullptr;

  // Used pointer - should NOT warn
  int *usedPtr = &usedInt;
  std::cout << "Pointer value: " << *usedPtr << "\n";

  // Unused vector - SHOULD WARN (no user-provided ctor/dtor in this context)
  std::vector<int> unusedVec = {1, 2, 3};

  // Used vector - should NOT warn
  std::vector<int> usedVec = {4, 5, 6};
  std::cout << "Vector size: " << usedVec.size() << "\n";

  return 0;
}
