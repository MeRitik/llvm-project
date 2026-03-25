// RUN: %check_clang_tidy %s hsc-explicit-single-argument-constructor %t

class BadCtor {
public:
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: single-argument constructors should be declared explicit
  BadCtor(int value);
};

class GoodCtor {
public:
  explicit GoodCtor(int value);
};

class CopyCtorOK {
public:
  CopyCtorOK(const CopyCtorOK& other);
};
