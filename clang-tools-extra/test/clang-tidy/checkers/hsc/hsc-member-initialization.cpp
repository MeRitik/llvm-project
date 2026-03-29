// RUN: %check_clang_tidy %s hsc-member-initialization %t

class NeedsInit {
public:
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: data member 'B' is not explicitly initialized [hsc-member-initialization]
  NeedsInit(int V) : A(V) {}

  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: data member 'B' is not explicitly initialized [hsc-member-initialization]
  NeedsInit() = default;

private:
  int A = 0;
  int B;
};

class ClassMemberIsOK {
public:
  ClassMemberIsOK(int V) : B(V) {}

private:
  struct Inner {
    Inner() = default;
  } A;
  int B;
};

class CopyCtorNotChecked {
public:
  CopyCtorNotChecked(int V) : A(V) {}
  CopyCtorNotChecked(const CopyCtorNotChecked &) = delete;

private:
  int A;
};

class UserProvidedCopyCtorStillChecked {
public:
  UserProvidedCopyCtorStillChecked(int V) : A(V) {}

  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: data member 'A' is not explicitly initialized [hsc-member-initialization]
  UserProvidedCopyCtorStillChecked(const UserProvidedCopyCtorStillChecked &) {}

private:
  int A;
};

// Implicit default constructor for aggregate with uninitialized scalar
// CHECK-MESSAGES: :[[@LINE+1]]:8: warning: data member 'value' is not explicitly initialized [hsc-member-initialization]
struct UnsafeAggregate {
  int value;
  double priority = 1.0;
};

void testUnsafeAggregate() {
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: variable 'unsafe' uses aggregate type with uninitialized members [hsc-member-initialization]
  UnsafeAggregate unsafe;

  // CHECK-MESSAGES-NOT: :[[@LINE+1]]:3: warning: variable 'safe' uses aggregate type with uninitialized members [hsc-member-initialization]
  UnsafeAggregate safe{};  // OK — zero-initialized
}
