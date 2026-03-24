## Rule HSCAA.1.2 --- The value returned by a function shall be used
Description: Invoking a non-void function without utilizing its return value may signal a programming oversight. When the return value is deliberately ignored, casting to void clarifies the intent. Overloaded operators invoked with operator syntax are exempt, as they are expected to mirror built-in operator behavior.

### Non-Compliant --- return value silently discarded:
uint16_t compute();

```cpp
void sample() {
    compute();               // Non-compliant — return value ignored
}
```

### Compliant --- value used or explicitly discarded:
```cpp
void sample() {
    auto result = compute();      // Compliant — used as initialiser
    (void)compute();              // Compliant — explicitly cast to void
}
```

### Non-compliant --- member function return value ignored:
```cpp
class DeviceController {
public:
    bool startup();
    int fetchData();
};

void checkDevice(DeviceController& ctrl) {
    ctrl.startup();       // Non-compliant — error status discarded
    ctrl.fetchData();     // Non-compliant — data discarded
}

```

### Compliant --- return values handled:
```cpp
void checkDevice(DeviceController& ctrl) {
    if (!ctrl.startup()) { reportError(); return; }
    int data = ctrl.fetchData();
    handleData(data);
}
```

### Compliant --- operator syntax excluded:

```cpp
std::string first = "alpha";
std::string second = "beta";
first + second;     // Operator syntax — rule does not apply
first = second;     // Operator syntax — rule does not apply

### Non-Compliant --- std::function and lambda calls:
void execute(std::function<int()>& fn) {
    fn();                  // Non-compliant — return value discarded
    auto operation = []() { return 42; };
    operation();           // Non-compliant — return value discarded
}
```
---

## Rule HSCAA.2.1 --- Variables with limited visibility should be used at least once
Description: A declared but unused variable adds no value to program output. It may suggest an incomplete computation, a naming error, or residual code from refactoring. Variables with user-provided constructors or destructors (e.g., RAII guards) are deemed "used" via their construction/destruction side effects.

### Non-Compliant --- unused variable:
```cpp
namespace {
    class Item {};          // No user-provided ctor/dtor
    Item obj;               // Non-compliant — obj is never used
}

void process() {
    int16_t counter = 42;   // Non-compliant — never read
}
```

### Compliant --- RAII guard:
```cpp
void protectedSection() {
    std::lock_guard<std::mutex> guard{mtx};  // Compliant — user-provided ctor/dtor
    // ... protected code ...
}   // guard released here
```
### Compliant --- [[maybe_unused]]:
```cpp
void check(int32_t val) {
    [[maybe_unused]] bool flag = val > 0;   // Compliant by exception
    assert(flag);                            // May expand to nothing if NDEBUG
}
```

### Non-Compliant --- accumulator never read:
```cpp
void aggregate(const std::vector<double>& items) {
    double total = 0.0;          // Non-compliant if total is never read
    for (auto item : items) {
        total += item;
    }
    // Developer forgot: return total / items.size();
}
```

### Compliant --- scoped file stream (RAII):
```cpp
void writeLog(const std::string& text) {
    std::ofstream stream{"output.log", std::ios::app};   // Compliant — user-provided ctor
    stream << text << "\n";
}
```
----

## Rule HSCAA.2.2 --- A named function parameter shall be used at least once
Description: An unused named parameter suggests the function's implementation may not fulfill its interface contract. When a parameter must exist for signature compatibility (e.g., overrides, callbacks) but is not required, it should remain unnamed or be marked `[[maybe_unused]]`.

### Non-Compliant --- unused parameter in override:
```cpp
class Parent {
public:
    virtual int calculate(int x, int y);
};

class Child : public Parent {
public:
    int calculate(int x, int y) override {   // Non-compliant — 'y' is unused
        return x;
    }
};
```

### Compliant --- unnamed parameter:
```cpp
class Child2 : public Parent {
public:
    int calculate(int x, int) override {     // Compliant — unnamed parameter
        return x;
    }
};
```

### Non-Compliant --- unused parameter in free function:
```cpp
void operation(int first, int second) {   // Non-compliant — 'first' is unused
    (void)second;
}
```

### Non-Compliant --- callback with unused context:
```cpp
using Handler = void(*)(int code, void* ctx);

void onEvent(int code, void* ctx) {   // Non-compliant — 'ctx' unused
    record(code);
}
```

### Compliant --- [[maybe_unused]] for conditionally-used parameter:

```cpp
template<bool verbose>
void output(int num, [[maybe_unused]] const char* tag) {
    if constexpr (verbose) {
        std::cout << tag << ": " << num << "\n";
    }
}
```

### Compliant --- lambda with unnamed parameter:

```cpp
auto skipFirst = [](int, double val) {   // Compliant — first param unnamed
    return val * 2.0;
};
```


## Rule HSCAA.2.3 --- Types with limited visibility should be used at least once
Description: A type declared in block scope or in an unnamed namespace that is never referenced outside its own definition is likely residual from refactoring or an incomplete implementation.

### Non-Compliant --- unused type alias:
```cpp
int compute() {
    using Alias = int;     // Non-compliant — Alias is never used
    return 67;
}
```

### Compliant --- `[[maybe_unused]]`:

```cpp
int compute() {
    using Alias [[maybe_unused]] = int;   // Compliant by exception
    return 67;
}
```

### Non-Compliant --- unused struct in anonymous namespace:

```cpp
namespace {
    struct Helper { Helper create(); };      // Non-compliant — Helper not used outside its definition
    Helper Helper::create() { return *this; }  // Not a use of Helper per rule
}
```

### Non-Compliant --- unused helper struct in block scope:

```cpp
void execute() {
    struct Settings {              // Non-compliant — Settings is never instantiated
        int delay;
        int attempts;
    };
    // Developer forgot to create a Settings object and use it
    performTask(30, 3);              // Hardcoded values instead
}
```

### Compliant --- type used for local variable:

```cpp
void execute() {
    struct Settings { int delay; int attempts; };
    Settings opts{30, 3};          // Compliant — Settings is used
    performTask(opts.delay, opts.attempts);
}
```

### Compliant --- closure type is always considered used:

```cpp
namespace {
    void run() {
        [](auto) {};            // Compliant — closure type always used
    }
}
```

Rule HSCAA.2.4 --- Functions with limited visibility should be used at least once
Description: Functions with limited visibility (static, private non-virtual, or in anonymous namespaces) are not part of an extensible API. If present but unused, they may indicate dead code or a design flaw.

### Non-Compliant --- unused private and static functions:
struct Container {
    Container() { helper(); }
private:
    void helper() {}          // Compliant — called in constructor
    void unused() {}          // Non-compliant — never used
};

static void standalone() {}       // Non-compliant — never used

namespace {
    void internal() {}          // Non-compliant — never used
}
### Compliant --- address taken:
static void utility() {}       // Compliant — address taken below

void (*getUtility())() {
    return &utility;
}

### Non-Compliant --- dead helper function after refactoring:
namespace {
    // Was used before refactoring, now dead code
    double blend(double a, double b, double t) {   // Non-compliant
        return a + (b - a) * t;
    }
}

double transform(double x) {
    return x * 2.0;       // blend() is no longer called
}
### Compliant --- private virtual function (excluded from rule):
class Notifier {
    virtual void onNotify(int id) = 0;   // Rule does not apply — virtual
};
### Compliant --- special member function (excluded from rule):
struct Handle {
private:
    Handle(const Handle&) = delete;          // Excluded — special member
    Handle& operator=(const Handle&) = delete;  // Excluded — special member
};


Rule HSCAG.4.2 --- Derived classes shall not conceal functions inherited from their bases
Description: When a derived class declares a function with the same name as a base class function, C++ name lookup stops at the derived class and does not consider the base class overloads. This "concealment" may cause calls to resolve to unexpected functions. The rule does not apply when the base is inherited privately, when the function is a virtual override, or when a using-declaration introduces the base function.

### Non-Compliant --- derived function conceals base:
class Parent {
public:
    void action(int i);
    void action(char c);
};

class Child : public Parent {
public:
    void action(float f);     // Non-compliant — conceals Parent::action(int)
    void action(char c);      // Non-compliant — conceals Parent::action(char), not an override
};
### Compliant --- using-declaration:
class Child : public Parent {
public:
    using Parent::action;       // Introduces Parent::action overload
    void action(float f);       // Compliant — Parent::action is not concealed
};
### Compliant --- virtual override:
class Child : public Parent {
public:
    Child* action(char c) override;   // Compliant — overrides Parent::action
};
### Compliant --- private inheritance:
class PrivateChild : private Parent {
public:
    void action(float f);   // Compliant — Parent inherited privately
};

### Non-Compliant --- concealment in a real-world hierarchy:
class Renderer {
public:
    void render(int x, int y);
    void render(int x, int y, int z);
};

class Sprite : public Renderer {
public:
    void render(double scale);   // Non-compliant — conceals both Renderer::render overloads
    // A call like sprite.render(10, 20) will fail to compile or resolve unexpectedly
};
### Compliant --- using-declaration preserves base overloads:
class Sprite : public Renderer {
public:
    using Renderer::render;          // Introduces Renderer::render overloads
    void render(double scale);       // Compliant
};


Rule HSCAG.4.3 --- A name present in a dependent base shall not be resolved by unqualified lookup
Description: In a template class with a dependent base, C++ does not search the base class during unqualified name lookup. An unqualified name resolves to a global or enclosing-namespace entity, even if an identically named entity exists in the base. This can silently invoke the wrong function or use the wrong type.

### Non-Compliant --- unqualified call resolves to global:
typedef int32_t DataType;
void helper();

template<typename T> struct Core;

template<typename T>
struct Module : Core<T> {
    void run() {
        DataType val = 0;      // Non-compliant — resolves to ::DataType, not Core<T>::DataType
        helper();              // Non-compliant — resolves to ::helper(), not Core<T>::helper()
    }
};

template<typename T>
struct Core {
    typedef T DataType;
    void helper();
};
### Compliant --- qualified name or this->:
template<typename T>
struct Module : Core<T> {
    void run() {
        ::DataType val1 = 0;               // Compliant — explicitly global
        typename Core<T>::DataType val2 = 0;  // Compliant — explicitly base
        this->helper();                    // Compliant — defers to base
        ::helper();                        // Compliant — explicitly global
    }
};
### Compliant --- using-declaration:
template<typename T>
struct Module : Core<T> {
    using typename Core<T>::DataType;   // Introduces base DataType
    void run() {
        DataType val = 0;              // Compliant — found via using-declaration
    }
};

### Non-Compliant --- real-world container adapter:
using length_type = unsigned long;

template<typename Storage>
class Wrapper : public Storage {
public:
    length_type total() {             // Non-compliant — length_type resolves to
        return length_type{0};        //   ::length_type, not Storage::length_type
    }
};
### Compliant --- qualified access in container adapter:
template<typename Storage>
class Wrapper : public Storage {
public:
    typename Storage::length_type total() {   // Compliant
        return typename Storage::length_type{0};
    }
};


Rule HSCAG.8.3 --- An assignment operator shall not assign the address of an automatic-storage object to an object with greater lifetime
Description: If the address of a local variable is stored in a pointer that outlives the local (e.g., declared in an outer scope or with static storage duration), the pointer becomes dangling when the local is destroyed. This leads to undefined behaviour.

### Non-Compliant --- address of local escapes scope:
void example1() {
    int8_t* ptr;
    {
        int8_t temp;
        int8_t arr[10];

        ptr = &temp;          // Non-compliant — ptr outlives temp
        ptr = arr;            // Non-compliant — ptr outlives arr
    }
    // ptr is now dangling
}
### Compliant --- same scope:
void example2() {
    int8_t* outer;
    {
        int8_t* inner = nullptr;
        int8_t temp;
        inner = &temp;         // Compliant — inner and temp have the same lifetime
    }
}

### Non-Compliant --- local address assigned to static pointer:
int* sharedPtr = nullptr;

void storeValue() {
    int num = 42;
    sharedPtr = &num;      // Non-compliant — sharedPtr has static storage
}
// sharedPtr is dangling after storeValue() returns

### Non-Compliant --- loop variable address escaping:
void iterate() {
    int* recent = nullptr;
    for (int i = 0; i < 10; ++i) {
        int current = calculate(i);
        recent = &current;        // Non-compliant — recent outlives current each iteration
    }
}
### Compliant --- pointer and pointee in same scope:
void example() {
    int num = 42;
    int* ptr = &num;         // Compliant — same scope, same lifetime
    consume(ptr);
}


Rule HSCAG.8.4 --- Member functions returning references to their object should be ref-qualified appropriately
Description: A member function that returns a reference or pointer to *this or one of its subobjects can produce a dangling reference if called on a temporary. Ref-qualifying such functions with & prevents them from being called on rvalues, eliminating immediate dangling.

### Non-Compliant --- not ref-qualified:
struct Record {
    int32_t field;
    int32_t& ref;

    Record& self() { return *this; }              // Non-compliant — not ref-qualified
};

Record makeRecord();
Record& invalid = makeRecord().self();              // Immediate dangling reference!
### Compliant --- lvalue-ref-qualified:
struct Record {
    int32_t field;

    int32_t& getField() & { return field; }           // Compliant — & ref-qualified

    int32_t const& getField() const& { return field; }  // Compliant — const& with
    int32_t getField() && { return field; }              //   rvalue overload
};
Rule does not apply --- returning a reference member:
struct Record {
    int32_t& ref;
    int32_t& getRef() { return ref; }   // Rule does not apply — ref is a reference, not a subobject
};

### Non-Compliant --- builder pattern without ref-qualification:
class RequestBuilder {
    std::string endpoint_;
    std::string params_;
public:
    RequestBuilder& setEndpoint(const std::string& ep) {     // Non-compliant
        endpoint_ = ep; return *this;
    }
    RequestBuilder& setParams(const std::string& p) {    // Non-compliant
        params_ = p; return *this;
    }
};

// Dangerous: chaining on a temporary
auto& req = RequestBuilder().setEndpoint("/api").setParams("id=1");  // Dangling!
### Compliant --- ref-qualified builder:
class RequestBuilder {
    std::string endpoint_;
    std::string params_;
public:
    RequestBuilder& setEndpoint(const std::string& ep) & {   // Compliant — & ref-qualified
        endpoint_ = ep; return *this;
    }
    RequestBuilder& setParams(const std::string& p) & {  // Compliant — & ref-qualified
        params_ = p; return *this;
    }
};
### Compliant --- returning pointer to this with const& + && deleted:
class Component {
public:
    const Component* instance() const& { return this; }   // Compliant
    void instance() const&& = delete;                      // Prevents call on temporaries
};


Rule HSCAP.1.2 --- All constructors of a class should explicitly initialise all virtual and immediate base classes
Description: When a constructor does not explicitly initialise a base class, the compiler silently invokes the base's default constructor. In hierarchies with virtual inheritance, this can lead to confusion about which constructor runs and with what arguments. Explicit initialisation makes the developer's intent unambiguous.

### Non-Compliant --- missing explicit base init:
class Root {
public:
    Root() {}
    Root(int32_t val) : val_(val) {}
private:
    int32_t val_ = 0;
};

class BranchA : public virtual Root {
public:
    BranchA() : Root{21} {}           // Compliant
};

class BranchB : public virtual Root {
public:
    BranchB() : Root{42} {}           // Compliant
};

class Leaf : public BranchA, public BranchB {
public:
    Leaf() {}                     // Non-compliant — Root, BranchA, BranchB not explicitly initialised
    // Root's default ctor runs (val_ == 0), despite BranchA and BranchB specifying 21 and 42
};
### Compliant --- explicit initialisation:
class Leaf : public BranchA, public BranchB {
public:
    Leaf() : Root{}, BranchA{}, BranchB{} {}  // Compliant — all bases explicit, Root::val_ == 0
};
### Compliant --- delegating constructor:
class Extended : public Core {
public:
    Extended(int32_t num) : Core{num} {}   // Compliant — Core explicitly initialised
    Extended() : Extended(0) {}             // Compliant — delegates to other constructor
};
Exception --- empty base class:
class Placeholder {};

class Concrete : public Placeholder {
public:
    Concrete() {}    // Compliant by exception — Placeholder has nothing to initialise
};

### Non-Compliant --- real-world device hierarchy:
class BusInterface {
public:
    BusInterface(int channel);
private:
    int channel_;
};

class Protocol1 : public virtual BusInterface {
public:
    Protocol1() : BusInterface(0) {}
};

class Protocol2 : public virtual BusInterface {
public:
    Protocol2() : BusInterface(1) {}
};

class MultiProtocol : public Protocol1, public Protocol2 {
public:
    MultiProtocol() {}   // Non-compliant — BusInterface, Protocol1, Protocol2
                          // not explicitly initialised.
                          // BusInterface's default ctor called — but
                          // it doesn't exist! Compile error or wrong channel.
};
### Compliant --- all bases explicit:
class MultiProtocol : public Protocol1, public Protocol2 {
public:
    MultiProtocol()
        : BusInterface(0), Protocol1(), Protocol2() {}   // Compliant
};


Rule HSCAN.1.2 --- An accessible base class shall not be both virtual and non-virtual in the same hierarchy
Description: When a base class is inherited both virtually and non-virtually in the same hierarchy, it is unclear whether the intent is for there to be one or more instances of the base class subobject. This leads to confusing object layouts and potential logic errors.

### Non-Compliant --- mixed virtual and non-virtual inheritance:
class Root {};

class Branch1 : public virtual Root {};
class Branch2 : public virtual Root {};
class Branch3 : public         Root {};

class Leaf : public Branch1, Branch2, Branch3 {};   // Non-compliant — Leaf has two Root subobjects
### Compliant --- consistent virtual inheritance:
class Root {};

class Branch1 : public virtual Root {};
class Branch2 : public virtual Root {};

class Leaf : public Branch1, public Branch2 {};   // Compliant — one Root subobject

### Non-Compliant --- accidental mixed inheritance in a plugin system:
class Source {};

class DisplayPlugin : public virtual Source {};
class SoundPlugin : public Source {};          // Non-virtual!

class Player : public DisplayPlugin, public SoundPlugin {};
// Non-compliant — Player has two Source subobjects
// Is that intentional? Almost certainly not.
### Compliant --- all paths use virtual:
class DisplayPlugin : public virtual Source {};
class SoundPlugin : public virtual Source {};

class Player : public DisplayPlugin, public SoundPlugin {};
// Compliant — one Source subobject


Rule HSCAN.3.1 --- User-declared member functions shall use the virtual, override and final specifiers appropriately
Description: Using a single, correct specifier for each member function makes the intent unambiguous: - virtual --- a new virtual function expected to be overridden. - override --- an override that may itself be overridden. - final --- an override that cannot be overridden further.
Redundant combinations (virtual + override, override + final, virtual + final) obscure the meaning and shall be avoided.

### Non-Compliant --- redundant and missing specifiers:
class Parent {
public:
    virtual ~Parent() = default;
    virtual void method1() noexcept = 0;            // Compliant
    virtual void method2() noexcept {}              // Compliant
    void method4() noexcept {}                      // Compliant — not virtual
    virtual void method5() noexcept final = 0;      // Non-compliant — virtual + final
};

class Child : public Parent {
public:
    ~Child();                                       // Non-compliant — no specifier for override
    virtual void method1() noexcept override {}      // Non-compliant — virtual + override
    void method2() noexcept override final {}        // Non-compliant — override + final
    void method3() noexcept {}                       // Non-compliant — no specifier for override
};
### Compliant --- single appropriate specifier:
class Child : public Parent {
public:
    ~Child() override;                             // Compliant
    void method1() noexcept override {}             // Compliant
    void method2() noexcept final {}                // Compliant — final implies override
};

### Non-Compliant --- forgotten override in real-world handler:
class MessageHandler {
public:
    virtual void onMessage(const Packet& pkt) = 0;
};

class DataHandler : public MessageHandler {
public:
    void onMessage(const Packet& pkt) {}     // Non-compliant — missing override
    // If the base signature changes, this silently becomes a new function
};
### Compliant:
class DataHandler : public MessageHandler {
public:
    void onMessage(const Packet& pkt) override {}   // Compliant
};


Rule HSCAN.3.2 --- Parameters in an overriding virtual function shall not specify different default arguments
Description: Default arguments are resolved by the static type of the object. If an overriding function specifies a different default, the value used depends on whether the call is made through a base or derived reference, which is inconsistent and error-prone.

### Non-Compliant --- different default values:
class Parent {
public:
    virtual void correct(int32_t val = 0);
    virtual void wrong1(int32_t val = 0);
    virtual void wrong2(int32_t val);
};

class Child : public Parent {
public:
    void correct(int32_t val = 0) override;    // Compliant — same default
    void wrong1(int32_t val = 1) override;     // Non-compliant — different value
    void wrong2(int32_t val = 2) override;     // Non-compliant — no default in base
};

void test(Child& c) {
    Parent& p = c;
    p.wrong1();    // Uses default of 0 (Parent's default)
    c.wrong1();    // Uses default of 1 (Child's default) — inconsistent!
}
### Compliant --- no default in override:
class Child : public Parent {
public:
    void correct(int32_t val = 0) override;   // Compliant — same value
    void wrong1(int32_t val) override;        // Compliant — no default specified
};

### Non-Compliant --- real-world timeout default mismatch:
class Link {
public:
    virtual bool transmit(const Payload& p, int timeout_ms = 1000);
};

class SecureLink : public Link {
public:
    bool transmit(const Payload& p, int timeout_ms = 5000) override;
    // Non-compliant — caller gets 1000ms via base ref, 5000ms via derived
};


Rule HSCAN.3.4 --- A comparison of a potentially virtual pointer to member function shall only be with nullptr
Description: The result of comparing a pointer to a virtual member function with anything other than nullptr is unspecified by the C++ Standard. Only comparisons with nullptr yield well-defined results.

### Non-Compliant --- comparing virtual member function pointers:
class Entity {
public:
    void action1();
    void action2();
    virtual void action3();
};

void test() {
    if (&Entity::action1 != &Entity::action2) {}      // Compliant — both non-virtual
    if (&Entity::action1 != nullptr) {}               // Compliant — comparison with nullptr
    if (&Entity::action3 == &Entity::action2) {}      // Non-compliant — action3 is virtual
    if (&Entity::action3 == nullptr) {}               // Compliant — nullptr comparison
}

### Non-Compliant --- runtime pointer to member with virtual functions present:
void check(void (Entity::*ptr)()) {
    if (ptr == &Entity::action2) {}    // Non-compliant — ptr could point to virtual Entity::action3
}
### Compliant --- no virtual functions with matching signature:
class Item {
public:
    void op1();
    void op2();
    virtual void op3(int32_t i);    // Different signature
};

void check(void (Item::*ptr)()) {
    if (ptr == &Item::op2) {}   // Compliant — no virtual functions with void() signature
}

### Non-Compliant --- incomplete class type:
struct Incomplete;

void test(void (Incomplete::*p1)(), void (Incomplete::*p2)()) {
    if (p1 == p2) {}        // Non-compliant — Incomplete is incomplete, unknown if virtual
}


Rule HSCAP.1.1 --- An object's dynamic type shall not be used from within its constructor or destructor
Description: During construction and destruction, the dynamic type of an object differs from the type of the fully constructed object. Virtual calls, typeid on polymorphic types, and dynamic_cast produce results that may not match developer expectations. Calling a pure virtual function from a constructor or destructor results in undefined behaviour.

### Non-Compliant --- virtual call in constructor:
class Polymorphic {
public:
    virtual ~Polymorphic();
    virtual void init();
    virtual void setup() = 0;

    Polymorphic() {
        typeid(*this);                       // Non-compliant — polymorphic typeid
        init();                              // Non-compliant — virtual call
        setup();                             // Non-compliant — undefined behaviour
        dynamic_cast<Polymorphic*>(this);    // Non-compliant — dynamic_cast
        Polymorphic::init();                 // Compliant — not a virtual call
        typeid(Polymorphic);                 // Compliant — type, not current object
    }
};
### Compliant --- non-polymorphic typeid:
class NonPolymorphic {
public:
    NonPolymorphic() {
        typeid(*this);     // Compliant — NonPolymorphic is not polymorphic (no virtual functions)
    }
};

### Non-Compliant --- indirect virtual call through pointer:
class Level1 {
public:
    explicit Level1(Level2* l);
    virtual ~Level1();
    virtual void init();
};

class Level2 : public Level1 {
public:
    Level2() : Level1(this) {}
};

Level1::Level1(Level2* l) {
    init();          // Non-compliant — calls Level1::init (not Level2::init as might be expected)
    l->init();       // Non-compliant — undefined behaviour, Level2 not yet constructed
}

### Non-Compliant --- factory-style init in constructor:
class Component {
public:
    virtual void configure() = 0;
    Component() {
        configure();   // Non-compliant — pure virtual call, undefined behaviour
    }
};
### Compliant --- two-phase initialisation:
class Component {
public:
    virtual void configure() = 0;
    Component() {}                              // Compliant — no virtual call
    static std::unique_ptr<Component> create(); // configure() called after construction
};


Rule HSCAP.1.3 --- Conversion operators and constructors that are callable with a single argument shall be explicit
Description: Without explicit, single-argument constructors and conversion operators allow implicit type conversions, which can lead to unexpected function calls and hard-to-diagnose bugs. Copy and move constructors are excluded from this rule.

### Non-Compliant --- implicit single-argument constructor:
class Wrapper {
public:
    Wrapper(int32_t val);                          // Non-compliant — allows implicit conversion
};

class Container {
public:
    explicit Container(int32_t val);                 // Compliant
    Container(const Container& c);                   // Rule does not apply — copy constructor
    operator int32_t() const;                        // Non-compliant — implicit conversion
    explicit operator bool() const;                  // Compliant
};

### Non-Compliant --- constructor with defaulted second argument:
class Config {
public:
    Config(int32_t a, int32_t b = 0);           // Non-compliant — callable with one argument
    Config(char a = 'a', int32_t b = 0);        // Non-compliant — callable with one argument
    Config(char a, char b);                     // Rule does not apply — requires two arguments
};

void apply(Config c);

void test() {
    apply(0);     // Implicit conversion from 0 to Config — surprising!
}

### Non-Compliant --- real-world ID wrapper:
class Identifier {
public:
    Identifier(int id);     // Non-compliant — int silently converts to Identifier
};

void removeEntry(Identifier id);

void test() {
    removeEntry(42);     // Compiles silently — is this intentional?
}
### Compliant --- explicit prevents accidental conversion:
class Identifier {
public:
    explicit Identifier(int id);   // Compliant
};

void test() {
    removeEntry(Identifier{42});    // Must be explicit — intent is clear
    // removeEntry(42);             // Error — no implicit conversion
}


Rule HSCAP.1.4 --- All direct, non-static data members of a class should be initialised before the class object is accessible
Description: A constructor should completely initialise its object. Uninitialised members can contain indeterminate values, leading to undefined behaviour when read. Members should be initialised via default member initialisers or constructor member initialisation lists, not by assignment in the constructor body.

### Non-Compliant --- member initialised in body instead of initialiser list:
class Record {
public:
    explicit Record(const std::string& label)
        : label_{label}              // Non-compliant — count_ not in initialiser list
    {
        count_ = 18;                  // Assignment in body is too late
    }

    Record() = default;       // Non-compliant — count_ has no default initialiser

private:
    std::string label_;
    int32_t count_;
    int32_t priority_ = 1000;        // Has default member initialiser — OK
};
### Compliant --- all members initialised:
class Record {
public:
    Record(const std::string& label, int32_t count)
        : label_{label}, count_{count} {}     // Compliant — all members initialised

    explicit Record(int32_t count)
        : count_{count} {}                  // Compliant — label_ default-constructed,
                                           //             priority_ has default initialiser
private:
    std::string label_;
    int32_t count_;
    int32_t priority_ = 1000;
};

### Non-Compliant --- aggregate without initialiser:
struct DataPoint {
    std::string tag;
    int32_t value;
    int32_t priority{1000};
};

void test() {
    DataPoint dp1;       // Non-compliant — value is uninitialised
    DataPoint dp2{};     // Compliant — value zero-initialised
}

### Non-Compliant --- reading with uninitialised timestamp:
class Measurement {
    double reading_;
    uint64_t timestamp_;      // No default initialiser!
public:
    explicit Measurement(double r) : reading_{r} {}
    // Non-compliant — timestamp_ is indeterminate
};
### Compliant --- default member initialiser:
class Measurement {
    double reading_ = 0.0;
    uint64_t timestamp_ = 0;
public:
    explicit Measurement(double r) : reading_{r} {}
    // Compliant — timestamp_ has default initialiser
};


Rule HSCAR.8.1 --- Function templates shall not be explicitly specialised
Description: Explicit function specialisations are only considered after overload resolution has chosen a best match from the primary templates. Additionally, non-template overloads are preferred over template versions. This means specialisations may never be called even when the developer expects them to be. Function overloads provide a clearer and more predictable alternative.

### Non-Compliant --- explicit specialisation:
template<typename T> void process(T);               // Overload #1A
template<> void process<char*>(char*);               // Non-compliant — specialisation of #1A
template<typename T> void process(T*);               // Overload #1B

void test(char* str) {
    process(str);    // Calls overload #1B (T=char), NOT the specialisation!
}
### Compliant --- use overloads instead:
template<typename T> void handle(T);               // Overload #2A
template<typename T> void handle(T*);              // Overload #2B
void handle(char*);                                // Overload #2C — rule does not apply

void test(char* str) {
    handle(str);    // Calls #2C — clear, predictable
}

### Non-Compliant --- specialised encoder:
template<typename T>
void encode(const T& obj, std::ostream& os);

template<>
void encode<std::string>(const std::string& s, std::ostream& os);
// Non-compliant — may not be selected if a better-matching primary exists
### Compliant --- overload instead:
template<typename T>
void encode(const T& obj, std::ostream& os);

void encode(const std::string& s, std::ostream& os);  // Overload — always preferred


Rule HSCAS.1.1 --- An exception object shall not have pointer type
Description: If a pointer is thrown as an exception and it refers to a dynamically allocated object, it is unclear which function is responsible for destroying it and when. Throwing by value avoids this ownership ambiguity entirely.

### Non-Compliant --- pointer thrown:
class Error { /* ... */ };

void operation(int16_t code) {
    static Error staticErr;
    Error* dynamicErr = new Error;

    if (code > 10) {
        throw &staticErr;      // Non-compliant — pointer type thrown
    } else {
        throw dynamicErr;       // Non-compliant — pointer type thrown
                        // Who deletes dynamicErr?
    }
}
### Compliant --- throw by value:
void operation(int16_t code) {
    if (code > 10) {
        throw std::runtime_error("code exceeds limit");   // Compliant — value type
    }
}

### Non-Compliant --- throwing dynamically allocated exception:
void execute() {
    try {
        riskyTask();
    } catch (...) {
        throw new std::runtime_error("task failed");   // Non-compliant — pointer type
        // Caller must remember to delete — fragile and error-prone
    }
}
### Compliant --- standard exception by value:
void execute() {
    try {
        riskyTask();
    } catch (...) {
        throw std::runtime_error("task failed");   // Compliant — thrown by value
    }
}

### Non-Compliant --- throwing C-string literal pointer:
void verify(int num) {
    if (num < 0) {
        throw "invalid number";   // Non-compliant — const char* is a pointer type
    }
}
### Compliant --- use exception class:
void verify(int num) {
    if (num < 0) {
        throw std::invalid_argument("invalid number");   // Compliant
    }
}


Rule HSCAI.2.1 --- A virtual base class shall only be cast to a derived class by means of dynamic_cast
Description: The behaviour when casting from a virtual base class to a derived class is only well defined when dynamic_cast is used. Using static_cast or reinterpret_cast can result in undefined behaviour. Since C++17, a static_cast from a virtual base class is ill-formed, but some compilers may not yet issue a diagnostic.

### Non-Compliant --- reinterpret_cast from virtual base:
class Root { };
class Leaf: public virtual Root { };

Leaf   obj;
Root   * pRoot    =    &obj;
Leaf   * pLeaf1   =    reinterpret_cast< Leaf * >( pRoot );    // Non-compliant
### Compliant --- dynamic_cast:
Leaf   * pLeaf2   =    dynamic_cast< Leaf * >( pRoot );        // Compliant — pLeaf2 may be null
Leaf   & refLeaf  =    dynamic_cast< Leaf & >( *pRoot );       // Compliant — may throw an exception


Rule HSCAI.2.2 --- C-style casts and functional notation casts shall not be used
Description: C-style casts and functional notation casts raise several concerns: they permit almost any type to be converted to almost any other type without checks, they give no indication why the conversion is taking place, and their syntax is more difficult to recognize. The use of const_cast, dynamic_cast, static_cast and reinterpret_cast addresses these concerns.
Exception: A C-style cast to void is permitted to explicitly discard a value (see 

Rule HSCAA.1.2).

### Non-Compliant --- C-style cast:
struct Item { Item( char c); };

void example1( Item obj ) {
    auto const & ref = obj;
    Item * ptr = ( Item * )&ref;                           // Non-compliant
}

void example2( int32_t num ) {
    auto narrow = int8_t ( num );                         // Non-compliant — functional notation
}
### Compliant --- explicit casts:
void example1( Item obj ) {
    auto const & ref = obj;
    Item * ptr = const_cast< Item * >( &ref );             // Compliant
}

void example() {
    ( void ) compute();                                  // Compliant by exception
}

void example2( int32_t num ) {
    auto narrow = int8_t { 42 };                        // Compliant — curly braces
    auto item = Item( 'c' );                             // Compliant — constructor call
}


Rule HSCAI.2.3 --- A cast shall not remove any const or volatile qualification from the type accessed via a pointer or by reference
Description: Using a cast to remove the qualification associated with the addressed type is a violation of the principle of type qualification. Removal of const qualification might circumvent the read-only status of an object, which may lead to undefined behaviour. Removal of volatile qualification might result in accesses to an object being removed during optimization.

### Non-Compliant --- casting away const/volatile:
     uint16_t           num;
const uint16_t * pConst; // pointer to const volatile uint16_t * pVolatile; // pointer to volatile uint16_t * pPlain;
pPlain = const_cast< uint16_t * >( pConst );              // Non-compliant
pPlain = const_cast< uint16_t * >( pVolatile );           // Non-compliant
### Compliant --- no cast removes qualification:
pPlain = pConst;                                          // Rule does not apply — no cast


Rule HSCAI.2.5 --- reinterpret_cast shall not be used
Description: Casting between unrelated types generally results in undefined behaviour.
Exception: The following are allowed as the behaviour is well defined: 1. Using reinterpret_cast< T * > to cast any object pointer to a pointer to T, where T is one of void, char, unsigned char or std::byte, possibly cv-qualified. 2. Using reinterpret_cast< T >( p ) to convert a pointer p to an integer of type T that is large enough to represent a pointer value (e.g. std::uintptr_t).

### Non-Compliant --- reinterpret_cast between unrelated types:
uint8_t * narrow;
uint32_t * wide;

wide = reinterpret_cast< uint32_t * >( narrow );         // Non-compliant

void convert() {
    uint32_t   val = fetchValue();
    uint16_t * ptr = reinterpret_cast< uint16_t * >( &val );   // Non-compliant
}
### Compliant by exception --- casting to byte pointer or uintptr_t:
void inspect( float num ) {
    auto bytes = reinterpret_cast< std::byte const * >( &num );  // Compliant by exception
    auto addr = reinterpret_cast< std::uintptr_t >( &num );      // Compliant by exception
}


Rule HSCAI.2.6 --- An object with integral, enumerated, or pointer to void type shall not be cast to a pointer type
Description: Casting from either an integral type or a pointer to void type to a pointer to an object may lead to unspecified behaviour. A round trip conversion of a pointer to object type through void * is well-defined but prohibited by this rule as it is error prone and detection of any error would be undecidable.
Note: Casting from an integer to a pointer may be unavoidable when addressing memory mapped registers or other hardware specific features.

### Non-Compliant --- casting from void or integer to pointer:*
struct Data { int32_t x; int32_t y; };

void convert( void * raw, int32_t addr ) {
    Data * d1 = static_cast< Data * >( raw );             // Non-compliant
    Data * d2 = reinterpret_cast< Data * >( addr );       // Non-compliant
    void * v = reinterpret_cast< void * >( addr );        // Non-compliant
}
### Compliant --- const_cast on void:*
void convert( void * raw ) {
    auto constRaw = const_cast< void const * >( raw );    // Compliant
}


Rule HSCAI.18.1 --- An object or subobject must not be copied to an overlapping object
Description: Copying between members of the same union object may result in undefined behaviour. If part of an array is to be copied to another part of the same array, std::memcpy may overwrite an element before it has been copied. By contrast, std::memmove is guaranteed to handle the overlap appropriately.

### Non-Compliant --- overlapping copy:
void example1( void ) {
    union {
        int16_t small;
        int32_t large;
    } data = { 0 };

    data.large = data.small;                                     // Non-compliant
}

void example2( std::array< int16_t, 20 > & arr ) {
    memcpy ( &arr[ 0 ], &arr[ 1 ], 10u * sizeof( arr[ 0 ] ) );   // Non-compliant
    memcpy ( &arr[ 1 ], &arr[ 0 ], 10u * sizeof( arr[ 0 ] ) );   // Non-compliant
}
### Compliant --- use memmove for overlapping regions:
void example2( std::array< int16_t, 20 > & arr ) {
    memmove( &arr[ 0 ], &arr[ 1 ], 10u * sizeof( arr[ 0 ] ) );   // Compliant
    memmove( &arr[ 1 ], &arr[ 0 ], 10u * sizeof( arr[ 0 ] ) );   // Compliant
    memcpy ( &arr[ 0 ], &arr[ 5 ], 5u * sizeof( arr[ 0 ] ) );    // Compliant — no overlap
}


Rule HSCAJ.4.1 --- All if ... else if constructs shall be terminated with an else statement
Description: Requiring an else clause ensures that the developer has considered all possible cases. It also provides a place to handle unexpected conditions or to document that no action is required.

### Non-Compliant --- missing else:
void classify( int32_t num ) {
    if ( num > 0 ) {
        handlePositive();
    } else if ( num < 0 ) {
        handleNegative();
    }
    // Non-compliant — no else for num == 0
}
### Compliant --- else present:
void classify( int32_t num ) {
    if ( num > 0 ) {
        handlePositive();
    } else if ( num < 0 ) {
        handleNegative();
    } else {
        // num == 0: no action required
    }
}
### Compliant --- single if without else if:
void toggle( bool enabled ) {
    if ( enabled ) {
        activate();
    }
    // Compliant — not an if...else if construct
}


Rule HSCAP.0.2 (Dir) --- User-provided copy and move assignment operators shall handle self-assignment
Description: Naïve implementations of copy and move assignment can exhibit undefined behaviour, resource leaks, or loss of data when an object is assigned to itself. Self-assignment is rarely intentional but can occur through aliased references or overlapping ranges. Well-known idioms, such as copy-and-swap, may help when complying with this directive.

### Non-Compliant --- self-assignment causes undefined behaviour:
class Array {
    std::size_t length_;
    int32_t* elements_;
public:
    Array& operator=(const Array& other) & {
        length_ = other.length_;
        delete[] elements_;                          // Deletes other.elements_ if self!
        elements_ = new int32_t[other.length_];      // Reads deleted memory — UB
        std::copy_n(other.elements_, other.length_, elements_);
        return *this;
    }
};
### Compliant --- self-assignment check:
Array& Array::operator=(const Array& other) & {
    if (this != std::addressof(other)) {
        length_ = other.length_;
        delete[] elements_;
        elements_ = new int32_t[other.length_];
        std::copy_n(other.elements_, other.length_, elements_);
    }
    return *this;
}
### Compliant --- copy-and-swap idiom:
Array& Array::operator=(const Array& other) & {
    Array temp(other);                             // Copy construction
    std::swap(length_, temp.length_);
    std::swap(elements_, temp.elements_);
    return *this;
    // temp destructor frees old elements
}


Rule HSCAS.4.1 --- Exception-unfriendly functions shall be noexcept
Description: The following functions must be implicitly or explicitly noexcept: 1. Any function directly called to initialize a non-constexpr, non-local variable with static or thread storage duration; 2. All destructors; 3. All copy-constructors of an exception object; 4. All move constructors; 5. All move assignment operators; 6. All functions named "swap".
When an exception is thrown, destructors for automatic objects are invoked. If one of these destructors exits with an exception, the program will terminate. Move constructors and move assignment operators are usually expected to be non-throwing; if they are not declared noexcept, strong exception safety is more difficult to achieve.

### Non-Compliant --- move operations without noexcept:
class Handler {
public:
    Handler(){}
    Handler( Handler && other ) {}                           // Non-compliant — move constructor
    Handler & operator=( Handler && other );                 // Non-compliant — move assignment
    friend void swap( Handler &, Handler & );                // Non-compliant — function named swap
};

Handler instance;                                             // Non-compliant — construction is non-local

class Fragile {
public:
    ~Fragile() noexcept( false ) {}                     // Non-compliant — destructor
};
### Compliant --- noexcept specified:
class Safe {
public:
    Safe(){}
    ~Safe(){}                                        // Compliant — noexcept by default
};

class Handler {
public:
    Handler( Handler && other ) noexcept {}                  // Compliant
    Handler & operator=( Handler && other ) & noexcept;      // Compliant
    friend void swap( Handler &, Handler & ) noexcept;       // Compliant
    ~Handler() noexcept( true ) {}                      // Compliant
};


Rule HSCAV.6.2 --- Dynamic memory shall be managed automatically
Description: The use of dynamic memory requires tracking of any memory resources that are allocated to ensure that they are released appropriately (no memory leaks, no double frees, use of a matching deallocation function). This is likely to be error prone if it is not managed automatically using facilities such as std::make_unique or std::vector.
A program shall NOT use: 1. Any non-placement form of new or delete; 2. malloc, calloc, realloc, aligned_alloc, free; 3. Any member function named allocate or deallocate enclosed by namespace std; 4. std::unique_ptr::release.

### Non-Compliant --- raw new/delete:
class Resource { /* ... */ };

auto raw1 = static_cast< Resource * >( malloc( sizeof( Resource ) ) );   // Non-compliant
auto raw2 = new Resource;                                          // Non-compliant
auto managed = std::make_unique< Resource >();
auto released = managed.release();                                   // Non-compliant

void cleanup( Resource * ptr ) {
    delete ptr;                                             // Non-compliant
}
### Compliant --- smart pointers:
auto managed1 = std::make_unique< Resource >();                        // Compliant
auto managed2 = std::make_shared< Resource >();                        // Compliant
std::vector< Resource > collection;                                     // Compliant


Rule HSCAV.6.3 --- Advanced memory management shall not be used
Description: There are a number of complex issues, such as alignment, object lifetimes and the need to use std::launder, that must be considered when using advanced memory management. Failure to deal with these appropriately results in the introduction of undefined behaviour that is hard to identify.
Advanced memory management occurs when: 1. An advanced memory management function is called (placement new, user-declared operator new/delete); 2. A destructor is called explicitly; 3. Any operator new or operator delete is user-declared.

### Non-Compliant --- placement new and explicit destructor:
struct Block { int32_t value; };

alignas( Block ) std::byte buffer[ sizeof( Block ) ];

void construct() {
    Block * ptr = new( buffer ) Block{ 42 };                // Non-compliant — placement new
    ptr->~Block();                                           // Non-compliant — explicit destructor call
}

void * operator new( std::size_t, void * loc ) {     // Non-compliant — user-declared
    return loc;
}
### Compliant --- standard allocation:
auto allocate() noexcept {
    return new( std::nothrow ) int{ 42 };          // Compliant — nothrow form
}
Rule HSCBC.6.3 --- An object shall not be used while in a potentially moved-from state
Description: Using std::forward or std::move on an lvalue to pass it as an rvalue reference argument in a function call can result in the lvalue object being in an indeterminate state after the call. However, a std::unique_ptr that has been moved-from is in a well-defined state, equal to nullptr.
An object in a potentially moved-from state shall not be used except for: - Assigning to the object; - Destroying the object; - Using an object having type std::unique_ptr.

### Non-Compliant --- using moved-from object:
size_t measure( std::string text ) {
    std::string copy = std::move( text );
    return text.size();                              // Non-compliant — text has potentially
}                                                  //                 moved-from state

void duplicate( std::string text ) {
    std::string first = std::move( text );
    std::string second = text;                           // Non-compliant — text has potentially
}                                                  //                 moved-from state

template< typename T >
void forward( T && item ) {
    consume( std::forward< T >( item ) );
    ++item;                                           // Non-compliant — std::forward leaves item
}                                                  //   in a potentially moved-from state
### Compliant --- no use after move:
void transfer( Block obj ) {
    Block target;
    target = std::move( obj );                            // Compliant — no more uses of obj
}

void reset( Block obj ) {
    Block target;
    target = std::move( obj );
    obj = Block{};                                       // Compliant — assigns to potentially
}                                                  //             moved-from object

