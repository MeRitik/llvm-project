// RUN: %check_clang_tidy %s hsc-dynamic-type-in-constructor %t

#include <memory>
#include <typeinfo>

class Level2;

class Polymorphic {
public:
  virtual ~Polymorphic();
  virtual void init();
  virtual void setup() = 0;

  Polymorphic() {
    // CHECK-MESSAGES: :[[@LINE+1]]:5: warning: typeid on a polymorphic expression should not be used in a constructor or destructor
    typeid(*this);
    // CHECK-MESSAGES: :[[@LINE+1]]:5: warning: virtual function call should not be used in a constructor or destructor
    init();
    // CHECK-MESSAGES: :[[@LINE+1]]:5: warning: virtual function call should not be used in a constructor or destructor
    setup();
    // CHECK-MESSAGES: :[[@LINE+1]]:5: warning: dynamic_cast should not be used in a constructor or destructor
    dynamic_cast<Polymorphic *>(this);
    Polymorphic::init();
    typeid(Polymorphic);
  }

  ~Polymorphic() {
    // CHECK-MESSAGES: :[[@LINE+1]]:5: warning: virtual function call should not be used in a constructor or destructor
    init();
  }
};

class NonPolymorphic {
public:
  NonPolymorphic() {
    typeid(*this);
  }
};

class Level1 {
public:
  explicit Level1(Level2 *L);
  virtual ~Level1();
  virtual void init();
};

class Level2 : public Level1 {
public:
  Level2() : Level1(this) {}
};

Level1::Level1(Level2 *L) {
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: virtual function call should not be used in a constructor or destructor
  init();
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: virtual function call should not be used in a constructor or destructor
  L->init();
}

class Component {
public:
  virtual void configure() = 0;
  Component() {
    // CHECK-MESSAGES: :[[@LINE+1]]:5: warning: virtual function call should not be used in a constructor or destructor
    configure();
  }
};

class Component2 {
public:
  virtual void configure() = 0;
  Component2() {}
  static std::unique_ptr<Component2> create();
};
