// RUN: %check_clang_tidy %s hsc-moved-from-state %t

void test() {
  std::vector<int> v1, v2;
  v2 = std::move(v1);
  // Note: Full data flow analysis would be needed for this check
}
