void helper();

template <typename T> struct Core {
  void helper();
};

template <typename T> struct Module : Core<T> {
  void run() {
    helper(); // should warn: unqualified lookup in dependent-base context
  }
};

template <typename T> struct ModuleOk : Core<T> {
  void run() {
    this->helper(); // should not warn
    ::helper();     // should not warn
  }
};
