// Rule: HSCAN.3.1 - Virtual/override/final Specifiers
// Description: Use virtual, override, and final keywords correctly in
// inheritance hierarchies

// BAD - no 'override' or 'final' specifier on virtual functions
class OldStyleBase {
public:
  virtual ~OldStyleBase() = default;
  virtual void process() {}
  virtual void update() {}
};

class OldStyleDerived : public OldStyleBase {
public:
  // Overrides base virtual functions but doesn't say so explicitly
  void process() {} // PROBLEM - missing override/final
  void update() {}  // PROBLEM - missing override/final
};

// GOOD - modern C++ with explicit specifiers
class ModernBase {
public:
  virtual ~ModernBase() = default;
  virtual void process() {}
  virtual void update() {}
};

class ModernDerived : public ModernBase {
public:
  ~ModernDerived() override = default;
  void process() override {} // Explicit override
  void update() final {}     // Declare final to prevent further overriding
};

// GOOD - prevent accidental overrides in final classes
class FinalImplementation final : public ModernBase {
public:
  void process() override {}
};

// PROBLEM - trying to override a final function
// class FailedDerived : public ModernDerived {
//   void update() override {}  // Compile error - function is final
// };
