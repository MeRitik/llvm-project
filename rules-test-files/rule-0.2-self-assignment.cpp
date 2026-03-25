// Rule: HSCAP.0.2 - Self-Assignment in Assignment Operator
// Description: Assignment operators must handle self-assignment (a = a)

// BAD - no self-assignment check
class BadBuffer {
private:
  char *data = nullptr;
  size_t size = 0;

public:
  BadBuffer &operator=(const BadBuffer &other) {
    if (data)
      delete[] data;
    data = new char[other.size];
    size = other.size;
    std::memcpy(data, other.data, size);
    return *this;
  }
  ~BadBuffer() { delete[] data; }
};

void testBad() {
  BadBuffer b;
  b = b; // DANGER: deletes data, then tries to copy from deleted data!
}

// GOOD - includes self-assignment check
class GoodBuffer {
private:
  char *data = nullptr;
  size_t size = 0;

public:
  GoodBuffer &operator=(const GoodBuffer &other) {
    if (this == &other) // Self-assignment check
      return *this;
    if (data)
      delete[] data;
    data = new char[other.size];
    size = other.size;
    std::memcpy(data, other.data, size);
    return *this;
  }
  ~GoodBuffer() { delete[] data; }
};

// ALTERNATIVE GOOD - copy-and-swap pattern
class AlternativeBuffer {
private:
  char *data = nullptr;
  size_t size = 0;

public:
  AlternativeBuffer &operator=(AlternativeBuffer temp) {
    std::swap(data, temp.data);
    std::swap(size, temp.size);
    return *this;
  }
  ~AlternativeBuffer() { delete[] data; }
};
