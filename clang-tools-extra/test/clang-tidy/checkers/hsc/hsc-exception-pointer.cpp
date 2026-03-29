// RUN: %check_clang_tidy %s hsc-exception-pointer %t -- -- -fcxx-exceptions

#include <cstdint>
#include <stdexcept>
#include <string>

// An exception object shall not have pointer type
// Description: If a pointer is thrown as an exception and it refers to a
// dynamically allocated object, it is unclear which function is responsible
// for destroying it and when. Throwing by value avoids this ownership ambiguity.

class Error { /* ... */ };

// Non-compliant --- pointer thrown:
void operation(int16_t code) {
    static Error staticErr;
    Error* dynamicErr = new Error;

    if (code > 10) {
        // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: exception should not be thrown as pointer; throw by value instead
        throw &staticErr;      // Non-compliant — pointer type thrown
    } else {
        // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: exception should not be thrown as pointer; throw by value instead
        throw dynamicErr;       // Non-compliant — pointer type thrown
    }
}

// Non-compliant --- throwing dynamically allocated exception:
void execute() {
    try {
        // riskyTask();
    } catch (...) {
        // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: exception should not be thrown as pointer; throw by value instead
        throw new std::runtime_error("task failed");   // Non-compliant — pointer type
    }
}

// Non-compliant --- throwing C-string literal pointer:
void verify(int num) {
    if (num < 0) {
        // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: exception should not be thrown as pointer; throw by value instead
        throw "invalid number";   // Non-compliant — const char* is a pointer type
    }
}

// Non-compliant --- throw pointer variable:
void badThrow() {
  const char* msg = "error";
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: exception should not be thrown as pointer; throw by value instead
  throw msg;
}

// Compliant --- throw by value:
void goodThrow() {
  throw std::runtime_error("code exceeds limit");   // Compliant — value type
}

// Compliant --- standard exception by value:
void goodExecute() {
    try {
        // riskyTask();
    } catch (...) {
        throw std::runtime_error("task failed");   // Compliant — thrown by value
    }
}

// Compliant --- use exception class:
void goodVerify(int num) {
    if (num < 0) {
        throw std::invalid_argument("invalid number");   // Compliant
    }
}

// Compliant --- throw std::string
void stringThrow() {
    throw std::string("error");  // Compliant — std::string is thrown by value, not pointer
}
