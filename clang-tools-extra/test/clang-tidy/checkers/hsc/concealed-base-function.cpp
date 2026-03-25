// RUN: %check_clang_tidy %s hsc-concealed-base-function %t

class Parent {
public:
  void action(int);
  void action(char);
};

class Child : public Parent {
public:
  void action(float);
  // CHECK-MESSAGES: :[[@LINE-1]]:8: warning: function 'action' conceals a function inherited from an accessible base class [hsc-concealed-base-function]
};

class ChildUsing : public Parent {
public:
  using Parent::action;
  void action(float);
};

class ParentVirtual {
public:
  virtual void run(int);
};

class ChildOverride : public ParentVirtual {
public:
  void run(int) override;
};

class PrivateChild : private Parent {
public:
  void action(float);
};
