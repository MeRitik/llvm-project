// RUN: %check_clang_tidy %s hsc-unused-type %t

// Non-compliant: unused type alias in block scope
int compute() {
    using Alias = int;
    // CHECK-MESSAGES: :[[@LINE-1]]:11: warning: type alias 'Alias' is never used [hsc-unused-type]
    return 67;
}

// Compliant: type alias marked with [[maybe_unused]]
int compute2() {
    using Alias [[maybe_unused]] = int;
    return 67;
}

// Non-compliant: unused struct in anonymous namespace
namespace {
    struct Helper { 
        Helper create(); 
    };
    // CHECK-MESSAGES: :[[@LINE-3]]:12: warning: type 'Helper' is never used [hsc-unused-type]
}

// Non-compliant: unused struct in block scope
void execute() {
    struct Settings {
        int delay;
        int attempts;
    };
    // CHECK-MESSAGES: :[[@LINE-4]]:12: warning: type 'Settings' is never used [hsc-unused-type]
    performTask(30, 3);
}

// Compliant: type used for variable
void execute2() {
    struct Settings { int delay; int attempts; };
    Settings opts{30, 3};
}

// Compliant: lambda type is always used
namespace {
    void run() {
        [](auto) {};
    }
}

// Compliant: type used in function parameter
void example() {
    struct Data { int x; };
    Data d{42};
}
