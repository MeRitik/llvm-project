// Rule: HSCAV.6.2 - Dynamic Memory Management
// Description: Use smart pointers instead of raw new/delete or C memory
// functions

// BAD - raw new/delete
void badMemoryManagement() {
  int *ptr1 = new int(42);
  int *ptr2 = new int[100];

  delete ptr1;
  delete[] ptr2;
  // Risk: exceptions between new and delete, double delete, wrong
  // delete/delete[]
}

void badCMemory() {
  int *ptr = (int *)malloc(sizeof(int) * 10);
  // ... use ptr ...
  free(ptr);
  // Risk: size mismatches, forgotten free, double free
}

// GOOD - use smart pointers
void goodMemoryManagement() {
  auto ptr1 = std::make_unique<int>(42);
  auto ptr2 = std::make_unique<int[]>(100);

  // Automatic cleanup on scope exit
  // No manual delete needed
  // Exception-safe
}

// GOOD - shared ownership when needed
void sharedOwnership() {
  std::shared_ptr<Resource> ptr1 = std::make_shared<Resource>();
  {
    std::shared_ptr<Resource> ptr2 = ptr1; // Shared ownership
  } // ptr2 destroyed, but Resource still alive (ptr1 holds reference)
  // ptr1 destroyed, Resource cleaned up
}

// GOOD - std::make_unique / std::make_shared allocation
class ComplexObject {
private:
  std::vector<int> data;
  std::string name;

public:
  ComplexObject(const std::string &n) : name(n), data(100) {}
};

void createObject() {
  auto obj = std::make_unique<ComplexObject>("important");
  // Exception-safe allocation and initialization
  // Automatic cleanup on scope or exception
}
