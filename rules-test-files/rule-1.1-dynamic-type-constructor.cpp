// Rule: HSCAP.1.1 - Dynamic Type in Constructor/Destructor
// Description: Virtual calls, typeid, and dynamic_cast should not be used in
// constructors or destructors because the object is not fully
// constructed/destructed yet.

#include <typeinfo>

// BAD - using typeid in constructor
class BadBase {
public:
  BadBase() {
    try {
      if (typeid(*this) == typeid(BadBase)) {
        init();
      }
    } catch (...) {
    }
  }
  virtual void init() {}
};

// BAD - using dynamic_cast in destructor
class BadDerived : public BadBase {
public:
  ~BadDerived() override {
    BadBase *pBase = dynamic_cast<BadBase *>(this);
    if (pBase)
      cleanup();
  }
  void cleanup() {}
};

// GOOD - use regular pointer/reference or type flag
class GoodBase {
private:
  bool isInitialized = false;

public:
  GoodBase() { isInitialized = init(); }
  virtual bool init() { return true; }
};

class GoodDerived : public GoodBase {
public:
  ~GoodDerived() override { cleanup(); }
  void cleanup() {}
};
