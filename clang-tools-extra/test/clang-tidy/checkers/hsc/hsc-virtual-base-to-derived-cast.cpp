// RUN: %check_clang_tidy %s hsc-virtual-base-to-derived-cast %t -- -- -fcxx-exceptions

class Root {
public:
  virtual ~Root() = default;
};

class Leaf : public virtual Root {};

void badReinterpret(Root *pRoot) {
  // CHECK-MESSAGES: :[[@LINE+1]]:17: warning: casting from a virtual base class to a derived class must use dynamic_cast
  Leaf *pLeaf1 = reinterpret_cast<Leaf *>(pRoot);
  (void)pLeaf1;
}

void goodDynamic(Root *pRoot) {
  Leaf *pLeaf2 = dynamic_cast<Leaf *>(pRoot);
  (void)pLeaf2;
}

void goodUnrelatedReinterpret() {
  int x = 5;
  // CHECK-MESSAGES-NOT: warning: casting from a virtual base class to a derived class must use dynamic_cast
  double *ptr = reinterpret_cast<double *>(&x);
  (void)ptr;
}
