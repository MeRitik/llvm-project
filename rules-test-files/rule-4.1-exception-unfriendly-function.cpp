// Rule: HSCAS.4.1 - Exception-Unfriendly Functions
// Description: Destructors, move constructors, and move assignment operators
// should be noexcept

// BAD - exceptions can throw
class BadResource {
private:
  std::unique_ptr<char[]> data;

public:
  BadResource() : data(std::make_unique<char[]>(1024)) {}

  // PROBLEM - destructor not noexcept
  ~BadResource() {
    if (data) {
      processBeforeDelete(); // Can throw!
      data.reset();
    }
  }

  // PROBLEM - move operations not noexcept
  BadResource(BadResource &&other) {
    data = std::move(other.data);
    validate(); // Can throw!
  }

  BadResource &operator=(BadResource &&other) {
    if (this != &other) {
      data = std::move(other.data);
      validate(); // Can throw!
    }
    return *this;
  }

private:
  void processBeforeDelete() {}
  void validate() {}
};

// PROBLEM - if BadResource is used in container and reallocation happens,
// and move throws, the container gets unstable state

// GOOD - exception-safe resource cleanup
class GoodResource {
private:
  std::unique_ptr<char[]> data;

public:
  GoodResource() : data(std::make_unique<char[]>(1024)) {}

  // Destructor does not throw
  ~GoodResource() noexcept {
    // No operations that can throw
    // unique_ptr destructor is noexcept
  }

  // Move operations don't throw (unique_ptr move is noexcept)
  GoodResource(GoodResource &&other) noexcept {
    data = std::move(other.data); // noexcept
  }

  GoodResource &operator=(GoodResource &&other) noexcept {
    data = std::move(other.data);
    return *this;
  }
};

// Test container safety
void testContainerSafety() {
  std::vector<GoodResource> resources;
  resources.push_back(GoodResource()); // Safe - move is noexcept
  // If reallocation happens, resources won't corrupt
}

// RULE SUMMARY - These MUST be noexcept or function is broken:
// 1. Destructors - unwinding during stack unwinding causes terminate()
// 2. Move constructors - needed for strong exception guarantee in containers
// 3. Move assignment operators - needed for strong exception guarantee in
// containers
// 4. Swap operations - usually noexcept for exception safety
