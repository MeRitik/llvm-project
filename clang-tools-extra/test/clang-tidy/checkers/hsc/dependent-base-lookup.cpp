// RUN: %check_clang_tidy %s hsc-dependent-base-lookup %t

void helper();
typedef int GlobalType;

template <typename T> struct Core {
  void helper();
  typedef T GlobalType;
};

template <typename T>
struct Module : Core<T> {
  void run() {
    helper();
    // CHECK-MESSAGES: :[[@LINE-1]]:5: warning: unqualified lookup in a class with dependent base resolved to 'helper'; use qualification or this-> [hsc-dependent-base-lookup]

    GlobalType v = 0;
    // CHECK-MESSAGES: :[[@LINE-1]]:5: warning: unqualified type lookup in a class with dependent base resolved to 'GlobalType'; use qualification [hsc-dependent-base-lookup]
    (void)v;
  }
};

template <typename T>
struct ModuleOk : Core<T> {
  void run() {
    this->helper();
    ::helper();

    typename Core<T>::GlobalType base_v = 0;
    ::GlobalType global_v = 0;
    (void)base_v;
    (void)global_v;
  }
};
