// Rule: HSCAP.1.4 - All Data Members Initialized
// Description: Constructors must explicitly initialize all data members in the
// initializer list

// BAD - not all members initialized
class BadPerson {
private:
  std::string name;
  int age;
  std::string email;

public:
  // Only initializes 'name' and 'age', 'email' is left uninitialized
  BadPerson(const std::string &n, int a) : name(n), age(a) {}
};

// GOOD - all members initialized
class GoodPerson {
private:
  std::string name;
  int age;
  std::string email;

public:
  GoodPerson(const std::string &n, int a, const std::string &e)
      : name(n), age(a), email(e) {}
};

// GOOD - with in-class initializers for some members
class BetterPerson {
private:
  std::string name;
  int age;
  std::string email = ""; // In-class initializer
public:
  BetterPerson(const std::string &n, int a) : name(n), age(a) {}
};
