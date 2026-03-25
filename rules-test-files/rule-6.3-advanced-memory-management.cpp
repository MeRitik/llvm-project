// Rule: HSCAV.6.3 - Advanced Memory Management
// Description: Avoid placement new, explicit destructor calls, and user-defined
// operator new/delete

// BAD - placement new (difficult to manage correctly)
void badPlacementNew() {
  alignas(std::max_align_t) char buffer[sizeof(ComplexObject)];

  ComplexObject *obj = new (buffer) ComplexObject("test");
  // PROBLEM - must call destructor explicitly
  obj->~ComplexObject();
  // Easy to forget or call twice
}

// BAD - explicit destructor call
void badExplicitDestructor() {
  ComplexObject *obj = new ComplexObject("test");
  obj->~ComplexObject(); // Manual call is dangerous
  delete obj;            // But we still need delete (calls destructor again!)
}

// BAD - user-defined operator new/delete
class BadAllocator {
public:
  static void *operator new(std::size_t size) {
    printf("Allocating %zu bytes\n", size);
    return malloc(size);
  }

  static void operator delete(void *ptr) noexcept {
    printf("Deallocating\n");
    free(ptr);
  }
};

// GOOD - use smart pointers that handle placement new properly
class PlacementWrapper {
private:
  alignas(std::max_align_t) char buffer[sizeof(ComplexObject)];
  ComplexObject *obj = nullptr;

public:
  void create(const std::string &name) {
    if (!obj) {
      obj = new (buffer) ComplexObject(name);
    }
  }

  ~PlacementWrapper() {
    if (obj) {
      obj->~ComplexObject(); // Only called during cleanup
      obj = nullptr;
    }
  }
};

// GOOD - allocation wrapper with RAII
template <typename T> class ScopedAllocation {
private:
  char *buffer;
  T *obj;

public:
  ScopedAllocation() : buffer(new char[sizeof(T)]), obj(nullptr) {}

  template <typename... Args> void construct(Args &&...args) {
    obj = new (buffer) T(std::forward<Args>(args)...);
  }

  ~ScopedAllocation() {
    if (obj)
      obj->~T();
    delete[] buffer;
  }
};

// BEST - just use regular new/delete or smart pointers
void bestApproach() {
  auto obj = std::make_unique<ComplexObject>("test");
  // Automatic cleanup, exception-safe
}
