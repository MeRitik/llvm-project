// Rule: HSCAN.1.2 - Mixed Virtual/Non-Virtual Inheritance
// Description: A class should not directly inherit the same base class with
// both virtual and non-virtual inheritance

// BAD - mixed virtual and non-virtual inheritance of Base
class Base {
public:
  virtual ~Base() = default;
  virtual void method() {}
};

class Path1 : virtual public Base {}; // virtual inheritance
class Path2 : public Base {};         // non-virtual inheritance

// This class has two separate Base subobjects due to mixed inheritance
class Problematic : public Path1, public Path2 {}; // PROBLEM

// GOOD - consistent virtual inheritance
class GoodBase {
public:
  virtual ~GoodBase() = default;
  virtual void method() {}
};

class GoodPath1 : virtual public GoodBase {};
class GoodPath2 : virtual public GoodBase {};

class Solution : public GoodPath1, public GoodPath2 {}; // Single Base subobject

// GOOD - all non-virtual inheritance (if diamond is acceptable)
class NonVirtualBase {
public:
  virtual ~NonVirtualBase() = default;
  virtual void method() {}
};

class NVPath1 : public NonVirtualBase {};
class NVPath2 : public NonVirtualBase {};

// Note: This still creates two separate NonVirtualBase subobjects
// but it's consistent, making the issue more apparent to maintainers
