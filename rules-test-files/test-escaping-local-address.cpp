int *GlobalPtr = nullptr;

void bad_global() {
  int local = 0;
  GlobalPtr = &local; // should warn
}

void bad_outer_scope() {
  int *outer = nullptr;
  {
    int inner = 1;
    outer = &inner; // should warn
  }
}

void good_same_scope() {
  int value = 0;
  int *ptr = &value; // should not warn
  (void)ptr;
}
