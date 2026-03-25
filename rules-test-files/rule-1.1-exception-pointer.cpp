// Rule: HSCAS.1.1 - Exception with Pointer Type
// Description: Exceptions should be thrown by value, not by pointer

// BAD - throwing pointer
class BadException {
  std::string message;

public:
  BadException(const std::string &msg) : message(msg) {}
  const char *what() const { return message.c_str(); }
};

void badThrowFunction() {
  // PROBLEM - throwing pointer
  throw new BadException("An error occurred");
  // Stack trace will be corrupted, memory leak likely
}

void badCatch() {
  try {
    badThrowFunction();
  } catch (BadException *e) {
    // PROBLEM - must remember to delete
    printf("Error: %s\n", e->what());
    delete e; // Easy to forget!
  }
}

// GOOD - throwing by value
class GoodException : public std::runtime_error {
public:
  GoodException(const std::string &msg) : std::runtime_error(msg) {}
};

void goodThrowFunction() {
  // GOOD - throw by value
  throw GoodException("An error occurred");
}

void goodCatch() {
  try {
    goodThrowFunction();
  } catch (const GoodException &e) {
    printf("Error: %s\n", e.what());
  } catch (const std::exception &e) {
    printf("Unexpected error: %s\n", e.what());
  }
}

// CONVENTION - throw standard exceptions
void standardThrow() {
  throw std::invalid_argument("Invalid input");
  throw std::runtime_error("Runtime failure");
  throw std::out_of_range("Index out of bounds");
}
