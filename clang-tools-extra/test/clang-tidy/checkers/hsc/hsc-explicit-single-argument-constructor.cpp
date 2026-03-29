// RUN: %check_clang_tidy %s hsc-explicit-single-argument-constructor %t

#include <cstdint>

class Wrapper {
public:
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: constructors callable with a single argument should be declared explicit
  Wrapper(int32_t Val);
};

class Container {
public:
  explicit Container(int32_t Val);
  Container(const Container &C);
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: conversion operators should be declared explicit
  operator int32_t() const;
  explicit operator bool() const;
};

class Config {
public:
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: constructors callable with a single argument should be declared explicit
  Config(int32_t A, int32_t B = 0);
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: constructors callable with a single argument should be declared explicit
  Config(char A = 'a', int32_t B = 0);
  Config(char A, char B);
};

void apply(Config C);

void testConfig() {
  apply(0);
}

class Identifier {
public:
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: constructors callable with a single argument should be declared explicit
  Identifier(int Id);
};

void removeEntry(Identifier Id);

void testIdentifier() {
  removeEntry(42);
}

class IdentifierGood {
public:
  explicit IdentifierGood(int Id);
};

void removeEntryGood(IdentifierGood Id);

void testIdentifierGood() {
  removeEntryGood(IdentifierGood{42});
}
