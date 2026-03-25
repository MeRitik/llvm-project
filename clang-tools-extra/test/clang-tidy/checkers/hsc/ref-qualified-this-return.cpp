// RUN: %check_clang_tidy %s hsc-ref-qualified-this-return %t

struct Record {
  int Field;
  int &Ref;

  Record &self() { return *this; }
  // CHECK-MESSAGES: :[[@LINE-1]]:11: warning: member function returning this/subobject should be ref-qualified to avoid dangling references on temporaries [hsc-ref-qualified-this-return]

  int &field() { return Field; }
  // CHECK-MESSAGES: :[[@LINE-1]]:8: warning: member function returning this/subobject should be ref-qualified to avoid dangling references on temporaries [hsc-ref-qualified-this-return]

  int &refMember() { return Ref; }

  Record &ok() & { return *this; }
};

struct PtrHolder {
  int Field;
  int *ptr() { return &Field; }
  // CHECK-MESSAGES: :[[@LINE-1]]:8: warning: member function returning this/subobject should be ref-qualified to avoid dangling references on temporaries [hsc-ref-qualified-this-return]
};
