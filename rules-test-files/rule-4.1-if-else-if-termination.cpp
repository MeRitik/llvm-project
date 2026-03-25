// Rule: HSCAJ.4.1 - If-Else-If Termination
// Description: Every if-else-if chain must end with an else clause to handle
// all cases

// BAD - if-else-if without final else
void classifyByScore(int score) {
  if (score >= 90) {
    printf("Grade: A\n");
  } else if (score >= 80) {
    printf("Grade: B\n");
  } else if (score >= 70) {
    printf("Grade: C\n");
  }
  // PROBLEM: what if score < 70? Undefined behavior if action required
}

int processStatusCode(int code) {
  if (code == 200) {
    return SUCCESS;
  } else if (code == 404) {
    return NOT_FOUND;
  } else if (code == 500) {
    return ERROR;
  }
  // PROBLEM: unhandled status codes silently ignored
}

// GOOD - terminate with else clause
void properClassification(int score) {
  if (score >= 90) {
    printf("Grade: A\n");
  } else if (score >= 80) {
    printf("Grade: B\n");
  } else if (score >= 70) {
    printf("Grade: C\n");
  } else { // Catch all remaining cases
    printf("Grade: F\n");
  }
}

void properStatusHandling(int code) {
  if (code == 200) {
    return SUCCESS;
  } else if (code == 404) {
    return NOT_FOUND;
  } else if (code == 500) {
    return ERROR;
  } else { // Handle unknown codes
    return UNKNOWN_ERROR;
  }
}

// GOOD - single if when you don't need else
void singleCondition(int x) {
  if (x < 0) {
    handleNegative(x); // No else needed - just action
  }
}

// GOOD - explicit enumeration when appropriate
enum Color { RED, GREEN, BLUE, UNKNOWN };

Color mapColorCode(int code) {
  if (code == 1) {
    return RED;
  } else if (code == 2) {
    return GREEN;
  } else if (code == 3) {
    return BLUE;
  } else {
    return UNKNOWN; // Must handle unknown
  }
}

// BETTER - use switch for enumerations
Color mapColorCodeSwitch(int code) {
  switch (code) {
  case 1:
    return RED;
  case 2:
    return GREEN;
  case 3:
    return BLUE;
  default:
    return UNKNOWN; // Switch enforces all cases
  }
}

// PATTERN - when else is not needed
void validateData(const std::string &data) {
  if (data.empty()) {
    throw std::invalid_argument("Data cannot be empty");
  }
  // No else needed - if we reach here, data is valid
  processData(data);
}

// WARNING - don't use else if it's unreachable
void warningExample(int x) {
  if (x > 0) {
    processPositive(x);
  } else if (x < 0) {
    processNegative(x);
  } else if (x == 0) {
    processZero(x);
  } else { // This is unreachable! (all cases covered above)
    // Remove this else
  }
}

// RULE SUMMARY:
// 1. If you have if-else-if: must end with else
// 2. If else is unreachable: remove it
// 3. If you have just if: else is optional
// 4. For state machines: use switch statements (clearer intent)
