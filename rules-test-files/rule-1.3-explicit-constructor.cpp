// Rule: HSCAP.1.3 - Explicit Single-Argument Constructors
// Description: Single-argument constructors should be declared explicit to
// avoid implicit conversions

// BAD - implicit conversion constructor
class String {
public:
  String(int count) { reserve(count); } // Single-argument, not explicit
private:
  void reserve(int capacity) {};
};

void useString(const String &s) {}

void test1() {
  useString(42); // Implicit conversion: int -> String
}

// GOOD - explicit single-argument constructor
class GoodString {
public:
  explicit GoodString(int count) { reserve(count); }

private:
  void reserve(int capacity) {};
};

void useGoodString(const GoodString &s) {}

void test2() {
  // useGoodString(42);  // Compile error - must use explicit conversion
  useGoodString(GoodString(42)); // OK - explicit
}

// Exception: copy/move constructors don't need explicit
class Vector {
public:
  Vector() = default;
  Vector(const Vector &other) {}     // Copy constructor - OK without explicit
  Vector(Vector &&other) noexcept {} // Move constructor - OK without explicit
};
