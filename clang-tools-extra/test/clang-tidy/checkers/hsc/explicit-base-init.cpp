// RUN: %check_clang_tidy %s hsc-explicit-base-init %t

class Root {
public:
  Root() = default;
};

class BranchA : public virtual Root {
public:
  BranchA() : Root() {}
};

class BranchB : public virtual Root {
public:
  BranchB() : Root() {}
};

class LeafBad : public BranchA, public BranchB {
public:
  LeafBad() {}
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: constructor does not explicitly initialize base class 'BranchA' [hsc-explicit-base-init]
  // CHECK-MESSAGES: :[[@LINE-2]]:3: warning: constructor does not explicitly initialize base class 'BranchB' [hsc-explicit-base-init]
  // CHECK-MESSAGES: :[[@LINE-3]]:3: warning: constructor does not explicitly initialize base class 'Root' [hsc-explicit-base-init]
};

class LeafGood : public BranchA, public BranchB {
public:
  LeafGood() : Root(), BranchA(), BranchB() {}
};

class EmptyBase {};

class EmptyDerived : public EmptyBase {
public:
  EmptyDerived() {}
};
