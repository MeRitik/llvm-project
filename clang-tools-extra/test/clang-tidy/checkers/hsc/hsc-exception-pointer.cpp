// RUN: %check_clang_tidy %s hsc-exception-pointer %t

void badThrow() {
  const char* msg = "error";
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: exception should not be thrown as pointer
  throw msg;
}

void goodThrow() {
  throw std::string("error");
}
