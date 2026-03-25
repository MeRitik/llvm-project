// Rule: HSCAI.2.1-2.6 - Cast Restrictions
// Description: Minimize and restrict use of type casts; prefer safer
// alternatives

// BAD - C-style casts (multiple problems)
void badCStyleCasts() {
  int x = 42;
  double *danger1 = (double *)&x; // Reinterpret cast danger

  const int y = 10;
  int *danger2 = (int *)&y; // Removes const - undefined behavior

  void *generic = &x;
  float *danger3 = (float *)generic; // Arbitrary type punning
}

// BAD - const_cast to remove const-protection
void badConstCast() {
  void printValue(const int *ptr);

  const int value = 42;
  printValue(&value);

  int *mutable_ptr = const_cast<int *>(&value); // PROBLEM
  *mutable_ptr = 100; // Undefined behavior on const object
}

// BAD - reinterpret_cast for type conversion
void badReinterpretCast() {
  int x = 42;
  int *ptr = &x;

  double d = *reinterpret_cast<double *>(&x); // Garbage data as double

  std::string str = "hello";
  int *bad = reinterpret_cast<int *>(&str); // Totally wrong
}

// GOOD - static_cast for known conversions
void goodStaticCast() {
  int x = 42;
  double d = static_cast<double>(x); // Clear intent, compile-time checked

  void *generic = &x;
  // DON'T: int* ptr = static_cast<int*>(generic);  // Still unsafe but intent
  // is clear

  Base *base = new Derived();                      // OK polymorphic downcast
  Derived *derived = static_cast<Derived *>(base); // OK - known relationship
}

// GOOD - dynamic_cast for safe polymorphic casts
void goodDynamicCast() {
  Base *base = getBasePtr();

  Derived *derived = dynamic_cast<Derived *>(base);
  if (derived) {
    derived->derivedMethod(); // Safe - only if type matches
  } else {
    printf("Not a Derived object\n");
  }
}

// GOOD - const_cast only for const-correctness interface issues
void goodConstCast() {
  // If a C function wrongly takes non-const but promises not to modify:
  extern void legacyFunction(int *ptr); // Bad C API

  const int data[] = {1, 2, 3};
  legacyFunction(const_cast<int *>(data)); // Justified exception
  // But: accessing data afterwards could be dangerous
}

// BETTER - avoid casts by proper design
class SafeDesign {
private:
  int value = 0;

public:
  int getValue() const { return value; } // Access without cast

  void setValue(int v) { value = v; } // Modification through interface

  // No need for casts - proper accessors
};

// SUMMARY:
// AVOID:     (type) expr                    // C-style cast
// AVOID:     const_cast<T>(expr)            // Removing const
// AVOID:     reinterpret_cast<T>(expr)      // Reinterpreting bits
// PREFER:    static_cast<T>(expr)           // Known conversions
// PREFER:    dynamic_cast<T*>(ptr)          // Safe polymorphic casts
// BEST:      Proper design avoiding casts
