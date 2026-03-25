// Rule: HSCAN.3.2 - Default Arguments in Overrides
// Description: Default arguments in an overriding function must match the base
// class definition

// BAD - mismatched default arguments
class BadBase {
public:
  virtual ~BadBase() = default;
  virtual void configure(int timeout = 5000) {
    printf("Timeout: %d ms\n", timeout);
  }
};

class BadDerived : public BadBase {
public:
  // Different default value - PROBLEM
  void configure(int timeout = 3000) override {
    printf("Timeout: %d ms\n", timeout);
  }
};

void testBad() {
  BadBase *obj = new BadDerived();
  obj->configure(); // Uses base's default (5000), but derived prefers (3000)
  delete obj;
}

// GOOD - matching default arguments
class GoodBase {
public:
  virtual ~GoodBase() = default;
  virtual void configure(int timeout = 5000) {
    printf("Timeout: %d ms\n", timeout);
  }
};

class GoodDerived : public GoodBase {
public:
  // Same default value as base
  void configure(int timeout = 5000) override {
    printf("Timeout: %d ms\n", timeout);
  }
};

// ALTERNATIVE - explicitly specify in both places
class AlternativeDerived : public GoodBase {
public:
  // Or remove defaults from override entirely (use base class defaults)
  void configure(int timeout) override { printf("Timeout: %d ms\n", timeout); }
};
