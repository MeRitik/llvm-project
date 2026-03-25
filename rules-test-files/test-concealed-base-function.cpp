class Parent {
public:
  void action(int);
  void action(char);
};

class Child : public Parent {
public:
  void action(float); // should warn: conceals base overload set
};

class ChildOk : public Parent {
public:
  using Parent::action;
  void action(float); // should not warn
};
