// Rule: HSCAN.3.4 - Virtual Member Function Pointer Comparison
// Description: Comparing virtual member function pointers with non-nullptr is
// undefined behavior

// BAD - comparing virtual member function pointers
class Virtual {
public:
  virtual ~Virtual() = default;
  virtual void handler() {}
};

void checkVirtualPointer(Virtual *obj) {
  typedef void (Virtual::*HandlerPtr)();
  HandlerPtr ptr = &Virtual::handler;

  // PROBLEM - undefined behavior with virtual functions
  if (ptr == nullptr) {
  }
  if (ptr != nullptr) {
  }

  // Virtual member function pointers cannot be nullptr
  // The comparison is undefined
}

// GOOD - don't compare virtual member pointers
class Good {
public:
  virtual ~Good() = default;
  virtual void handler() {}
};

void checkGoodPointer(Good *obj) {
  // Instead of checking the function pointer,
  // just call it directly or use a wrapper
  obj->handler();
}

// GOOD - use non-virtual function pointers if nullability matters
class Mixed {
public:
  virtual ~Mixed() = default;
  virtual void virtualHandler() {}

  void (*nonVirtualHandler)() = nullptr; // Can be nullptr
};

void checkMixedPointer() {
  typedef void (Mixed::*HandlerPtr)();

  // This is OK - non-virtual function
  HandlerPtr ptr = &Mixed::Mixed; // Constructor pointer

  // For actual handlers, use the non-virtual version
  Mixed m;
  if (m.nonVirtualHandler != nullptr) {
    m.nonVirtualHandler();
  }
}
