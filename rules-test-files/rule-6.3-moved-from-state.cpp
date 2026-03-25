// Rule: HSCBC.6.3 - Moved-From State Usage
// Description: Variables should not be accessed after being moved, except for
// specific types like unique_ptr

// BAD - using moved-from variable
void badMovedUsage() {
  std::vector<int> original = {1, 2, 3};
  std::vector<int> copy = std::move(original);

  // PROBLEM - original is in moved-from state
  for (int value : original) { // Undefined behavior!
    printf("%d ", value);
  }

  size_t size = original.size(); // Might be 0, might be garbage
}

void badStringUsage() {
  std::string source = "Hello";
  std::string dest = std::move(source);

  // PROBLEM - source is in moved-from state
  const char *cstr = source.c_str(); // Likely empty, but undefined
}

// GOOD - don't use moved-from variables
void goodMovedUsage() {
  std::vector<int> original = {1, 2, 3};
  std::vector<int> copy = std::move(original);

  // Use 'copy' instead of 'original'
  for (int value : copy) {
    printf("%d ", value);
  }
}

// GOOD - reassign after move if you need to reuse
void reassignAfterMove() {
  std::vector<int> vec = {1, 2, 3};

  processVector(std::move(vec)); // vec is moved-from

  vec = {4, 5, 6}; // Re-initialize with new values
  processVector(std::move(vec));
}

// EXCEPTION - unique_ptr is still valid but nullptr after move
void uniquePtrAfterMove() {
  std::unique_ptr<Resource> ptr1 = std::make_unique<Resource>();
  std::unique_ptr<Resource> ptr2 = std::move(ptr1);

  // Safe to check ptr1
  if (ptr1) {
    ptr1->doSomething(); // Won't execute
  } else {
    printf("ptr1 is nullptr after move\n"); // This executes
  }
}

// PATTERN - transfer ownership and mark as moved
void transferOwnership(std::unique_ptr<Resource> &ptr) {
  auto temp = std::move(ptr);

  if (!ptr) {
    printf("ptr was successfully moved\n"); // Safe check
  }

  // temp is now responsible for the resource
}
