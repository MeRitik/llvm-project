struct Record {
  int field;
  int &ref;

  Record &self() { return *this; }  // should warn
  int &getField() { return field; } // should warn
  int &getRef() { return ref; }     // should not warn (reference member)

  Record &ok() & { return *this; } // should not warn
};

struct PtrHolder {
  int field;
  int *ptr() { return &field; } // should warn
};
