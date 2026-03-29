// RUN: %check_clang_tidy %s hsc-escaping-local-address %t

int *GlobalPtr;

void badGlobal() {
  int Local = 0;
  GlobalPtr = &Local;
  // CHECK-MESSAGES: :[[@LINE-1]]:13: warning: assigning address of automatic-storage object 'Local' to pointer 'GlobalPtr' with greater lifetime may create a dangling pointer [hsc-escaping-local-address]
}

void badOuter() {
  int *Outer = nullptr;
  {
    int Inner = 1;
    Outer = &Inner;
    // CHECK-MESSAGES: :[[@LINE-1]]:11: warning: assigning address of automatic-storage object 'Inner' to pointer 'Outer' with greater lifetime may create a dangling pointer [hsc-escaping-local-address]

    int Arr[4] = {0, 1, 2, 3};
    Outer = Arr;
    // CHECK-MESSAGES: :[[@LINE-1]]:11: warning: assigning address of automatic-storage object 'Arr' to pointer 'Outer' with greater lifetime may create a dangling pointer [hsc-escaping-local-address]
  }
}

int twice(int X) { return X * 2; }

void badLoopEscape() {
  int *Recent = nullptr;
  for (int I = 0; I < 3; ++I) {
    int Current = twice(I);
    Recent = &Current;
    // CHECK-MESSAGES: :[[@LINE-1]]:12: warning: assigning address of automatic-storage object 'Current' to pointer 'Recent' with greater lifetime may create a dangling pointer [hsc-escaping-local-address]
  }
  (void)Recent;
}

void goodSameScope() {
  int Value = 0;
  int *Ptr = &Value;
  (void)Ptr;
}
