// RUN: %check_clang_tidy %s hsc-dependent-base-lookup %t

void helper();

template <typename T> struct Core {
  void helper();
};

template <typename T>
struct Module : Core<T> {
  void run() {
    helper();
    // CHECK-MESSAGES: :[[@LINE-1]]:5: warning: unqualified lookup in a class with dependent base resolved to 'helper'; use qualification or this-> [hsc-dependent-base-lookup]
  }
};

template <typename T>
struct ModuleOk : Core<T> {
  void run() {
    this->helper();
    ::helper();
  }
};
