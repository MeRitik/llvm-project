// RUN: %check_clang_tidy %s hsc-exception-unfriendly-function %t

class Foo {
public:
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: destructors should be declared 'noexcept'
  ~Foo();
  
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: move constructors should be declared 'noexcept'
  Foo(Foo&&);
};

class GoodFoo {
public:
  ~GoodFoo() noexcept;
  GoodFoo(GoodFoo&&) noexcept;
};
