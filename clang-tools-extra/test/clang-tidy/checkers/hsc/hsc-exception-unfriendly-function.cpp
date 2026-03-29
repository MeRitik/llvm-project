// RUN: %check_clang_tidy %s hsc-exception-unfriendly-function %t -- -- -std=c++17

class Handler {
public:
  Handler() {}

  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: move constructor should be declared 'noexcept'
  Handler(Handler &&other) {}

  // CHECK-MESSAGES: :[[@LINE+1]]:12: warning: move assignment operator should be declared 'noexcept'
  Handler &operator=(Handler &&other);

  // CHECK-MESSAGES: :[[@LINE+1]]:15: warning: function 'swap' should be declared 'noexcept'
  friend void swap(Handler &, Handler &);
};

// CHECK-MESSAGES: :[[@LINE+1]]:9: warning: non-local object of type 'Handler' should use a noexcept constructor
Handler instance;

class Fragile {
public:
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: destructor should be declared 'noexcept'
  ~Fragile() noexcept(false) {}
};

class Safe {
public:
  // CHECK-MESSAGES-NOT: warning: destructor should be declared 'noexcept'
  ~Safe() {}
};

class Handler1 {
public:
  // CHECK-MESSAGES-NOT: warning: move constructor should be declared 'noexcept'
  Handler1(Handler1 &&other) noexcept {}

  // CHECK-MESSAGES-NOT: warning: move assignment operator should be declared 'noexcept'
  Handler1 &operator=(Handler1 &&other) & noexcept;

  // CHECK-MESSAGES-NOT: warning: function 'swap' should be declared 'noexcept'
  friend void swap(Handler1 &, Handler1 &) noexcept;

  // CHECK-MESSAGES-NOT: warning: destructor should be declared 'noexcept'
  ~Handler1() noexcept(true) {}
};
