// Rule: HSCAA.2.2 - Unused Named Parameters
// Description: Function parameters that are not used in the function body
// should be removed or marked as intentionally unused using [[maybe_unused]]
// attribute.

// BAD - unused named parameter
void processData(int id, int count, int unused) {
  // 'count' is used, but 'unused' is not
  for (int i = 0; i < count; ++i) {
    printf("ID: %d\n", id);
  }
}

// GOOD - remove the parameter
void processGood(int id, int count) {
  for (int i = 0; i < count; ++i) {
    printf("ID: %d\n", id);
  }
}

// GOOD - mark as intentionally unused
void processAlternative(int id, int count, [[maybe_unused]] int unused) {
  for (int i = 0; i < count; ++i) {
    printf("ID: %d\n", id);
  }
}
