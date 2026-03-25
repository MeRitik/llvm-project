// Rule: HSCAI.18.1 - Overlapping Copy
// Description: Use memmove instead of memcpy when source and destination might
// overlap

// BAD - memcpy with potentially overlapping regions
void badBufferShift(char *buffer, size_t size) {
  // Shifting contents to the right - regions might overlap
  memcpy(buffer + 10, buffer, size - 10); // PROBLEM: overlap!
  // memcpy doesn't handle overlapping regions correctly
  // Result: corrupted data
}

void badMergeData(char *dest, const char *src, size_t size) {
  // If dest and src overlap or are the same:
  memcpy(dest + 5, src, size); // PROBLEM: potential overlap
}

// GOOD - memmove handles overlapping correctly
void goodBufferShift(char *buffer, size_t size) {
  memmove(buffer + 10, buffer, size - 10); // Safe - handles overlap
}

void goodMergeData(char *dest, const char *src, size_t size) {
  memmove(dest + 5, src, size); // Safe - even if regions overlap
}

// EXAMPLE - copying part of buffer to itself
void inPlaceTransform(uint32_t *data, size_t count) {
  // Need to transform in-place while reordering

  // BAD approach (if using memcpy):
  // uint32_t temp[1000];
  // memcpy(temp, data + 100, 900 * sizeof(uint32_t));  // Read
  // memcpy(data, temp, 900 * sizeof(uint32_t));        // Write back - overlap!

  // GOOD approach (if absolutely needed):
  uint32_t temp[1000];
  memmove(temp, data + 100, 900 * sizeof(uint32_t));
  memmove(data, temp, 900 * sizeof(uint32_t));

  // BEST approach - use std algorithms:
  // std::shift_left(data, data + count, 100);
  // std::rotate(data, data + 100, data + count);
}

// WARNING - memcpy vs memmove performance
void memoryOperationNotes() {
  char buffer[1000];

  // memcpy assumptions:
  // - Faster (can assume non-overlapping)
  // - Source and destination don't overlap
  // - Copy direction doesn't matter for performance

  memcpy(buffer + 100, buffer, 100); // OK - distinct regions

  // memmove implementation:
  // - Slower (must detect and handle overlap)
  // - Works with any overlapping scenario
  // - Forward copy if source < destination, backward if source > destination

  memmove(buffer + 50, buffer, 100); // OK - overlapping

  // Rule: When uncertain about overlap, use memmove
}

// C++ SOLUTION - avoid raw memory operations
void modernApproach() {
  std::vector<uint32_t> data(1000);

  // Safe, clear, no manual memory management
  std::rotate(data.begin(), data.begin() + 100, data.end());

  std::shift_left(data.begin(), data.end(), 50);
}
