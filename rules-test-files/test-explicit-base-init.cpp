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
  LeafBad() {} // should warn for Root/BranchA/BranchB missing explicit init
};

class LeafGood : public BranchA, public BranchB {
public:
  LeafGood() : Root(), BranchA(), BranchB() {} // should not warn
};
