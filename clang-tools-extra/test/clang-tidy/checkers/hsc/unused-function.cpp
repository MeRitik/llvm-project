// RUN: %check_clang_tidy %s hsc-unused-function %t

// Non-compliant: unused private member function
struct Container {
    Container() { helper(); }
private:
    void helper() { }  // Compliant — called in constructor
    void unused() { }
    // CHECK-MESSAGES: :[[@LINE-1]]:10: warning: function 'unused' is never used [hsc-unused-function]
};

// Non-compliant: unused static function
static void standalone() {}
// CHECK-MESSAGES: :[[@LINE-1]]:14: warning: function 'standalone' is never used [hsc-unused-function]

// Non-compliant: unused function in anonymous namespace
namespace {
    void internal() {}
    // CHECK-MESSAGES: :[[@LINE-1]]:10: warning: function 'internal' is never used [hsc-unused-function]
}

// Compliant: address taken
static void utility() {}
void (*getUtility())() {
    return &utility;
}

// Non-compliant: dead helper function
namespace {
    double blend(double a, double b, double t) {
        // CHECK-MESSAGES: :[[@LINE-1]]:12: warning: function 'blend' is never used [hsc-unused-function]
        return a + (b - a) * t;
    }
}
double transform(double x) {
    return x * 2.0;
}

// Compliant: private virtual function (excluded)
class Notifier {
private:
    virtual void onNotify(int id) = 0;
};

// Compliant: special member functions excluded
struct Handle {
private:
    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;
};

// Compliant: static function that is called
static void used_static_func() {}
void call_used_static() {
    used_static_func();
}
